# Copyright (c) 2022 Martin Jäger
#
# SPDX-License-Identifier: Apache-2.0

add_library(sml_parser_lib STATIC "")

target_sources(sml_parser_lib PRIVATE obis.c)
target_sources(sml_parser_lib PRIVATE sml_parser.c)
