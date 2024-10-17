#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/usb/functionfs.h>
#include <sys/poll.h>
#include <errno.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "descriptors.h"
#include "worker.h"
#include "common.h"

char g_edid[128];
void *g_gfxram, *g_reg;
pthread_t th_ep1;
decoder_ctx_t ctx_ep1;

static void display_event(struct usb_functionfs_event *event) {
    static const char *const names[] = {
        [FUNCTIONFS_BIND] = "BIND",
        [FUNCTIONFS_UNBIND] = "UNBIND",
        [FUNCTIONFS_ENABLE] = "ENABLE",
        [FUNCTIONFS_DISABLE] = "DISABLE",
        [FUNCTIONFS_SETUP] = "SETUP",
        [FUNCTIONFS_SUSPEND] = "SUSPEND",
        [FUNCTIONFS_RESUME] = "RESUME",
    };
    switch (event->type) {
        case FUNCTIONFS_BIND:
        case FUNCTIONFS_UNBIND:
        case FUNCTIONFS_ENABLE:
        case FUNCTIONFS_DISABLE:
        // case FUNCTIONFS_SETUP:
        case FUNCTIONFS_SUSPEND:
        case FUNCTIONFS_RESUME:
            printf("Event %s\n", names[event->type]);
    }
}

static void handle_ep0(int ep0, bool *ready) {
    struct usb_functionfs_event event;
    char frags[128] = {0,};
    int ret;

    struct pollfd pfds[1];
    pfds[0].fd = ep0;
    pfds[0].events = POLLIN;

    ret = poll(pfds, 1, 0);

    if (ret && (pfds[0].revents & POLLIN)) {
        ret = read(ep0, &event, sizeof(event));
        if (!ret) {
            perror("unable to read event from ep0");
            return;
        }
        display_event(&event);
        switch (event.type) {
            case FUNCTIONFS_SETUP:
                // printf("reqType: %d, req: %d, val: %d, idx: %d, len: %d\n",
                //        event.u.setup.bRequestType, event.u.setup.bRequest,
                //        event.u.setup.wValue, event.u.setup.wIndex, event.u.setup.wLength);
                if (event.u.setup.bRequestType & USB_DIR_IN) {
                    if (event.u.setup.bRequestType & USB_TYPE_VENDOR) {
                        switch (event.u.setup.bRequest) {
                            case 2:
                                frags[0] = 0; // HACK: no check
                                memcpy(frags+1, g_edid + (event.u.setup.wValue >> 8), event.u.setup.wLength - 1);
                                write(ep0, frags, event.u.setup.wLength);
                            case 4:
                                switch (event.u.setup.wIndex) {
                                    case 0xc484:
                                        write(ep0, "\x83", 1);
                                        break;
                                    default:
                                        write(ep0, "\x00", 1);
                                        break;
                                }
                                break;
                            case 6:
                                write(ep0, "\x00\x50\x00\xf0", 4);
                                break;
                            default:
                                write(ep0, NULL, 0);
                        }
                    } else write(ep0, NULL, 0);
                } else { // USB_DIR_OUT
                    if (event.u.setup.bRequestType & USB_TYPE_VENDOR) {
                        switch (event.u.setup.bRequest) {
                            case 3:
                                read(ep0, frags, 1);
                                break;
                            case 18:
                                read(ep0, frags, 16);
                                printf("encryption key: ");
                                for (int i=0;i<16;i++) {
                                    printf("%02x", frags[i] & 0xff);
                                }
                                printf("\n");
                                break;
                            case 20:
                            default:
                                read(ep0, NULL, 0);
                        }
                    } else read(ep0, NULL, 0);
                }
                break;
            case FUNCTIONFS_ENABLE:
                *ready = true;
                pthread_create(&th_ep1, NULL, worker, &ctx_ep1);
                break;
            case FUNCTIONFS_DISABLE:
                *ready = false;
                pthread_cancel(th_ep1);
                break;
            default:
                break;
        }
    }
}


int main(int argc, char *argv[]) {
    int ret;
    char *ep_path;

    int ep0, ep1, ep2;
    fd_set rfds;

    bool ready;

    if (argc != 3) {
        printf("Usage: %s functionfs-dir edid\n", argv[0]);
        return 1;
    }

    // Load EDID
    FILE *f = fopen(argv[2], "r");
    if (!f) {
        perror("cannot open edid");
        return 1;
    }
    if (fread(g_edid, 1, 128, f) < 128) {
        perror("edid is not 128byte");
        return 1;
    }
    fclose(f);

    // g_gfxram = malloc(0x1000000);
    // if (!g_gfxram) {
    //     perror("failed to allocate gfxram");
    //     return 1;
    // }
    int shmid_gfxram, shmid_reg;

    if ((shmid_gfxram = shmget(SHM_KEY, 0x1000000, IPC_CREAT | 0666)) < 0) {
        perror("failed to allocate shm");
        return 1;
    }
    g_gfxram = shmat(shmid_gfxram, NULL, 0);
    if (g_gfxram == (void *) -1) {
        perror("failed to load gfxram");
        return 1;
    }

    if ((shmid_reg = shmget(SHM_KEY+1, 0x100, IPC_CREAT | 0666)) < 0) {
        perror("failed to allocate shm");
        return 1;
    }
    g_reg = shmat(shmid_reg, NULL, 0);
    if (g_reg == (void *) -1) {
        perror("failed to load registor");
        return 1;
    }
    printf("shm_ram: %d, shm_reg: %d\n", shmid_gfxram, shmid_reg);
    printf("shm_ram: 0x%p, shm_reg: 0x%p\n", g_gfxram, g_reg);

    // BEGIN Setup endpoint
    ep_path = malloc(strlen(argv[1]) + 4 + 1);
    if (!ep_path) {
        perror("malloc");
        return 1;
    }

    sprintf(ep_path, "%s/ep0", argv[1]);
    ep0 = open(ep_path, O_RDWR);
    if (ep0 < 0) {
        perror("unable to open ep0");
        return 1;
    }
    if (write(ep0, &descriptors, sizeof(descriptors)) < 0) {
        perror("unable to write descriptors");
        return 1;
    }
    if (write(ep0, &strings, sizeof(strings)) < 0) {
        perror("unable to write strings");
        return 1;
    }

    sprintf(ep_path, "%s/ep1", argv[1]);
    ep1 = open(ep_path, O_RDWR);
    if (ep1 < 0) {
        perror("unable to open ep1");
        return 1;
    }

    ctx_ep1.gfxram = g_gfxram;
    ctx_ep1.reg = g_reg;
    ctx_ep1.stream = fdopen(ep1, "r");

    sprintf(ep_path, "%s/ep2", argv[1]);
    ep2 = open(ep_path, O_RDWR);
    if (ep2 < 0) {
        perror("unable to open ep2");
        return 1;
    }

    free(ep_path);
    // END Setup endpoint

    while (true) {
        FD_ZERO(&rfds);
        FD_SET(ep0, &rfds);

        ret = select(ep0 + 1,
                     &rfds, NULL, NULL, NULL);
        if (ret < 0) {
            if (errno == EINTR)
                continue;
            perror("select");
            break;
        }

        if (FD_ISSET(ep0, &rfds))
            handle_ep0(ep0, &ready);

        if (!ready)
            continue;
    }
    close(ep0);
    close(ep1);
    return 0;
}
