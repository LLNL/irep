// Copyright (c) 2016, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory.  Written by Lee Busby,
// busby1@llnl.gov. LLNL-CODE-702338. All rights reserved.
// See ../../Copyright for additional notices.

#ifndef wkt_statistics_h
#define statistics_h
#include "ir_start.h"

Beg_struct(irt_test3)
  ir_int(i,138)
  Vir_dbl(dd3,3,27.5)
  Timeitem(item3)
End_struct(irt_test3)

Beg_struct(irt_test2)
  ir_int(i,97)
  Vir_dbl(dd,2,2.5)
  Structure(irt_test3,test3)
End_struct(irt_test2)

Beg_struct(irt_test1)
  ir_str(t1foo,32,"default t1foo")
  Vstructure(irt_test2,test2,0:1,2)
  ir_dbl(t1d, 8.3)
  Vir_int(t1vi,2,38)
End_struct(irt_test1)

Beg_struct(irt_statistics)
  ir_str(foo,32,"default foo")
  Timeitem(item1)
  Structure(irt_test1,test1)
End_struct(irt_statistics)

ir_wkt(irt_statistics,statistics)

#include "ir_end.h"
#endif
