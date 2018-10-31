// Copyright (c) 2016, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory.  Written by Lee Busby,
// busby1@llnl.gov. LLNL-CODE-702338. All rights reserved.
// See ../../Copyright for additional notices.

#ifndef wkt_table1_h
#define wkt_table1_h
#include "ir_start.h"

Beg_struct(irt_table2)
  ir_int(i,1)
 Callback(f2,3,1)
End_struct(irt_table2)

Beg_struct(irt_table3)
  ir_int(i,2)
 Callback(f3,3,2)
End_struct(irt_table3)

Beg_struct(irt_table1)
  ir_int(i,42)
  ir_dbl(d,3.14)
  Vir_dbl(e,5,3.14)
  ir_str(s,8,"abcd")
  ir_log(b,true)
 Structure(irt_table3,table3)
 Vstructure(irt_table2,table2,0:5,6)
 Callback(f1,3,1)
 Callback(f4,3,3)
 Callback(f5,3,-1)
End_struct(irt_table1)

ir_wkt(irt_table1, table1)

#include "ir_end.h"
#endif
