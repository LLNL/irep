// Copyright (c) 2016, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory.  Written by Lee Busby,
// busby1@llnl.gov. LLNL-CODE-702338. All rights reserved.
// See ../../Copyright for additional notices.

#ifndef wkt_table4_h
#define wkt_table4_h
#include "ir_start.h"

Beg_struct(irt_table4)
  ir_str(name,32,"default name")
End_struct(irt_table4)

// Define table4 as an array.  This is too complicated to use the
// ir_wkt macro, so do it directly below.
#if defined(LANG_FORTRAN)
type(irt_table4), public, target, dimension(0:3), bind(c) :: table4
save :: table4
namelist /ir_input/ table4
#elif defined(LANG_C)
extern irt_table4 table4[4];
#endif

#include "ir_end.h"
#endif
