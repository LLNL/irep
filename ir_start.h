// Copyright 2016-2021 Lawrence Livermore National Security, LLC and other
// IREP Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

#include "ir_macros.h"

// Include this file at the beginning of each wkt_*.h file.

#if defined(IREP_LANG_FORTRAN)
module IR_WKT_NAME
use, intrinsic :: iso_c_binding
use :: ir_std

#else
#include "ir_std.h"
#if defined(__cplusplus)
namespace irep {
extern "C" {
#endif
#endif
