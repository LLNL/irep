// Copyright (c) 2016, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory.  Written by Lee Busby,
// busby1@llnl.gov. LLNL-CODE-702338. All rights reserved.
// See ./Copyright for additional notices.

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
  ir_ptr(data)
End_struct(lua_cb_data)

// A time entry for the wkt_statistics table.
Beg_struct(ir_time_item)
  ir_int(count, 0)
  ir_dbl(time, 0.0)
End_struct(ir_time_item)

#if defined(__cplusplus)
}
#endif

#if defined(IREP_LANG_FORTRAN)
end module
#endif

#endif
