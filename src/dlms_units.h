/*
 * Copyright (c) 2022 Martin Jäger
 *
 * SPDX-License-Identifier: Apache-2.0
 */

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

/* most relevant unit codes of above list */
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
