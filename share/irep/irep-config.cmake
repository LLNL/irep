# Copyright 2016-2021 Lawrence Livermore National Security, LLC and other
# IREP Project Developers. See the top-level LICENSE file for details.
#
# SPDX-License-Identifier: MIT

# IREP lives two directories above this config file
get_filename_component(irep_DIR ${CMAKE_CURRENT_LIST_DIR}/../.. ABSOLUTE)

# location of the irep-generate executable
set(IREP_GENERATE ${irep_DIR}/bin/irep-generate)

# location of the irep-generate executable
set(IREP_LIBRARIES ${irep_DIR}/lib/libIR.a)

# location of the irep-generate executable
set(IREP_INCLUDE_DIR ${irep_DIR}/include)

# add_wkt_library()
#
# Generate Fortran modules and a WKT library from wkt_*.h headers.
#
# Usage:
#
#     add_wkt_library(
#         name-wkt                     # name of WKT library
#         wkt_foo.h wkt_bar.h ...      # non-generated wkt headers
#         [GENERATED wkt_gen1.h ...]   # generated wkt headers (optional)
#     )
#
# Output variables:
#     NAME_FFILES    Fortran files that went into libname-wkt.a
#     NAME_MODFILES  Fortran modules generated from NAME_FFILES
#
function(add_wkt_library name)
  cmake_parse_arguments(WKT_LIB "" "" "GENERATED" ${ARGN})
  set(WKT_HEADERS ${WKT_LIB_UNPARSED_ARGUMENTS})

  set(WKT_FFILES "")
  set(WKT_MODFILES "")
  list(APPEND WKT_HEADERS "${WKT_LIB_GENERATED}")
  foreach(WKT_H ${WKT_HEADERS})
    if(NOT WKT_H MATCHES ".h$")
      message(FATAL_ERROR "Invalid WKT header name: '${WKT_H}'")
    endif()

    # get abspath to WKT_H to handle wkt's in source dir correctly
    get_filename_component(WKT_H "${WKT_H}" ABSOLUTE)

    string(REGEX REPLACE ".h$" ".f" WKT_F "${WKT_H}")
    get_filename_component(WKT_F "${WKT_F}" NAME)
    list(APPEND WKT_FFILES "${WKT_F}")

    string(REGEX REPLACE ".h$" ".mod" WKT_MOD "${WKT_H}")
    get_filename_component(WKT_MOD "${WKT_MOD}" NAME)
    list(APPEND WKT_MODFILES "${WKT_MOD}")

    set_source_files_properties(
      ${WKT_F} PROPERTIES
      Fortran_FORMAT FREE
      COMPILE_FLAGS -DIREP_LANG_FORTRAN -assume bscc
    )

    # ensure that CPPFLAGS is set to include lib target properties
    set(incl "$<TARGET_PROPERTY:${name},INCLUDE_DIRECTORIES>")
    set(defs "$<TARGET_PROPERTY:${name},COMPILE_DEFINITIONS>")
    list(APPEND WKT_LIB_CPPFLAGS
      "$<$<BOOL:${incl}>:-I$<JOIN:${incl},$<SEMICOLON>-I>>"
      "$<$<BOOL:${defs}>:-D$<JOIN:${defs},$<SEMICOLON>-D>>"
    )
    add_custom_command(
      OUTPUT ${WKT_F}
      DEPENDS ${WKT_H}
      COMMAND
        ${CMAKE_COMMAND} -E env CPPFLAGS="${WKT_LIB_CPPFLAGS}"
        ${IREP_GENERATE} --mode fortran ${WKT_H} > ${WKT_F}
      COMMAND_EXPAND_LISTS
    )
  endforeach()

  add_library("${name}" STATIC ${WKT_FFILES} ${WKT_LIB_GENERATED})
  set_target_properties("${name}" PROPERTIES LINKER_LANGUAGE C)
  target_include_directories(
    "${name}" PUBLIC
    ${IREP_INCLUDE_DIR}
    $<BUILD_INTERFACE:${CMAKE_Fortran_MODULE_DIRECTORY}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  )

  # convert the target name to a typical variable identifier
  # (uppercase, no dashes, etc.)
  string(MAKE_C_IDENTIFIER "${name}" varname)
  string(TOUPPER "${varname}" varname)

  # set some output variables for this function using the identifier we made
  set(${varname}_FFILES   "${WKT_FFILES}"   PARENT_SCOPE)
  set(${varname}_MODFILES "${WKT_MODFILES}" PARENT_SCOPE)
endfunction()


# add_wkt_index_library()
#
# Generate an irep WKT index library from a list of WKT files.
#
# Usage:
#
#     add_wkt_index_library(
#         name-index-wkt               # name of WKT index library
#         wkt_foo.h wkt_bar.h ...      # non-generated wkt headers
#         [GENERATED wkt_gen1.h ...]   # generated wkt headers (optional)
#     )
#
function(add_wkt_index_library name)
  cmake_parse_arguments(WKT_INDEX "" "" "GENERATED" ${ARGN})
  set(WKT_HEADERS ${WKT_INDEX_UNPARSED_ARGUMENTS})

  list(APPEND WKT_HEADERS "${WKT_INDEX_GENERATED}")
  set(list_WKT_HEADERS "")
  foreach(WKT_H ${WKT_HEADERS})
    if(NOT WKT_H MATCHES ".h$")
      message(FATAL_ERROR "Invalid WKT header name: '${WKT_H}'")
    endif()

    get_filename_component(WKT_H ${WKT_H} ABSOLUTE)
    list(APPEND list_WKT_HEADERS ${WKT_H})
  endforeach()

  # name of single C source file for index library
  set(WKT_INDEX_C "${name}.c")

  # ensure that CPPFLAGS is set to include lib target properties
  set(incl "$<TARGET_PROPERTY:${name},INCLUDE_DIRECTORIES>")
  set(defs "$<TARGET_PROPERTY:${name},COMPILE_DEFINITIONS>")
  list(APPEND WKT_INDEX_CPPFLAGS
    "$<$<BOOL:${incl}>:-I$<JOIN:${incl},$<SEMICOLON>-I>>"
    "$<$<BOOL:${defs}>:-D$<JOIN:${defs},$<SEMICOLON>-D>>"
  )

  add_custom_command(
    OUTPUT ${WKT_INDEX_C}
    DEPENDS ${WKT_HEADERS}
    COMMAND
      ${CMAKE_COMMAND} -E env CPPFLAGS="${WKT_INDEX_CPPFLAGS}"
      ${IREP_GENERATE} --mode index ${WKT_HEADERS} > ${WKT_INDEX_C}
    COMMAND_EXPAND_LISTS
  )

  add_library("${name}" STATIC ${WKT_INDEX_C} ${WKT_INDEX_GENERATED})
  set_target_properties("${name}" PROPERTIES LINKER_LANGUAGE C)
  target_include_directories(
    "${name}" PUBLIC
    ${IREP_INCLUDE_DIR}
    ${CMAKE_Fortran_MODULE_DIRECTORY}
    ${CMAKE_CURRENT_SOURCE_DIR}
  )
endfunction()
