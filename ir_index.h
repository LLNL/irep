// Copyright 2016-2021 Lawrence Livermore National Security, LLC and other
// IREP Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

#ifndef ir_index_h
#define ir_index_h

// These are definitions for IREP's internal index representation.
// It's used by generated wkt indexes, but is not intended for use
// by IREP users' code.

// needed for size_t
#include <stddef.h>

// Enum of all data types described by the IREP index.
enum ir_type {
  T_int,
  T_dbl,
  T_log,
  T_str,
  T_cbk,
  T_tbl,
  T_ref,
  T_ptr
};


// Descriptor for an IREP variable.
typedef struct {
  const char *name; // Name of the variable.
  int ti;           // If variable is itself a struct, index into ir_ta, or -1.
  size_t sz;        // Size of (one element of) the variable.
  size_t off;       // Offset of the variable in its enclosing struct.
  int len;          // Max length for string variable; include trailing null.
  int flb;          // Fortran lower bound, if array.
  int fub;          // Fortran upper bound, if array.  Zero for scalar.
  int typ;          // Type code for the variable.  See Typ above.
} ir_element;


// Descriptor for an IREP well known table.
typedef struct {
  void *p;          // Address of the table instance.
  ir_element e;     // As above.
} ir_wkt_desc;


// These lookup tables need to be generated by irep-generate for the entire
// program, and must include *all* wkt's. See irep-generate for details;
// linking irep into a program that does not define ir_wktt and ir_ta will
// result in link failure.

// list of all well known tables and sub-tables
extern ir_element *ir_ta[];

// the index of top-level wkt's (this is where ir_read looks to figure out
// where to write things)
extern ir_wkt_desc ir_wktt[];

// total number of wkt's in the index
extern size_t ir_wktt_size;


#endif // ir_index_h
