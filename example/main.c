/*
 * Copyright (c) 2022 Martin Jäger
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "sml_parser.h"

uint8_t sml_buf[10000];
uint8_t json_buf[2000];

struct sml_values_electricity values_electricity;

int main(void)
{
    freopen(NULL, "rb", stdin);
    size_t len = fread(sml_buf, 1, sizeof(sml_buf), stdin);

    struct sml_context sml_ctx = {
        .sml_buf = sml_buf,
        .sml_buf_len = len,
        .sml_buf_pos = 0,
        .values_electricity = &values_electricity,
    };

    printf("Parsing %d bytes:\n", len);

    while (sml_ctx.sml_buf_pos < sml_ctx.sml_buf_len) {
        int err = sml_parse(&sml_ctx);
        if (err) {
            printf("Parser error %d at position 0x%x\n", err, sml_ctx.sml_buf_pos);
            return 1;
        }

        printf("---------------------------------\n");
        sml_debug_print(&sml_ctx);
        printf("---------------------------------\n\n");
    }

    return 0;
}
