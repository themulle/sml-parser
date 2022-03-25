/*
 * Copyright (c) 2022 Martin Jäger
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "obis.h"
#include "dlms_units.h"

#include <stdio.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#endif

/* clang-format off */
static struct obis_code obis_map[] = {
    { OBIS_ELECTRICITY,  1,   8, 0, "ImpActEnergy" },
    { OBIS_ELECTRICITY,  1,   8, 1, "ImpActEnergyT1" },
    { OBIS_ELECTRICITY,  1,   8, 2, "ImpActEnergyT2" },
    { OBIS_ELECTRICITY,  1,   7, 0, "ImpActPwr" },
    { OBIS_ELECTRICITY,  1,   7, 1, "ImpActPwrT1" },
    { OBIS_ELECTRICITY,  1,   7, 2, "ImpActPwrT2" },
    { OBIS_ELECTRICITY,  2,   8, 0, "ExpActEnergy" },
    { OBIS_ELECTRICITY,  2,   8, 1, "ExpActEnergyT1" },
    { OBIS_ELECTRICITY,  2,   8, 2, "ExpActEnergyT2" },
    { OBIS_ELECTRICITY,  3,   8, 0, "ImpReactEnergy" },
    { OBIS_ELECTRICITY,  4,   8, 0, "ExpReactEnergy" },
    { OBIS_ELECTRICITY, 14,   7, 0, "Freq_Hz" },      // frequency
    { OBIS_ELECTRICITY, 15,   7, 0, "ActPwr" },
    { OBIS_ELECTRICITY, 16,   7, 0, "ActPwrDelta" },
    { OBIS_ELECTRICITY, 31,   7, 0, "IL1" },    // current
    { OBIS_ELECTRICITY, 32,   7, 0, "VL1" },    // voltage
    { OBIS_ELECTRICITY, 51,   7, 0, "IL2" },
    { OBIS_ELECTRICITY, 52,   7, 0, "VL2" },
    { OBIS_ELECTRICITY, 71,   7, 0, "IL3" },
    { OBIS_ELECTRICITY, 72,   7, 0, "VL3" },
    { OBIS_ELECTRICITY, 81,   7, 1, "PhaseUL2UL1" },
    { OBIS_ELECTRICITY, 81,   7, 2, "PhaseUL3UL1" },
    { OBIS_ELECTRICITY, 81,   7, 4, "PhaseIL1UL1" },
    { OBIS_ELECTRICITY, 81,   7, 15, "PhaseIL2UL2" },
    { OBIS_ELECTRICITY, 81,   7, 26, "PhaseIL3UL3" },
    { OBIS_ELECTRICITY,  0,   0, 9, "DeviceID" },
    { 129,             199, 130, 3, "Manufacturer" },
    { 129,             199, 130, 5, "PublicKey" },
};
/* clang-format on */

void obis_print_object_name(uint8_t *obis, size_t obis_len, uint8_t unit, int scaler)
{
    for (int i = 0; i < ARRAY_SIZE(obis_map); i++) {
        if (obis[2] == obis_map[i].c && obis[3] == obis_map[i].d && obis[4] == obis_map[i].e) {
            printf("%s unit:%s scaler:%d\n", obis_map[i].name, dlms_units[unit], scaler);
            return;
        }
    }

    printf("%d-%d:%d.%d.%d*%d", obis[0], obis[1], obis[2], obis[3], obis[4], obis[5]);
    printf(" unit:%s scaler:%d\n", dlms_units[unit], scaler);
}
