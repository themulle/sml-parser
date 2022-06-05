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

/**
 * DLMS units used by SML (specified in ISO EN 62056-62)
 *
 * The units are translated into alphanumeric strings + underscore so that they form valid
 * JavaScript names if passed into JSON documents. The same applies for units as used in the
 * ThingSet protocol.
 *
 * Source: https://www.dlms.com/files/Blue-Book-Ed-122-Excerpt.pdf
 */
static const char *dlms_units[] = {
    // Unit     Code  Quantity              Unit name               SI definition
    "",        // 0   dummy
    "a",       // 1   time                  year
    "mo",      // 2   time                  month
    "wk",      // 3   time                  week
    "d",       // 4   time                  day
    "h",       // 5   time                  hour
    "min",     // 6   time                  minute
    "s",       // 7   time                  second
    "deg",     // 8   (phase) angle         degree
    "degC",    // 9   temperature (T)	    degree celsius          K-273.15
    "",        // 10  (local) currency
    "m",       // 11  length (l)		    metre                   m
    "m_s",     // 12  speed (v)			    metre per second	    m/s
    "m3",      // 13  volume (V)            cubic metre		        m³
    "m3",      // 14  corrected volume      cubic metre		        m³
    "m3_h",    // 15  volume flux		    cubic metre per hour 	m³/(60*60s)
    "m3_h",    // 16  corrected volume flux	cubic metre per hour 	m³/(60*60s)
    "m3_d",    // 17  volume flux					                m³/(24*60*60s)
    "m3_d",    // 18  corrected volume flux				            m³/(24*60*60s)
    "l",       // 19  volume			    litre			        10-3 m³
    "kg",      // 20  mass (m)			    kilogram
    "N",       // 21  force (F)			    newton
    "Nm",      // 22  energy			    newtonmeter		        J = Nm = Ws
    "Pa",      // 23  pressure (p)		    pascal			        N/m²
    "bar",     // 24  pressure (p)		    bar			            10⁵ N/m²
    "J",       // 25  energy			    joule			        J = Nm = Ws
    "J_h",     // 26  thermal power		    joule per hour		    J/(60*60s)
    "W",       // 27  active power (P)		watt			        W = J/s
    "VA",      // 28  apparent power (S)	volt-ampere
    "var",     // 29  reactive power (Q)	var
    "Wh",      // 30  active energy		    watt-hour		        W*(60*60s)
    "VAh",     // 31  apparent energy		volt-ampere-hour	    VA*(60*60s)
    "varh",    // 32  reactive energy		var-hour		        var*(60*60s)
    "A",       // 33  current (I)		    ampere			        A
    "C",       // 34  electrical charge (Q)	coulomb			        C = As
    "V",       // 35  voltage (U)		    volt			        V
    "V_m",     // 36  electr. field strength (E)    volt per metre
    "F",       // 37  capacitance (C)		farad			        C/V = As/V
    "Ohm",     // 38  resistance (R)		ohm			            Ω = V/A
    "Ohmm2_m", // 39  resistivity (ρ)		Ωm
    "Wb",      // 40  magnetic flux (Φ)		weber			        Wb = Vs
    "T",       // 41  magnetic flux density (B)     tesla			Wb/m2
    "A_m",     // 42  magnetic field strength (H)	ampere per metre	A/m
    "H",       // 43  inductance (L)		henry			        H = Wb/A
    "Hz",      // 44  frequency (f, ω)		hertz			        1/s
    "1_Wh",    // 45  R_W (active energy meter constant or pulse value)
    "1_varh",  // 46  R_B (reactive energy meter constant or pulse value)
    "1_VAh",   // 47  R_S (apparent energy meter constant or pulse value)
    "V2h",     // 48  volt-squared hour		volt-squaredhours	    V²(60*60s)
    "A2h",     // 49  ampere-squared hour	ampere-squaredhours	    A²(60*60s)
    "kg_s",    // 50  mass flux			    kilogram per second	    kg/s
    "S, mho",  // 51  conductance           siemens			        1/Ω
    "K",       // 52  temperature (T)		kelvin
    "1_V2h",   // 53  R_U²h (Volt-squared hour meter constant or pulse value)
    "1_A2h",   // 54  R_I²h	(Ampere-squared hour meter constant or pulse value)
    "1_m3",    // 55  R_V, meter constant or pulse value (volume)
    "pct",     // 56  percentage		                            %
    "Ah",      // 57  energy		        ampere-hour
    "",        // 58  dummy
    "",        // 59  dummy
    "Wh_m3",   // 60  energy per volume		3,6*103 J/m³
    "J_m3",    // 61  calorific value, wobbe
    "Molpct",  // 62  molar fraction of mole percent	(Basic gas composition unit)
    "g_m3",    // 63  mass density, quantity of material	(Gas analysis, accompanying elements)
    "Pas",     // 64  dynamic viscosity pascal second	(Characteristic of gas stream)
};

/* clang-format off */
static const struct obis_code obis_map[] = {
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
