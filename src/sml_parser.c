/*
 * Copyright (c) 2022 Martin Jäger
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sml_parser.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "obis.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#endif

/**
 * Retrieve actual length of value excluding the length of the TL byte itself
 *
 * @param ctx SML context
 * @param length Pointer to the variable to store the result
 *
 * @returns Always 0 (currently no failure case known)
 */
static int sml_deserialize_length(struct sml_context *ctx, uint8_t *length)
{
    uint8_t first_byte = ctx->sml_buf[ctx->sml_buf_pos];
    uint32_t len_read = 0;
    uint8_t len_tl = 0;

    // limit loop to max. 8 extended length bytes to prevent issues with erroneous data
    for (int i = 0; i < 8; i++) {
        uint8_t byte = ctx->sml_buf[ctx->sml_buf_pos];
        len_read = (len_read << 4) + (byte & SML_LENGTH_MASK);
        ctx->sml_buf_pos++;
        len_tl++;
        if ((byte & SML_TL_SINGLE_MASK) == SML_TL_SINGLE) {
            break;
        }
    }

    if ((first_byte & SML_TYPE_LIST_OF_MASK) == SML_TYPE_LIST_OF
        || first_byte == SML_END_OF_MESSAGE) {
        *length = len_read;
    }
    else {
        *length = len_read - len_tl;
    }

    return 0;
}

/**
 * Deserialize SML octet string (byte array)
 *
 * @param ctx SML context
 * @param buf Buffer to store the result
 * @param size Size of that buffer
 *
 * @returns Actual length of octet string or negative value in case of error
 */
static int sml_deserialize_octet_string(struct sml_context *ctx, uint8_t *buf, size_t size)
{
    uint8_t len = 0;

    int ret = sml_deserialize_length(ctx, &len);
    if (ret < 0) {
        return ret;
    }

    if (len < size) {
        memcpy(buf, ctx->sml_buf + ctx->sml_buf_pos, len);
        ctx->sml_buf_pos += len;
        return len;
    }
    else {
        ctx->sml_buf_pos += len;
        return SML_ERR_BUFFER_TOO_SMALL;
    }
}

/**
 * Deserialize SML end of message byte
 *
 * @param ctx SML context
 *
 * @returns 0 for success or negative value in case of error
 */
static int sml_deserialize_end_of_message(struct sml_context *ctx)
{
    if (ctx->sml_buf[ctx->sml_buf_pos] == SML_END_OF_MESSAGE) {
        ctx->sml_buf_pos++;
        return 0;
    }

    return SML_ERR_GENERIC;
}

/**
 * Deserialize SML boolean value
 *
 * @param ctx SML context
 * @param value Pointer to the variable to store the result
 *
 * @returns 0 for success or negative value in case of error
 */
static int sml_deserialize_bool(struct sml_context *ctx, bool *value)
{
    if (ctx->sml_buf[ctx->sml_buf_pos] == SML_TYPE_BOOL) {
        *value = (ctx->sml_buf[ctx->sml_buf_pos + 1] != 0);
        ctx->sml_buf_pos += 2;
        return 0;
    }

    return SML_ERR_GENERIC;
}

/**
 * Deserialize SML unsigned integer value
 *
 * @param ctx SML context
 * @param value Pointer to the variable to store the result
 *
 * @returns 0 for success or negative value in case of error
 */
static int sml_deserialize_uint64(struct sml_context *ctx, uint64_t *value)
{
    int num_bytes = (ctx->sml_buf[ctx->sml_buf_pos] & SML_LENGTH_MASK) - 1;
    ctx->sml_buf_pos++;

    *value = 0;
    for (int i = 0; i < num_bytes; i++) {
        *value = (*value << 8) + ctx->sml_buf[ctx->sml_buf_pos];
        ctx->sml_buf_pos++;
    }

    return 0;
}

/**
 * Deserialize SML integer value
 *
 * @param ctx SML context
 * @param value Pointer to the variable to store the result
 *
 * @returns 0 for success or negative value in case of error
 */
static int sml_deserialize_int64(struct sml_context *ctx, int64_t *value)
{
    int num_bytes = (ctx->sml_buf[ctx->sml_buf_pos] & SML_LENGTH_MASK) - 1;
    ctx->sml_buf_pos++;

    bool negative = (ctx->sml_buf[ctx->sml_buf_pos] & SML_TYPE_INT_MASK) == SML_TYPE_INT
                    && ctx->sml_buf[ctx->sml_buf_pos] >= 0x80;

    *value = 0;
    for (int i = 0; i < num_bytes; i++) {
        *value = (*value << 8) + ctx->sml_buf[ctx->sml_buf_pos];
        ctx->sml_buf_pos++;
    }

    if (negative) {
        /* fill remaining bytes with 0xFF */
        for (int i = num_bytes; i < 8; i++) {
            *value |= ((uint64_t)0xFF) << (8 * i);
        }
    }

    return 0;
}

/**
 * Skip next SML element
 *
 * @param ctx SML context
 *
 * @returns 0 for success or negative value in case of error
 */
static int sml_skip_element(struct sml_context *ctx)
{
    uint8_t byte = ctx->sml_buf[ctx->sml_buf_pos];
    uint8_t len = 0;

    if (sml_deserialize_length(ctx, &len) < 0) {
        return SML_ERR_GENERIC;
    }

    if ((byte & SML_TYPE_LIST_OF_MASK) == SML_TYPE_LIST_OF) {
        // printf("skipping list of %d elements at 0x%x\n", len, ctx->sml_buf_pos);
        for (int i = 0; i < len; i++) {
            sml_skip_element(ctx);
        }
    }
    else {
        // printf("skipping %d bytes for 0x%x at 0x%x\n", len, byte, ctx->sml_buf_pos);
        ctx->sml_buf_pos += len;
        return 0;
    }

    return SML_ERR_GENERIC;
}

static uint32_t sml_scale_uint32(int64_t number, int scaler)
{
    if (scaler < 0) {
        // negative scaler: reduce resolution
        int divider = 10;
        for (int i = -1; i > scaler; i--) {
            divider *= 10;
        }
        number /= divider;
    }
    else {
        // positive scaler: append zeros at the end
        for (int i = 0; i < scaler; i++) {
            number *= 10;
        }
    }

    return (uint32_t)number;
}

static int32_t sml_scale_int32(int64_t number, int scaler)
{
    if (scaler < 0) {
        // negative scaler: reduce resolution
        int divider = 10;
        for (int i = -1; i > scaler; i--) {
            divider *= 10;
        }
        number /= divider;
    }
    else {
        // positive scaler: append zeros at the end
        for (int i = 0; i < scaler; i++) {
            number *= 10;
        }
    }

    return number;
}

static float sml_scale_float(int64_t number, int scaler)
{
    if (scaler < 0) {
        int divider = 10;
        for (int i = -1; i > scaler; i--) {
            divider *= 10;
        }
        return (float)number / divider;
    }
    else {
        int factor = 1;
        for (int i = 0; i < scaler; i++) {
            number *= 10;
        }
        return (float)number * factor;
    }
}

static int sml_store_number(struct sml_context *ctx, int64_t number, uint32_t obis_short,
                            int scaler, uint8_t unit)
{
    switch (obis_short) {
        case OBIS_ELECTRICITY_IMPORT_ACTIVE_ENERGY_TOTAL:
            if (unit == DLMS_UNIT_WATT_HOUR) {
                ctx->values_electricity->energy_import_active_Wh = sml_scale_uint32(number, scaler);
            }
            break;
        case OBIS_ELECTRICITY_IMPORT_ACTIVE_ENERGY_TARIFF_1:
            if (unit == DLMS_UNIT_WATT_HOUR
                && ctx->values_electricity->energy_import_active_Wh == UINT32_MAX) {
                ctx->values_electricity->energy_import_active_Wh = sml_scale_uint32(number, scaler);
            }
            break;
        case OBIS_ELECTRICITY_EXPORT_ACTIVE_ENERGY_TOTAL:
            if (unit == DLMS_UNIT_WATT_HOUR) {
                ctx->values_electricity->energy_export_active_Wh = sml_scale_uint32(number, scaler);
            }
            break;
        case OBIS_ELECTRICITY_EXPORT_ACTIVE_ENERGY_TARIFF_1:
            if (unit == DLMS_UNIT_WATT_HOUR
                && ctx->values_electricity->energy_export_active_Wh == UINT32_MAX) {
                ctx->values_electricity->energy_export_active_Wh = sml_scale_uint32(number, scaler);
            }
            break;
        case OBIS_ELECTRICITY_FREQUENCY:
            if (unit == DLMS_UNIT_WATT_HOUR) {
                ctx->values_electricity->frequency_Hz = sml_scale_float(number, scaler);
            }
            break;
        case OBIS_ELECTRICITY_IMPORT_ACTIVE_POWER_TOTAL:
        case OBIS_ELECTRICITY_ACTIVE_POWER:
        case OBIS_ELECTRICITY_ACTIVE_POWER_DELTA:
            if (unit == DLMS_UNIT_WATT) {
                ctx->values_electricity->power_active_W = sml_scale_float(number, scaler);
            }
            break;
        case OBIS_ELECTRICITY_L1_CURRENT:
            if (unit == DLMS_UNIT_AMPERE) {
                ctx->values_electricity->current_l1_A = sml_scale_float(number, scaler);
            }
            break;
        case OBIS_ELECTRICITY_L2_CURRENT:
            if (unit == DLMS_UNIT_AMPERE) {
                ctx->values_electricity->current_l2_A = sml_scale_float(number, scaler);
            }
            break;
        case OBIS_ELECTRICITY_L3_CURRENT:
            if (unit == DLMS_UNIT_AMPERE) {
                ctx->values_electricity->current_l3_A = sml_scale_float(number, scaler);
            }
            break;
        case OBIS_ELECTRICITY_L1_VOLTAGE:
            if (unit == DLMS_UNIT_VOLT) {
                ctx->values_electricity->voltage_l1_V = sml_scale_float(number, scaler);
            }
            break;
        case OBIS_ELECTRICITY_L2_VOLTAGE:
            if (unit == DLMS_UNIT_VOLT) {
                ctx->values_electricity->voltage_l2_V = sml_scale_float(number, scaler);
            }
            break;
        case OBIS_ELECTRICITY_L3_VOLTAGE:
            if (unit == DLMS_UNIT_VOLT) {
                ctx->values_electricity->voltage_l3_V = sml_scale_float(number, scaler);
            }
            break;
        case OBIS_ELECTRICITY_IL1_UL1_PHASE_ANGLE:
            if (unit == DLMS_UNIT_DEGREE) {
                ctx->values_electricity->phase_shift_l1_deg = sml_scale_int32(number, scaler);
            }
            break;
        case OBIS_ELECTRICITY_IL2_UL2_PHASE_ANGLE:
            if (unit == DLMS_UNIT_DEGREE) {
                ctx->values_electricity->phase_shift_l2_deg = sml_scale_int32(number, scaler);
            }
            break;
        case OBIS_ELECTRICITY_IL3_UL3_PHASE_ANGLE:
            if (unit == DLMS_UNIT_DEGREE) {
                ctx->values_electricity->phase_shift_l3_deg = sml_scale_int32(number, scaler);
            }
            break;
    }

    return 0;
}

static int sml_deserialize_value(struct sml_context *ctx, uint32_t obis_short, int scaler,
                                 uint8_t unit)
{
    uint8_t tl = ctx->sml_buf[ctx->sml_buf_pos];
    if ((tl & SML_TYPE_OCTET_STRING_MASK) == SML_TYPE_OCTET_STRING) {
        char str[32];
        int ret = sml_deserialize_octet_string(ctx, (uint8_t *)str, sizeof(str) - 1);
        if (ret < 0 && ret != SML_ERR_BUFFER_TOO_SMALL) {
            printf("deserializing octet string %x at pos %x failed\n", tl, ctx->sml_buf_pos);
            return SML_ERR_GENERIC;
        }
    }
    else if ((tl & SML_TYPE_INT_MASK) == SML_TYPE_INT) {
        int64_t i64;
        sml_deserialize_int64(ctx, &i64);
        sml_store_number(ctx, i64, obis_short, scaler, unit);
    }
    else if ((tl & SML_TYPE_UINT_MASK) == SML_TYPE_UINT) {
        uint64_t u64;
        sml_deserialize_uint64(ctx, &u64);
        sml_store_number(ctx, u64, obis_short, scaler, unit);
    }
    else if (tl == SML_TYPE_BOOL) {
        bool b;
        sml_deserialize_bool(ctx, &b);
    }
    else {
        printf("unknown type: 0x%x\n", ctx->sml_buf[ctx->sml_buf_pos]);
        sml_skip_element(ctx);
    }

    return 0;
}

/**
 * Deserialize SML list entry
 *
 * List entries contain the actual data points we are interested in.
 *
 * @param ctx SML context
 *
 * @returns 0 for success or negative value in case of error
 */
static int sml_deserialize_list_entry(struct sml_context *ctx)
{
    int ret;

    uint8_t obj_name[8];
    int obj_name_len = sml_deserialize_octet_string(ctx, obj_name, sizeof(obj_name));
    if (obj_name_len < 0) {
        printf("deserializing obj_name failed\n");
        return SML_ERR_GENERIC;
    }

    sml_skip_element(ctx); // status
    sml_skip_element(ctx); // valTime

    if (obj_name_len == 1 + 6) { // name is a valid obis code
        uint32_t obis_short = OBIS_CODE_SHORT(obj_name[1], obj_name[3], obj_name[4], obj_name[5]);

        uint64_t unit;
        ret = sml_deserialize_uint64(ctx, &unit);
        if (ret < 0) {
            printf("deserializing unit string failed\n");
            return SML_ERR_GENERIC;
        }

        int64_t scaler = 0;
        ret = sml_deserialize_int64(ctx, &scaler);
        if (ret < 0) {
            printf("deserializing scaler failed\n");
            return SML_ERR_GENERIC;
        }

        ret = sml_deserialize_value(ctx, obis_short, (int)scaler, (uint8_t)unit);
        if (ret < 0) {
            printf("deserializing value failed\n");
            return SML_ERR_GENERIC;
        }

        // for debugging purposes
        // obis_print_object_name(obj_name + 1, obj_name_len - 1, unit, scaler);
    }
    else {
        sml_skip_element(ctx); // unit
        sml_skip_element(ctx); // scaler
        sml_skip_element(ctx); // value
    }

    sml_skip_element(ctx); // valueSignature

    return 0;
}

/**
 * Deserialize SML list
 *
 * @param ctx SML context
 *
 * @returns 0 for success or negative value in case of error
 */
static int sml_deserialize_list(struct sml_context *ctx)
{
    uint8_t len = 0;
    if (sml_deserialize_length(ctx, &len) < 0 || len != 7) {
        printf("Error: Message length for list not correct\n");
        return SML_ERR_GENERIC;
    }

    sml_skip_element(ctx); // clientId
    sml_skip_element(ctx); // serverId
    sml_skip_element(ctx); // listName
    sml_skip_element(ctx); // actSensorTime

    uint8_t num_entries = 0;
    if (sml_deserialize_length(ctx, &num_entries) < 0) {
        printf("Error: List length not correct\n");
        return SML_ERR_GENERIC;
    }
    for (int i = 0; i < num_entries; i++) {
        sml_deserialize_list_entry(ctx);
    }

    sml_skip_element(ctx); // listSignature
    sml_skip_element(ctx); // actGatewayTime

    return 0;
}

/**
 * Deserialize SML message body
 *
 * @param ctx SML context
 *
 * @returns Message body tag in case of success or negative value in case of error
 */
static int sml_deserialize_msg_body(struct sml_context *ctx)
{
    uint8_t len = 0;
    if (sml_deserialize_length(ctx, &len) < 0 || len != 2) {
        printf("Error: Message body length not correct\n");
        return SML_ERR_GENERIC;
    }

    uint64_t tag;
    if (sml_deserialize_uint64(ctx, &tag) < 0) {
        printf("Error parsing tag\n");
        return SML_ERR_GENERIC;
    }

    switch (tag) {
        case SML_MSG_BODY_PUBLIC_OPEN_RES:
            // printf("open res\n");
            sml_skip_element(ctx);
            break;
        case SML_MSG_BODY_GET_LIST_RES:
            // printf("get list res\n");
            sml_deserialize_list(ctx);
            break;
        case SML_MSG_BODY_PUBLIC_CLOSE_RES:
            // printf("close res\n");
            sml_skip_element(ctx);
            break;
        default:
            printf("unknown msg body: 0x%x\n", (uint32_t)tag);
    }

    return (int)tag; // safe because tags are only 16-bit
}

/**
 * Deserialize SML message
 *
 * Essentially skips anything except for the message body.
 *
 * @param ctx SML context
 *
 * @returns Message body tag in case of success or negative value in case of error
 */
static int sml_parse_msg(struct sml_context *ctx)
{
    uint8_t len = 0;
    if (sml_deserialize_length(ctx, &len) < 0 || len != 6) {
        printf("Error: Message length %d at pos 0x%x not correct\n", len, ctx->sml_buf_pos);
        return SML_ERR_FORMAT;
    }

    sml_skip_element(ctx); // transactionId
    sml_skip_element(ctx); // groupNo
    sml_skip_element(ctx); // abortOnError

    int msg_body_tag = sml_deserialize_msg_body(ctx);
    if (msg_body_tag < 0) {
        printf("error parsing message body\n");
        return msg_body_tag;
    }

    sml_skip_element(ctx); // crc16

    if (sml_deserialize_end_of_message(ctx) < 0) {
        return SML_ERR_FORMAT;
    }

    return msg_body_tag;
}

/**
 * Deserialize SML file
 *
 * An SML file can contain multiple messages.
 *
 * @param ctx SML context
 *
 * @returns Message body tag in case of success or negative value in case of error
 */
static int sml_parse_file(struct sml_context *ctx)
{
    while (true) {
        int msg_body_tag = sml_parse_msg(ctx);
        if (msg_body_tag == SML_MSG_BODY_PUBLIC_CLOSE_RES) {
            return 0;
        }
        else if (msg_body_tag < 0) {
            return msg_body_tag;
        }
    }

    return 0;
}

/**
 * Set values to agreed value meaning the measurement is not available from the meter.
 */
static void sml_init_elctricity(struct sml_context *ctx)
{
    ctx->values_electricity->energy_import_active_Wh = UINT32_MAX;
    ctx->values_electricity->energy_export_active_Wh = UINT32_MAX;

    ctx->values_electricity->frequency_Hz = NAN;
    ctx->values_electricity->power_active_W = NAN;

    ctx->values_electricity->voltage_l1_V = NAN;
    ctx->values_electricity->voltage_l2_V = NAN;
    ctx->values_electricity->voltage_l3_V = NAN;

    ctx->values_electricity->current_l1_A = NAN;
    ctx->values_electricity->current_l2_A = NAN;
    ctx->values_electricity->current_l3_A = NAN;

    ctx->values_electricity->phase_shift_l1_deg = INT16_MAX;
    ctx->values_electricity->phase_shift_l2_deg = INT16_MAX;
    ctx->values_electricity->phase_shift_l3_deg = INT16_MAX;
}

/* only public API of the parser, see header for description */
int sml_parse(struct sml_context *ctx)
{
    if (ctx->sml_buf == NULL || ctx->values_electricity == NULL) {
        return SML_ERR_MEMORY;
    }

    sml_init_elctricity(ctx);

    if (ctx->sml_buf_pos>=ctx->sml_buf_len && (ctx->sml_buf_len-ctx->sml_buf_pos) <= 16 ) {
        /* buffer position has to be valid (double check because of unsigned overflow) */
        /* at least the escape sequences at beginning and end are needed in the remaining buffer */
        return SML_ERR_INCOMPLETE;
    }

    // printf("Parsing SML file at pos 0x%x\n", ctx->sml_buf_pos);

    /* check escape sequence */
    for (int i = 0; i < 4; i++) {
        if (ctx->sml_buf[ctx->sml_buf_pos] != SML_ESCAPE_CHAR) {
            return SML_ERR_ESCAPE_SEQ;
        }
        ctx->sml_buf_pos++;
    }

    /* check version number */
    for (int i = 0; i < 4; i++) {
        if (ctx->sml_buf[ctx->sml_buf_pos] != SML_VERSION1_CHAR) {
            return SML_ERR_VERSION;
        }
        ctx->sml_buf_pos++;
    }

    int ret = sml_parse_file(ctx);

    /* skip padding */
    ctx->sml_buf_pos += 4 - (ctx->sml_buf_pos % 4);

    /* skip final escape and CRC */
    ctx->sml_buf_pos += 8;

    return ret;
}

void sml_debug_print(struct sml_context *ctx)
{
    struct sml_values_electricity *electricity = ctx->values_electricity;

    if (electricity->energy_import_active_Wh != UINT32_MAX) {
        printf("ImpAct_Wh:%u ", electricity->energy_import_active_Wh);
    }
    if (electricity->energy_export_active_Wh != UINT32_MAX) {
        printf("ExpAct_Wh:%u ", electricity->energy_export_active_Wh);
    }
    if (electricity->frequency_Hz != NAN) {
        printf("Freq_Hz:%.1f ", electricity->frequency_Hz);
    }
    if (electricity->power_active_W != NAN) {
        printf("PwrAct_W:%.1f ", electricity->power_active_W);
    }
    if (electricity->voltage_l1_V != NAN) {
        printf("L1_V:%.1f L2_V:%.1f L3_V:%.1f ", electricity->voltage_l1_V,
               electricity->voltage_l2_V, electricity->voltage_l3_V);
    }
    if (electricity->current_l1_A != NAN) {
        printf("L1_A:%.1f L2_A:%.1f L3_A:%.1f ", electricity->current_l1_A,
               electricity->current_l2_A, electricity->current_l3_A);
    }
    if (electricity->phase_shift_l1_deg != INT16_MAX) {
        printf("L1_deg:%d L2_deg:%d L3_deg:%d ", electricity->phase_shift_l1_deg,
               electricity->phase_shift_l2_deg, electricity->phase_shift_l3_deg);
    }
    printf("\n");
}
