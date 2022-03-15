# Copyright (c) 2022 Martin Jäger
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.10)

project(sml_parser_example)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

add_executable(parser
    main.c
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../src sml_parser_lib)
target_link_libraries(parser sml_parser_lib)
