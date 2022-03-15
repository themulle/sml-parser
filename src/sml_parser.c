/*
 * Copyright (c) 2022 Martin Jäger
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sml_parser.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "dlms_units.h"
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

    return SML_ERR_GENERIC;
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
 * Scale JSON number
 *
 * Hack to avoid using floating-point math and printing for scaled values
 *
 * @param buf JSON buffer
 * @param buf_size Size of JSON buffer
 * @param pos Position of the end of the number in the buffer
 * @param scaler Exponent to the basis of 10 for scaling the result
 *
 * @returns 0 for success or negative value in case of error
 */
static int json_scale_number(char *buf, size_t buf_size, int pos, int scaler)
{
    if (pos > 0 && scaler != 0) {
        if (scaler < 0) {
            // negative scaler: move digits to the right and insert decimal dot
            int offset = (-scaler == pos) ? 2 : 1;
            for (int i = pos + offset; i > pos + scaler; i--) {
                buf[i] = buf[i - offset];
            }
            if (offset == 2) {
                buf[0] = '0';
                buf[1] = '.';
            }
            else {
                buf[pos + scaler] = '.';
            }
            pos += offset;
        }
        else {
            // positive scaler: append zeros at the end
            if (buf_size - pos < scaler) {
                return -1;
            }
            else {
                for (int i = pos; i < pos + scaler; i++) {
                    buf[pos++] = '0';
                }
            }
        }
    }

    return pos;
}

/**
 * Serialize integer value to JSON string
 *
 * @param buf JSON buffer
 * @param buf_size Size of JSON buffer
 * @param value Pointer to the variable to store the result
 * @param scaler Exponent to the basis of 10 for scaling the result
 *
 * @returns 0 for success or negative value in case of error
 */
static int json_serialize_int64(char *buf, size_t buf_size, const int64_t *value, int scaler)
{
    int pos = snprintf(buf, buf_size - 1, "%" PRIi64, *value);

    return json_scale_number(buf, buf_size, pos, scaler);
}

/**
 * Serialize unsigned integer value to JSON string
 *
 * @param buf JSON buffer
 * @param buf_size Size of JSON buffer
 * @param value Pointer to the variable to store the result
 * @param scaler Exponent to the basis of 10 for scaling the result
 *
 * @returns 0 for success or negative value in case of error
 */
static int json_serialize_uint64(char *buf, size_t buf_size, const uint64_t *value, int scaler)
{
    int pos = snprintf(buf, buf_size - 1, "%" PRIu64, *value);

    return json_scale_number(buf, buf_size, pos, scaler);
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
    size_t pos = ctx->sml_buf_pos;
    uint8_t len = 0;

    if (sml_deserialize_length(ctx, &len) < 0) {
        return SML_ERR_GENERIC;
    }

    if ((byte & SML_TYPE_LIST_OF_MASK) == SML_TYPE_LIST_OF) {
        // printf("skipping list of %d elements at 0x%x\n", len, pos);
        for (int i = 0; i < len; i++) {
            sml_skip_element(ctx);
        }
    }
    else {
        // printf("skipping %d bytes for 0x%x at 0x%x\n", len, byte, pos);
        ctx->sml_buf_pos += len;
        return 0;
    }

    return SML_ERR_GENERIC;
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
    ret = sml_deserialize_octet_string(ctx, obj_name, sizeof(obj_name));
    if (ret < 0) {
        printf("deserializing obj_name failed\n");
        return SML_ERR_GENERIC;
    }
    int name_len =
        obis_decode_object_name(&obj_name[1], ret - 1, &ctx->json_buf[ctx->json_buf_pos + 1],
                                ctx->json_buf_size - ctx->json_buf_pos - 2, false);
    if (name_len > 0) {
        ctx->json_buf[ctx->json_buf_pos++] = '"';
        ctx->json_buf_pos += name_len;
    }

    sml_skip_element(ctx); // status
    sml_skip_element(ctx); // valTime

    if (name_len > 0) {
        uint64_t unit;
        ret = sml_deserialize_uint64(ctx, &unit);
        if (ret < 0) {
            printf("deserializing unit string failed\n");
            return SML_ERR_GENERIC;
        }
        else {
            if (unit < ARRAY_SIZE(dlms_units) && unit != 0) {
                int pos = 0;
                pos = snprintf(&ctx->json_buf[ctx->json_buf_pos],
                               ctx->json_buf_size - ctx->json_buf_pos, "_%s", dlms_units[unit]);
                if (pos > 0) {
                    ctx->json_buf_pos += pos;
                }
                else {
                    return SML_ERR_GENERIC;
                }
            }
            ctx->json_buf[ctx->json_buf_pos++] = '"';
            ctx->json_buf[ctx->json_buf_pos++] = ':';
        }
    }
    else {
        sml_skip_element(ctx); // unit
    }

    int64_t scaler = 0;
    ret = sml_deserialize_int64(ctx, &scaler);
    if (ret < 0) {
        printf("deserializing scaler failed\n");
        return SML_ERR_GENERIC;
    }

    if (name_len > 0) {
        int pos = 0;
        uint8_t tl = ctx->sml_buf[ctx->sml_buf_pos];
        if ((tl & SML_TYPE_OCTET_STRING_MASK) == SML_TYPE_OCTET_STRING) {
            char str[32];
            int ret = sml_deserialize_octet_string(ctx, (uint8_t *)str, sizeof(str) - 1);
            if (ret < 0) {
                printf("%d: deserializing octet string failed\n", tl);
                return SML_ERR_GENERIC;
            }
            str[ret] = '\0';
            pos = snprintf(ctx->json_buf + ctx->json_buf_pos,
                           ctx->json_buf_size - ctx->json_buf_pos - 1, "\"%s\"", str);
        }
        else if ((tl & SML_TYPE_INT_MASK) == SML_TYPE_INT) {
            int64_t i64;
            sml_deserialize_int64(ctx, &i64);
            pos = json_serialize_int64(ctx->json_buf + ctx->json_buf_pos,
                                       ctx->json_buf_size - ctx->json_buf_pos - 1, &i64, scaler);
        }
        else if ((tl & SML_TYPE_UINT_MASK) == SML_TYPE_UINT) {
            uint64_t u64;
            sml_deserialize_uint64(ctx, &u64);
            pos = json_serialize_uint64(ctx->json_buf + ctx->json_buf_pos,
                                        ctx->json_buf_size - ctx->json_buf_pos - 1, &u64, scaler);
        }
        else if (tl == SML_TYPE_BOOL) {
            bool b;
            sml_deserialize_bool(ctx, &b);
            pos = snprintf(ctx->json_buf + ctx->json_buf_pos,
                           ctx->json_buf_size - ctx->json_buf_pos - 1, "\"%s\"",
                           b ? "true" : "false");
        }
        else {
            printf("unknown type: 0x%x\n", ctx->sml_buf[ctx->sml_buf_pos]);
            sml_skip_element(ctx);
        }
        if (pos > 0) {
            ctx->json_buf[ctx->json_buf_pos + pos] = ',';
            ctx->json_buf_pos += pos + 1;
        }
    }
    else {
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
            printf("unknown msg body: 0x%x\n", tag);
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
    sml_skip_element(ctx); // endOfSmlMsg

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
    size_t json_buf_pos_init = ctx->json_buf_pos;

    ctx->json_buf[ctx->json_buf_pos++] = '{';

    while (true) {
        int msg_body_tag = sml_parse_msg(ctx);
        if (msg_body_tag == SML_MSG_BODY_PUBLIC_CLOSE_RES) {
            ctx->json_buf[ctx->json_buf_pos - 1] = '}';
            ctx->json_buf[ctx->json_buf_pos] = '\0';
            return 0;
        }
        else if (msg_body_tag < 0) {
            ctx->json_buf_pos = json_buf_pos_init;
            ctx->json_buf[ctx->json_buf_pos - 1] = '\0';
            return msg_body_tag;
        }
    }

    return 0;
}

/* only public API of the parser, see header for description */
int sml_parse(struct sml_context *ctx)
{
    if (ctx->sml_buf_len < 16) {
        /* at least the escape sequences at beginning and end are needed */
        return SML_ERR_INCOMPLETE;
    }

    while (ctx->sml_buf_pos < ctx->sml_buf_len) {

        printf("Parsing SML file at pos 0x%x\n", ctx->sml_buf_pos);

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

        sml_parse_file(ctx);

        printf("%s\n", ctx->json_buf);
        ctx->json_buf_pos = 0; // reset JSON buffer

        /* skip padding */
        ctx->sml_buf_pos += 4 - (ctx->sml_buf_pos % 4);

        /* skip final escape and CRC */
        ctx->sml_buf_pos += 8;
    }

    return 0;
}
