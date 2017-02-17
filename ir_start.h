// Copyright (c) 2016, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory.  Written by Lee Busby,
// busby1@llnl.gov. LLNL-CODE-702338. All rights reserved.
// See ./Copyright for additional notices.

#include "ir_macros.h"

// Include this file at the beginning of each wkt_*.h file.

#if defined(IREP_LANG_FORTRAN)
module IR_WKT_NAME
use, intrinsic :: iso_c_binding
use :: ir_std

#else
#if defined(__cplusplus)
namespace irep {
extern "C" {
#endif
#include "ir_std.h"
#endif
