# Copyright 2016-2021 Lawrence Livermore National Security, LLC and other
# IREP Project Developers. See the top-level LICENSE file for details.
#
# SPDX-License-Identifier: MIT

# To use this cmakefile Lua needs to be installed, or you can add Lua's
# prefix to CMAKE_PREFIX_PATH

cmake_minimum_required(VERSION 3.1)
project(irep LANGUAGES C Fortran)

# Find lua and figure out bin dir
find_package(Lua REQUIRED)
string(REGEX REPLACE "/include$" "/bin" LUA_BIN ${LUA_INCLUDE_DIR})

# Set up IREP_GENERATE varaible
include(${CMAKE_CURRENT_SOURCE_DIR}/share/irep/irep-config.cmake)

# Be sure to include lua headers
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LUA_INCLUDE_DIR}
)

# ir_std.f & ir_extern.f is generated from coresponding
# headers using IREP_GENERATE
add_custom_command(
    OUTPUT ir_std.f
    COMMAND
        ${CMAKE_COMMAND} -E env PATH="${LUA_BIN}:$ENV{PATH}"
        ${IREP_GENERATE} --mode fortran
        ${CMAKE_CURRENT_SOURCE_DIR}/ir_std.h > ir_std.f
)

add_custom_command(
    DEPENDS ir_std.f
    OUTPUT ir_extern.f
    COMMAND
        ${CMAKE_COMMAND} -E env PATH="${LUA_BIN}:$ENV{PATH}"
        ${IREP_GENERATE} --mode fortran
        ${CMAKE_CURRENT_SOURCE_DIR}/ir_extern.h > ir_extern.f
)

# All Fortran files have free formatting
set(CMAKE_Fortran_FORMAT FREE)

# Fortran modules should be output to the build directory
set(CMAKE_Fortran_MODULE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# libIR.a is mostly basic functions from irep.c. ir_extern.f, & ir_std.f
# are very much like wkt headers. This ensures their tables are in
# libIR.a. Note that they are automatically included in index libraries
# by irep-config.cmake
add_library(
    IR STATIC
    irep.c
    ${CMAKE_CURRENT_BINARY_DIR}/ir_extern.f
    ${CMAKE_CURRENT_BINARY_DIR}/ir_std.f
)

# Install irep-generate in bin dir
install(
    PROGRAMS bin/irep-generate
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

# Install libIR.a in lib dir
install(
    TARGETS IR
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)

# Headers and Fortran modules go in include
install(
  FILES
    # headers
    ir_end.h ir_extern.h ir_index.h ir_macros.h ir_start.h
    ir_std.h ir_undef.h
    # fortran modules
    ${CMAKE_Fortran_MODULE_DIRECTORY}/ir_std.mod
    ${CMAKE_Fortran_MODULE_DIRECTORY}/ir_extern.mod
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include
)

# Install  share/irep/* in share/irep dir
install(
    FILES
        share/irep/irep-config.cmake
        share/irep/wkt.mk
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/irep
)

# Install doc files needed to build docs from an installed irep
install(
    FILES
        docs/irep_types.rst
    DESTINATION ${CMAKE_INSTALL_PREFIX}/docs
)
