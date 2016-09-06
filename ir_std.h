// Copyright (c) 2016, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory.  Written by Lee Busby,
// busby1@llnl.gov. LLNL-CODE-702338. All rights reserved.
// See ./Copyright for additional notices.

#ifndef ir_std_h
#define ir_std_h
#include "ir_macros.h"

#if defined(LANG_FORTRAN)
module IR_WKT_NAME
use, intrinsic :: iso_c_binding
integer, parameter :: c_dbl = kind(real(1.0, c_double))
#endif

// Standard types and structures predefined by the IR.

// A structure to hold data registering a Lua callback function
// 'dd' means this function takes double args and returns double values.
Beg_struct(lua_cb_dd_data)
  ir_int(nprm, -1)
  ir_int(nret, -1)
  ir_int(fref, -1) // -1 == LUA_REFNIL
 Vir_dbl(const_val,9,0.0)
End_struct(lua_cb_dd_data)

#if defined(LANG_FORTRAN)
end module
#endif

#endif
