// Copyright (c) 2016, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory.  Written by Lee Busby,
// busby1@llnl.gov. LLNL-CODE-702338. All rights reserved.
// See ../../Copyright for additional notices.

#ifndef wkt_table4_h
#define wkt_table4_h
#include "ir_start.h"

Beg_struct(irt_table4)
  ir_str(name,32,"default name")
  ir_reference(fooref)
End_struct(irt_table4)

Vir_wkt(irt_table4,table4,0:98,4)

#include "ir_end.h"
#endif
