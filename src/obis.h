/*
 * Copyright (c) 2022 Martin Jäger
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef OBIS_H_
#define OBIS_H_

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

struct obis_code
{
    uint8_t a;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    char *name;
};

#define OBIS_ELECTRICITY 1U
#define OBIS_GAS         7U

/**
 * Generate object name from an OBIS code
 *
 * @param decode_unknown Decode unknown codes by converting the code itself into a string
 */
int obis_decode_object_name(uint8_t *obis, size_t obis_len, char *str, size_t str_size,
                            bool decode_unknown);

#endif /* OBIS_H_ */
