# Copyright 2016-2021 Lawrence Livermore National Security, LLC and other
# IREP Project Developers. See the top-level LICENSE file for details.
#
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.3)
project(irep-cxx-test LANGUAGES C CXX Fortran)

find_package(irep REQUIRED)

find_package(Lua REQUIRED)
include_directories("${LUA_INCLUDE_DIR}")

add_wkt_library(prog-wkt wkt_table1.h wkt_table4.h)
add_wkt_index_library(prog-wkt-index wkt_table1.h wkt_table4.h)

add_executable(cxx_prog cxx_main.cpp)
target_link_libraries(
  cxx_prog
  prog-wkt
  ${IREP_LIBRARIES}
  prog-wkt-index
  prog-wkt
  ${LUA_LIBRARIES}
)
