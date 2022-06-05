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

/* most relevant unit codes of list in obis.c */
enum dlms_units_enum
{
    DLMS_UNIT_YEAR = 1,                  // time                year
    DLMS_UNIT_MONTH = 2,                 // time                month
    DLMS_UNIT_WEEK = 3,                  // time                week
    DLMS_UNIT_DAY = 4,                   // time                day
    DLMS_UNIT_HOUR = 5,                  // time                hour
    DLMS_UNIT_MINUTE = 6,                // time                minute
    DLMS_UNIT_SECOND = 7,                // time                second
    DLMS_UNIT_DEGREE = 8,                // (phase) angle       degree
    DLMS_UNIT_DEGREE_CELSIUS = 9,        // temperature (T)     degree celsius          K-273.15
    DLMS_UNIT_CURRENCY = 10,             // (local) currency
    DLMS_UNIT_METRE = 11,                // length (l)		    metre                   m
    DLMS_UNIT_METRE_PER_SECOND = 12,     // speed (v)			metre per second	    m/s
    DLMS_UNIT_CUBIC_METRE = 13,          // volume (V)          cubic metre		        m³
    DLMS_UNIT_CORR_CUBIC_METRE = 14,     // corrected volume    cubic metre		        m³
    DLMS_UNIT_CUBIC_METRE_PER_HOUR = 15, // volume flux		    cubic metre per hour 	m³/(60*60s)
    DLMS_UNIT_CORR_CUBIC_METRE_PER_HOUR = 16, // corrected volume flux
    DLMS_UNIT_CUBIC_METRE_PER_DAY = 17,       // volume flux m³/(24*60*60s)
    DLMS_UNIT_CORR_CUBIC_METRE_PER_DAY = 18,  // corrected volume flux
    DLMS_UNIT_LITRE = 19,                     // volume			    litre			        10-3 m³
    DLMS_UNIT_KILOGRAM = 20,                  // mass (m)		    kilogram
    DLMS_UNIT_NEWTON = 21,                    // force (F)		    newton
    DLMS_UNIT_NEWTON_METRE = 22,              // energy			    newtonmeter		        J = Nm = Ws
    DLMS_UNIT_PASCAL = 23,                    // pressure (p)	    pascal			        N/m²
    DLMS_UNIT_BAR = 24,                       // pressure (p)	    bar			            10⁵ N/m²
    DLMS_UNIT_JOULE = 25,                     // energy			    joule			        J = Nm = Ws
    DLMS_UNIT_JOULE_PER_HOUR = 26,            // thermal power	    joule per hour		    J/(60*60s)
    DLMS_UNIT_WATT = 27,                      // active power (P)	watt			        W = J/s
    DLMS_UNIT_VOLT_AMPERE = 28,               // apparent power (S)	volt-ampere
    DLMS_UNIT_VAR = 29,                       // reactive power (Q)	var
    DLMS_UNIT_WATT_HOUR = 30,                 // active energy		watt-hour		        W*(60*60s)
    DLMS_UNIT_VOLT_AMPERE_HOUR = 31,          // apparent energy	volt-ampere-hour	    VA*(60*60s)
    DLMS_UNIT_VAR_HOUR = 32,                  // reactive energy	var-hour		        var*(60*60s)
    DLMS_UNIT_AMPERE = 33,                    // current (I)		ampere			        A
    DLMS_UNIT_COULOMB = 34,                   // electrical charge (Q)	coulomb			        C = As
    DLMS_UNIT_VOLT = 35,                      // voltage (U)		volt			        V
    DLMS_UNIT_VOLT_PER_METRE = 36,            // electr. field strength (E)    volt per metre
    DLMS_UNIT_FARAD = 37,                     // capacitance (C)	farad			        C/V = As/V
    DLMS_UNIT_OHM = 38,                       // resistance (R)		ohm			            Ω = V/A
    DLMS_UNIT_OHM_METRE = 39,                 // resistivity (ρ)	Ωm
    DLMS_UNIT_WEBER = 40,                     // magnetic flux (Φ)	weber			        Wb = Vs
    DLMS_UNIT_TESLA = 41,                     // magnetic flux density (B)  tesla			Wb/m2
    DLMS_UNIT_AMPERE_PER_METRE = 42,          // magnetic field strength (H)   ampere per metre	A/m
    DLMS_UNIT_HENRY = 43,                     // inductance (L)		henry			        H = Wb/A
    DLMS_UNIT_HERTZ = 44,                     // frequency (f, ω)	hertz			        1/s
    DLMS_UNIT_1_PER_WATT_HOUR = 45,           // R_W (active energy meter constant or pulse value)
    DLMS_UNIT_1_PER_VAR_HOUR = 46,            // R_B (reactive energy meter constant or pulse value)
    DLMS_UNIT_1_PER_VOLT_AMPERE_HOUR = 47,    // R_S (apparent energy meter constant or pulse value)
    DLMS_UNIT_KG_S = 50,                      // mass flux			kilogram per second	    kg/s
    DLMS_UNIT_SIEMENS = 51,                   // conductance        siemens			        1/Ω
    DLMS_UNIT_KELVIN = 52,                    // temperature (T)	kelvin
    DLMS_UNIT_1_PER_CUBIC_METRE = 55,         // R_V, meter constant or pulse value (volume)
    DLMS_UNIT_PERCENT = 56,                   // percentage		                            %
    DLMS_UNIT_AMPERE_HOUR = 57,               // energy		        ampere-hour
    DLMS_UNIT_MOLE_PERCENT = 62,              // molar fraction of mole percent
    DLMS_UNIT_GRAM_PER_CUBIC_METRE = 63,      // mass density, quantity of material
    DLMS_UNIT_PASCAL_SECOND = 64,             // dynamic viscosity pascal second
};

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
