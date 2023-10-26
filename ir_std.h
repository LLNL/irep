// Copyright 2016-2021 Lawrence Livermore National Security, LLC and other
// IREP Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

#ifndef ir_std_h
#define ir_std_h
#include "ir_macros.h"

#if defined(IREP_LANG_FORTRAN)
module IR_WKT_NAME
use, intrinsic :: iso_c_binding
#endif

#if defined(__cplusplus)
extern "C" {
#endif

// Standard types and structures predefined by the IR.

// Data for Lua callback function.
Beg_struct(lua_cb_data)
  ir_int(fref, -1) // -1 == LUA_REFNIL
  ir_int(npnr, -9) // packed nprm,nret
  ir_int(base_npnr, -9) // unaltered version
  ir_ptr(data)
End_struct(lua_cb_data)

#if defined(__cplusplus)
}
#endif

#if defined(IREP_LANG_FORTRAN)
end module
#endif

#endif
