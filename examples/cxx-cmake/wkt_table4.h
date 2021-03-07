// Copyright 2016-2021 Lawrence Livermore National Security, LLC and other
// IREP Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

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
