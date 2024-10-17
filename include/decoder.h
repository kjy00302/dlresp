#ifndef DECODER_H_
#define DECODER_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h> // for memcpy
#include "common.h"

typedef struct {
    uint16_t color;
    uint8_t repeat;
    uint32_t jump;
} decomp_entry_t;


typedef struct {
    void* gfxram;
    uint8_t *reg;
    decomp_entry_t decomp_table[512][2];
    FILE *stream;
} decoder_ctx_t;

void cmd_setreg(decoder_ctx_t *ctx);
void cmd_loadtable(decoder_ctx_t *ctx);
void cmd_fill8(decoder_ctx_t *ctx);
void cmd_fill16(decoder_ctx_t *ctx);
void cmd_comp8(decoder_ctx_t *ctx);
void cmd_comp16(decoder_ctx_t *ctx);
void cmd_memcpy8(decoder_ctx_t *ctx);
void cmd_memcpy16(decoder_ctx_t *ctx);

#endif
