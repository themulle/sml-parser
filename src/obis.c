/*
 * Copyright (c) 2022 Martin Jäger
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "obis.h"

#include <stdio.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#endif

/* clang-format off */
static struct obis_code obis_map[] = {
    { OBIS_ELECTRICITY,  1,   8, 0, "rSumImpAct" },
    { OBIS_ELECTRICITY,  1,   8, 1, "rSumImpActT1" },
    { OBIS_ELECTRICITY,  1,   8, 2, "rSumImpActT2" },
    { OBIS_ELECTRICITY,  2,   8, 0, "rSumExpAct" },
    { OBIS_ELECTRICITY,  2,   8, 1, "rSumExpActT1" },
    { OBIS_ELECTRICITY,  2,   8, 2, "rSumExpActT2" },
    { OBIS_ELECTRICITY,  3,   8, 0, "rSumImpReact" },
    { OBIS_ELECTRICITY,  4,   7, 0, "rSumExpReact" },
    { OBIS_ELECTRICITY, 14,   7, 0, "r" },      // frequency
    { OBIS_ELECTRICITY, 15,   7, 0, "rAct" },
    { OBIS_ELECTRICITY, 16,   7, 0, "rActDelta" },
    { OBIS_ELECTRICITY, 31,   7, 0, "rL1" },    // current
    { OBIS_ELECTRICITY, 32,   7, 0, "rL1" },    // voltage
    { OBIS_ELECTRICITY, 51,   7, 0, "rL2" },
    { OBIS_ELECTRICITY, 52,   7, 0, "rL2" },
    { OBIS_ELECTRICITY, 71,   7, 0, "rL3" },
    { OBIS_ELECTRICITY, 72,   7, 0, "rL3" },
    { OBIS_ELECTRICITY, 81,   7, 1, "rUL2UL1" },
    { OBIS_ELECTRICITY, 81,   7, 2, "rUL3UL1" },
    { OBIS_ELECTRICITY, 81,   7, 4, "rIL1UL1" },
    { OBIS_ELECTRICITY, 81,   7, 15, "rIL2UL2" },
    { OBIS_ELECTRICITY, 81,   7, 26, "rIL3UL3" },
//  { OBIS_ELECTRICITY,  0,   0, 9, "rDeviceID" },
    { 129,             199, 130, 3, "rManufacturer" },
//  { 129,             199, 130, 5, "rPublicKey" },
};
/* clang-format on */

int obis_decode_object_name(uint8_t *obis, size_t obis_len, char *str, size_t str_size,
                            bool decode_unknown)
{
    int pos = 0;

    for (int i = 0; i < ARRAY_SIZE(obis_map); i++) {
        if (obis[2] == obis_map[i].c && obis[3] == obis_map[i].d && obis[4] == obis_map[i].e) {
            pos = snprintf(str, str_size - 1, "%s", obis_map[i].name);
            goto out;
        }
    }

    if (decode_unknown) {
        pos = snprintf(str, str_size - 1, "A%dB%dC%dD%dE%dF%d", obis[0], obis[1], obis[2], obis[3],
                       obis[4], obis[5]);
    }

out:
    if (pos > 0) {
        return pos;
    }
    else {
        return -1;
    }
}
