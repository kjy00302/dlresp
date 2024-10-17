#include "decoder.h"

void cmd_setreg(decoder_ctx_t *ctx) {
    uint8_t buf[2];
    fread(buf, 2, 1, ctx->stream);
    ctx->reg[buf[0]] = buf[1];
    // printf("cmd_setreg addr: %02x, value: %02x\n", buf[0], buf[1]);
    // if (buf[0] == 0xff && buf[1] == 0xff && ctx->reg[0x1f] == 0) {
    //     printf("render frame at %06x\n", read24_be(0x20, ctx->reg));
    // }
}

void cmd_loadtable(decoder_ctx_t *ctx) {
    uint8_t buf[9];
    // fseek(ctx->stream, 4, SEEK_CUR); // skip 263871cd
    fread(&buf, 4, 1, ctx->stream); // fd doesn't have seek
    fread(&buf, 4, 1, ctx->stream); // read length
    uint32_t len = read32_be(buf, 0);
    // printf("cmd_loadtable cnt: %d\n", len);
    for (int i=0; i<len; i++) {
        decomp_entry_t *row = ctx->decomp_table[i];
        fread(buf, 9, 1, ctx->stream);
        // short0msb, short0lsb, byte0a, byte0b, byte01, short1msb, short1lsb, byte1a, byte1b
        row[0].color = read16_be(buf, 0);
        row[0].repeat = buf[2];
        row[0].jump = (buf[3] & 0x1f) << 4 | buf[4] >> 4;
        row[1].color = read16_be(buf, 5);
        row[1].repeat = buf[7];
        row[1].jump = (buf[8] & 0x1f) << 4 | (buf[4] & 0xf);
    }
}

void cmd_fill8(decoder_ctx_t *ctx) {
    uint32_t addr, totalcnt, cnt;
    uint8_t buf[4];
    fread(&buf, 4, 1, ctx->stream);
    addr = read24_be(buf, 0);
    totalcnt = wrap256(buf[3]);
    // printf("cmd_fill8 addr: %06x, cnt: %d\n", addr, totalcnt);
    while (totalcnt > 0) {
        fread(&buf, 2, 1, ctx->stream);
        cnt = wrap256(buf[0]);
        for (int i=0;i<cnt;i++) {
            ((uint8_t*)(ctx->gfxram+addr))[i] = buf[1];
        }
        totalcnt -= cnt;
        addr += cnt;
    }
}

void cmd_fill16(decoder_ctx_t *ctx) {
    uint32_t addr, totalcnt, cnt;
    uint8_t buf[4];
    fread(&buf, 4, 1, ctx->stream);
    addr = read24_be(buf, 0);
    totalcnt = wrap256(buf[3]);
    // printf("cmd_fill16 addr: %06x, cnt: %d\n", addr, totalcnt);
    while (totalcnt > 0) {
        fread(&buf, 3, 1, ctx->stream);
        cnt = wrap256(buf[0]);
        for (int i=0;i<cnt;i++) {
            ((uint16_t*)(ctx->gfxram+addr))[i] = *(uint16_t*)(buf+1);
            // ((uint8_t *)(ctx->gfxram + addr))[i*2] = buf[1];
            // ((uint8_t *)(ctx->gfxram + addr))[i*2+1] = buf[2];
        }
        totalcnt -= cnt;
        addr += cnt * 2;
    }
}

void cmd_comp8(decoder_ctx_t *ctx) {
    uint32_t addr, cnt;
    uint32_t bitcnt = 8;
    uint32_t tableidx = 0;
    uint8_t accumulator = 0;
    uint8_t buf[4];
    uint8_t bitbuffer = 0;
    fread(&buf, 4, 1, ctx->stream);
    addr = read24_be(buf, 0);
    cnt = wrap256(buf[3]);
    // printf("cmd_comp8 addr: %06x, cnt: %d\n", addr, cnt);
    for (int i=0; i<cnt; i++) {
        while (1) {
            if (bitcnt == 8) {
                fread(&bitbuffer, 1, 1, ctx->stream);
                bitcnt = 0;
            }
            decomp_entry_t *table = &ctx->decomp_table[tableidx][bitbuffer & 1];
            bitbuffer >>= 1;
            bitcnt++;
            accumulator += table->color & 0xff;
            tableidx = table->jump;
            if (tableidx == 0) break;
        }
        if (addr + i > 0xffffff) {
            printf("comp8 warning: oob write: %06x * %d\n", addr, i);
            tableidx = 0;
            continue;
        }
        ((uint8_t*)(ctx->gfxram+addr))[i] = accumulator;
        tableidx = 0;
    }
}

void cmd_comp16(decoder_ctx_t *ctx) {
    uint32_t addr, cnt;
    uint32_t bitcnt = 8;
    uint32_t tableidx = 8;
    uint16_t accumulator = 0;
    uint8_t buf[4];
    uint8_t bitbuffer = 0;
    fread(&buf, 4, 1, ctx->stream);
    addr = read24_be(buf, 0);
    cnt = wrap256(buf[3]);
    // printf("cmd_comp16 addr: %06x, cnt: %d\n", addr, cnt);
    for (int i=0; i<cnt; i++) {
        while (1) {
            if (bitcnt == 8) {
                fread(&bitbuffer, 1, 1, ctx->stream);
                bitcnt = 0;
            }
            decomp_entry_t *table = &ctx->decomp_table[tableidx][bitbuffer & 1];
            bitbuffer >>= 1;
            bitcnt++;
            accumulator += table->color;
            tableidx = table->jump;
            if (tableidx == 0) break;
        }
        ((uint16_t*)(ctx->gfxram+addr))[i] = accumulator;
        tableidx = 8;
    }
}

void cmd_memcpy8(decoder_ctx_t *ctx) {
    uint32_t dstaddr, srcaddr, cnt;
    uint8_t buf[7];
    fread(&buf, 7, 1, ctx->stream);
    dstaddr = read24_be(buf, 0);
    cnt = wrap256(buf[3]);
    srcaddr = read24_be(buf, 4);
    // printf("cmd_memcpy8 addr: %06x, cnt: %d\n", dstaddr, cnt);
    memcpy(ctx->gfxram+dstaddr, ctx->gfxram+srcaddr, cnt);
}

void cmd_memcpy16(decoder_ctx_t *ctx) {
    uint32_t dstaddr, srcaddr, cnt;
    uint8_t buf[7];
    fread(&buf, 7, 1, ctx->stream);
    dstaddr = read24_be(buf, 0);
    cnt = wrap256(buf[3]);
    srcaddr = read24_be(buf, 4);
    // printf("cmd_memcpy16 addr: %06x, cnt: %d\n", dstaddr, cnt);
    memcpy(ctx->gfxram+dstaddr, ctx->gfxram+srcaddr, cnt*2);
}
