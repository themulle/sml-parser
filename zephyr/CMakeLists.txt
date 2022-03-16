# Copyright (c) 2022 Martin Jäger
#
# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(../src)

zephyr_library_named(sml_parser)

add_subdirectory(../src build/sml_parser)
