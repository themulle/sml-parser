/*
 * Copyright (c) 2022 Martin Jäger
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SML_PARSER_H_
#define SML_PARSER_H_

#include <math.h>
#include <stdint.h>
#include <string.h>

#ifndef NAN
/* for some reason Zephyr does not know NAN even though math.h is included */
#define NAN (__builtin_nanf(""))
#endif

#define SML_ESCAPE_CHAR   0x1b
#define SML_VERSION1_CHAR 0x01

#define SML_MSG_BODY_PUBLIC_OPEN_REQ      0x00000100
#define SML_MSG_BODY_PUBLIC_OPEN_RES      0x00000101
#define SML_MSG_BODY_PUBLIC_CLOSE_REQ     0x00000200
#define SML_MSG_BODY_PUBLIC_CLOSE_RES     0x00000201
#define SML_MSG_BODY_GET_PROFILE_PACK_REQ 0x00000300
#define SML_MSG_BODY_GET_PROFILE_PACK_RES 0x00000301
#define SML_MSG_BODY_GET_PROFILE_LIST_REQ 0x00000400
#define SML_MSG_BODY_GET_PROFILE_LIST_RES 0x00000401
#define SML_MSG_BODY_GET_PROC_PARAM_REQ   0x00000500
#define SML_MSG_BODY_GET_PROC_PARAM_RES   0x00000501
#define SML_MSG_BODY_SET_PROC_PARAM_REQ   0x00000600
#define SML_MSG_BODY_SET_PROC_PARAM_RES   0x00000601
#define SML_MSG_BODY_GET_LIST_REQ         0x00000700
#define SML_MSG_BODY_GET_LIST_RES         0x00000701
#define SML_MSG_BODY_GET_COSEM_REQ        0x00000800
#define SML_MSG_BODY_GET_COSEM_RES        0x00000801
#define SML_MSG_BODY_SET_COSEM_REQ        0x00000900
#define SML_MSG_BODY_SET_COSEM_RES        0x00000901
#define SML_MSG_BODY_ACTION_COSEM_REQ     0x00000A00
#define SML_MSG_BODY_ACTION_COSEM_RES     0x00000A01
#define SML_MSG_BODY_ATTENTION_RES        0x0000FF01

#define SML_END_OF_MESSAGE 0x00
#define SML_TYPE_OPTIONAL  0x01

#define SML_TYPE_OCTET_STRING      0x00
#define SML_TYPE_OCTET_STRING_MASK 0x70

#define SML_TYPE_BOOL   0x42
#define SML_TYPE_INT8   0x52
#define SML_TYPE_INT16  0x53
#define SML_TYPE_INT32  0x55
#define SML_TYPE_INT64  0x59
#define SML_TYPE_UINT8  0x62
#define SML_TYPE_UINT16 0x63
#define SML_TYPE_UINT32 0x65
#define SML_TYPE_UINT64 0x69

#define SML_TYPE_INT       0x50
#define SML_TYPE_INT_MASK  0x50
#define SML_TYPE_UINT      0x60
#define SML_TYPE_UINT_MASK 0x60

#define SML_TYPE_LIST_OF      0x70
#define SML_TYPE_LIST_OF_MASK 0x70

#define SML_LENGTH_MASK 0x0F

#define SML_TL_SINGLE        0x00 /* type information is stored in single byte */
#define SML_TL_SINGLE_MASK   0x80
#define SML_TL_EXTENDED      0x80 /* indicates that a length > 15 bytes */
#define SML_TL_EXTENDED_MASK 0x80

#define SML_ERR_GENERIC          -1
#define SML_ERR_ESCAPE_SEQ       -2
#define SML_ERR_VERSION          -3
#define SML_ERR_INCOMPLETE       -4
#define SML_ERR_FORMAT           -5
#define SML_ERR_MEMORY           -6
#define SML_ERR_BUFFER_TOO_SMALL -7

/*
 * float values of NaN and integers of positive max mean that the variable is not set.
 */
struct sml_values_electricity
{
    /* uint32_t will overflow after 50 years with 9,8 kW average power */
    uint32_t energy_import_active_Wh;
    uint32_t energy_export_active_Wh;

    float frequency_Hz;
    float power_active_W;

    float voltage_l1_V;
    float voltage_l2_V;
    float voltage_l3_V;

    float current_l1_A;
    float current_l2_A;
    float current_l3_A;

    int16_t phase_shift_l1_deg;
    int16_t phase_shift_l2_deg;
    int16_t phase_shift_l3_deg;
};

struct sml_context
{
    uint8_t *sml_buf;
    size_t sml_buf_len;
    int sml_buf_pos;
    struct sml_values_electricity *values_electricity;
};

/**
 * Main SML parser function
 *
 * Processes the provided SML data buffer (can contain multiple SML files) and stores the values
 * inside the struct sml_values_electricity.
 *
 * @param sml SML context containing buffer information
 */
int sml_parse(struct sml_context *sml);

void sml_debug_print(struct sml_context *ctx);

#endif /* SML_PARSER_H_ */
