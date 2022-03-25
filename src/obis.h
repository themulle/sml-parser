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
 * A: media type
 * B: channel number (ignored for simple household meters)
 * C: abstract or phyisical data item (e.g. voltage)
 * D: processing of physical quantities (e.g. integral)
 * E: further classification (e.g. tariff number)
 * F: billing period data (ignored, usually 255)
 */
#define OBIS_CODE_SHORT(a, c, d, e) \
    (((uint32_t)a << 24) | ((uint32_t)c << 16) | ((uint32_t)d << 8) | (uint32_t)e)

#define OBIS_CODE_ELECTRICITY(c, d, e) OBIS_CODE_SHORT(OBIS_ELECTRICITY, c, d, e)

#define OBIS_ELECTRICITY_IMPORT_ACTIVE_ENERGY_TOTAL    OBIS_CODE_ELECTRICITY(1, 8, 0)
#define OBIS_ELECTRICITY_IMPORT_ACTIVE_ENERGY_TARIFF_1 OBIS_CODE_ELECTRICITY(1, 8, 1)
#define OBIS_ELECTRICITY_IMPORT_ACTIVE_ENERGY_TARIFF_2 OBIS_CODE_ELECTRICITY(1, 8, 2)
#define OBIS_ELECTRICITY_IMPORT_ACTIVE_POWER_TOTAL     OBIS_CODE_ELECTRICITY(1, 7, 0)
#define OBIS_ELECTRICITY_IMPORT_ACTIVE_POWER_TARIFF_1  OBIS_CODE_ELECTRICITY(1, 7, 1)
#define OBIS_ELECTRICITY_IMPORT_ACTIVE_POWER_TARIFF_2  OBIS_CODE_ELECTRICITY(1, 7, 2)
#define OBIS_ELECTRICITY_EXPORT_ACTIVE_ENERGY_TOTAL    OBIS_CODE_ELECTRICITY(2, 8, 0)
#define OBIS_ELECTRICITY_EXPORT_ACTIVE_ENERGY_TARIFF_1 OBIS_CODE_ELECTRICITY(2, 8, 1)
#define OBIS_ELECTRICITY_EXPORT_ACTIVE_ENERGY_TARIFF_2 OBIS_CODE_ELECTRICITY(2, 8, 2)
#define OBIS_ELECTRICITY_IMPORT_REACTIVE_ENERGY_TOTAL  OBIS_CODE_ELECTRICITY(3, 8, 0)
#define OBIS_ELECTRICITY_EXPORT_REACTIVE_ENERGY_TOTAL  OBIS_CODE_ELECTRICITY(4, 8, 0)
#define OBIS_ELECTRICITY_FREQUENCY                     OBIS_CODE_ELECTRICITY(14, 7, 0)
#define OBIS_ELECTRICITY_ACTIVE_POWER                  OBIS_CODE_ELECTRICITY(15, 7, 0)
#define OBIS_ELECTRICITY_ACTIVE_POWER_DELTA            OBIS_CODE_ELECTRICITY(16, 7, 0)
#define OBIS_ELECTRICITY_L1_CURRENT                    OBIS_CODE_ELECTRICITY(31, 7, 0)
#define OBIS_ELECTRICITY_L1_VOLTAGE                    OBIS_CODE_ELECTRICITY(32, 7, 0)
#define OBIS_ELECTRICITY_L2_CURRENT                    OBIS_CODE_ELECTRICITY(51, 7, 0)
#define OBIS_ELECTRICITY_L2_VOLTAGE                    OBIS_CODE_ELECTRICITY(52, 7, 0)
#define OBIS_ELECTRICITY_L3_CURRENT                    OBIS_CODE_ELECTRICITY(71, 7, 0)
#define OBIS_ELECTRICITY_L3_VOLTAGE                    OBIS_CODE_ELECTRICITY(72, 7, 0)
#define OBIS_ELECTRICITY_UL2_UL1_PHASE_ANGLE           OBIS_CODE_ELECTRICITY(81, 7, 1)
#define OBIS_ELECTRICITY_UL3_UL1_PHASE_ANGLE           OBIS_CODE_ELECTRICITY(81, 7, 2)
#define OBIS_ELECTRICITY_IL1_UL1_PHASE_ANGLE           OBIS_CODE_ELECTRICITY(81, 7, 4)
#define OBIS_ELECTRICITY_IL2_UL2_PHASE_ANGLE           OBIS_CODE_ELECTRICITY(81, 7, 15)
#define OBIS_ELECTRICITY_IL3_UL3_PHASE_ANGLE           OBIS_CODE_ELECTRICITY(81, 7, 26)

/**
 * Print object name from an OBIS code (for debugging)
 */
void obis_print_object_name(uint8_t *obis, size_t obis_len, uint8_t unit, int scaler);

#endif /* OBIS_H_ */
