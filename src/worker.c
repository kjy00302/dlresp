#include "worker.h"

void * worker(void* arg) {
    decoder_ctx_t *ctx = arg;
    // printf("Bulk loop in\n");
    while (1) {
        int c = fgetc(ctx->stream); // errro
        if (c == EOF) break;
        if (c != 0xaf) continue;
        int cmd = fgetc(ctx->stream);
        switch (cmd) {
            case 0x20:
                cmd_setreg(ctx);
                break;
            case 0x61:
                cmd_fill8(ctx);
                break;
            case 0x62:
                cmd_memcpy8(ctx);
                break;
            case 0x69:
                cmd_fill16(ctx);
                break;
            case 0x6a:
                cmd_memcpy16(ctx);
                break;
            case 0x70:
                cmd_comp8(ctx);
                break;
            case 0x78:
                cmd_comp16(ctx);
                break;
            case 0xa0:break;
            case 0xe0:
                cmd_loadtable(ctx);
                break;
            default:
                printf("err: unknown cmd 0x%02x\n", cmd & 0xff);
                return NULL;
        }
    }
    // printf("bulk loop out\n");
    return NULL;
} 
