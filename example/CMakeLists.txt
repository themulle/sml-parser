# Copyright (c) 2022 Martin Jäger
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.10)

project(sml_parser_example)

add_executable(sml_parser
    main.c
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../src src)
