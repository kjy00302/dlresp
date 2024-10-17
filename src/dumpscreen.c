#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <png.h>
#include "common.h"

void dump_png(void* gfxram, uint32_t addr, uint32_t width, uint32_t height, FILE *f);

int main(int argc, char* argv[]) {
    int shmid_gfxram, shmid_reg;
    void *gfxram;
    uint8_t *reg;
    char* filename = "framedump.png";
    void *framebuffer;

    if (argc > 1) {
        filename = argv[1];
    }

    if ((shmid_gfxram = shmget(SHM_KEY, 0x1000000, 0644)) < 0) {
        perror("failed to load shm");
        return 1;
    }
    gfxram = shmat(shmid_gfxram, NULL, 0);
    if (gfxram == (void *) -1) {
        perror("failed to allocate gfxram");
        return 1;
    }
    if ((shmid_reg = shmget(SHM_KEY+1, 0x100, 0644)) < 0) {
        perror("failed to load shm");
        return 1;
    }
    reg = shmat(shmid_reg, NULL, 0);
    if (reg == (void *) -1) {
        perror("failed to allocate registor");
        return 1;
    }
    for (int i=0;i<16;i++) {
        for (int j=0;j<16;j++) {
            printf("%02x ", reg[i*16+j]);
        }
        printf("\n");
    }

    int addr, width, height;
    addr = read24_be(reg, 0x20);
    width = read16_be(reg, 0x0f);
    height = read16_be(reg, 0x17);
    printf("addr: %06x, width:%d, height:%d\n", addr, width, height);
    if (addr == 0) return 0;
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("cannot create file");
        return 1;
    }
    framebuffer = malloc(width * height * 2);
    if (framebuffer == NULL) {
        perror("failed to allocate framebuffer");
        return 1;
    }
    memcpy(framebuffer, gfxram+addr, width * height * 2);
    dump_png(framebuffer, 0, width, height, f);
    fclose(f);
    shmdt(gfxram);
    return 0;
}

void dump_png(void* gfxram, uint32_t addr, uint32_t width, uint32_t height, FILE *f) {
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);
    png_bytep row_pointer = malloc(width * 3);
    png_init_io(png, f);
    png_set_IHDR(
        png, info,
        width, height, 8,
        PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);
    for (int y=0;y<height;y++) {
        for (int x=0;x<width;x++) {
            uint16_t h = ((uint16_t *)(gfxram+addr))[y*width+x];
            row_pointer[x*3] = (h >> 8) & 0xf8;
            row_pointer[x*3+1] = (h >> 3) & 0xfc;
            row_pointer[x*3+2] = (h << 3) & 0xf8;
        }
        png_write_row(png, row_pointer);
    }
    png_write_end(png, info);
    png_destroy_write_struct(&png, &info);
    free(row_pointer);
}
