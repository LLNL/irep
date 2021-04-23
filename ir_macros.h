// Copyright 2016-2021 Lawrence Livermore National Security, LLC and other
// IREP Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

#ifndef ir_macros_h
#define ir_macros_h

// Macros to help define the Intermediate Representation (IR).

// ==================================================================
// ================== CONSTANTS FOR IR DEFINITIONS ==================
// ==================================================================
#define FILENAMESIZE 256

// ==================================================================
// ========================  FORTRAN SECTION  =======================
// ==================================================================
#if defined(IREP_LANG_FORTRAN)
#define Doc(a)

#define ir_wkt(T,ID) type(T), public, target, bind(c) :: ID
#define Vir_wkt(T,ID,FB,CB) \
  type(T), public, target, dimension(FB), bind(c) :: ID; save :: ID

#define Beg_struct(T) type, bind(c) :: T
#define End_struct(T) end type T

// ir_{dbl,int,log,str}: Scalar double, integer, logical, string.
// ID: the name of the variable.
// LEN: the number of characters in a string.
// DV: default value (for ir_log, use "true" or "false".)
#define ir_dbl(ID,DV) real(c_double) :: ID = DV##_c_double
#define ir_int(ID,DV) integer(c_int) :: ID = DV
#define ir_log(ID,DV) logical(c_bool) :: ID = .DV.
#define ir_str(ID,LEN,DV) character(c_char) :: ID(LEN) = \
   transfer([character(kind=c_char,len=LEN)::DV +/+/ c_null_char],[c_char_'a'])
#define ir_reference(ID) integer(c_int) :: ID = -1
#define ir_ptr(ID) type(c_ptr) :: ID

// Vir_{dbl,int,log,str}: Vector double, integer, logical, string.
// NELEM: number of elements in the vector.
// A "vector" is also known as a one-dimensional array.
// Note that string vectors cannot set a default value.
#define Vir_dbl(ID,NELEM,DV) real(c_double),dimension(NELEM) :: ID = DV##_c_double
#define Vir_int(ID,NELEM,DV) integer(c_int),dimension(NELEM) :: ID = DV
#define Vir_log(ID,NELEM,DV) logical(c_bool),dimension(NELEM) :: ID = .DV.
#define Vir_str(ID,LEN,NELEM) character(c_char),dimension(LEN,NELEM) :: ID

// Structure: Declare a variable ID of type T.
#define Structure(T,ID) type(T) :: ID
#define Callback(ID,NP,NR) Structure(lua_cb_data, ID)

// Vstructure: Declare a vector of structures.
// FB: Fortran bounds; CB: C bounds
#define Vstructure(T,ID,FB,CB) type(T) :: ID(FB)

// ==================================================================
// ===========================  LUA SECTION  ========================
// ==================================================================
#elif defined(IREP_LANG_LUA)
#define Doc(a) %%% a

#define ir_wkt(T,ID) ID = T --
#define Vir_wkt(T,ID,FB,CB) VDEFINE ID T

#define Beg_struct(T) T = { --
#define End_struct(T) } -- End T

// Scalar double, integer, logical, string.
#define ir_dbl(ID,DV)     ID @@@ sd %%% DV %%% 0   %%% 0
#define ir_int(ID,DV)     ID @@@ si %%% DV %%% 0   %%% 0
#define ir_log(ID,DV)     ID @@@ sb %%% DV %%% 0   %%% 0
#define ir_str(ID,LEN,DV) ID @@@ ss %%% DV %%% LEN %%% 0
#define ir_reference(ID)  ID @@@ si %%% -1 %%% 0   %%% 0
#define ir_ptr(ID)        ID @@@ sp %%%  0 %%% 0   %%% 0

// Vector double, integer, logical, string.
#define Vir_dbl(ID,NELEM,DV)  ID @@@ vd %%% DV       %%% 0   %%% NELEM
#define Vir_int(ID,NELEM,DV)  ID @@@ vi %%% DV       %%% 0   %%% NELEM
#define Vir_log(ID,NELEM,DV)  ID @@@ vb %%% DV       %%% 0   %%% NELEM
#define Vir_str(ID,LEN,NELEM) ID @@@ vs %%% "(none)" %%% LEN %%% NELEM

#define Structure(T,ID) ID = T, --
#define Callback(ID,NP,NR)    ID @@@ lf %%% function %%% NP   %%% NR
#define Vstructure(T,ID,FB,CB) ID = { [1] = T }, --

// ==================================================================
// ===========================  GENERATOR SECTION  ==================
// ==================================================================
#elif defined(IREP_GENERATE)
#define Doc(a)

#define ir_wkt(T,ID) wkt ID T 0:0
#define Vir_wkt(T,ID,FB,CB) wkt ID T FB

#define Beg_struct(T) bst T 0 0
#define End_struct(T) est T 0 0

// Scalar double, integer, logical, string.
#define ir_dbl(ID,DV)         T_dbl ID 0 0
#define ir_int(ID,DV)         T_int ID 0 0
#define ir_log(ID,DV)         T_log ID 0 0
#define ir_str(ID,LEN,DV)     T_str ID LEN 0
#define Callback(ID,NP,NR)    T_cbk ID NP:NR 0
#define ir_reference(ID)      T_ref ID 0 0
#define ir_ptr(ID)            T_ptr ID 0 0

// Vector double, integer, logical, string.
#define Vir_dbl(ID,NELEM,DV)  T_dbl ID 0 NELEM
#define Vir_int(ID,NELEM,DV)  T_int ID 0 NELEM
#define Vir_log(ID,NELEM,DV)  T_log ID 0 NELEM
#define Vir_str(ID,LEN,NELEM) T_str ID LEN NELEM

#define Structure(T,ID) T_tbl ID T 0:0
#define Vstructure(T,ID,FB,CB) T_tbl ID T FB

// ==================================================================
// ===========================  C SECTION  ==========================
// ==================================================================
#else  // no IREP_LANG_C required
// Earlier versions of IREP required IREP_LANG_C to be defined, but
// now the default is to generate C. This allows IREP headers to be
// included without any special defines.

#define Doc(a)

#define IR_STR(s) s

#define ir_wkt(T,ID) extern T ID;
#define Vir_wkt(T,ID,FB,CB) extern T ID[CB];

#define Beg_struct(T) typedef struct T {
#define End_struct(T) } T;

#if defined(__cplusplus)
#define BOOLEAN bool
#else
#define BOOLEAN _Bool
#endif

// Scalar double, integer, logical, string.
#define ir_dbl(ID,DV) double ID;
#define ir_int(ID,DV) int ID;
#define ir_log(ID,DV) BOOLEAN ID;
#define ir_str(ID,LEN,DV) char ID[LEN];
#define ir_reference(ID) int ID;
#define ir_ptr(ID) void *ID;

// Vector double, integer, logical, string.
#define Vir_dbl(ID,NELEM,DV) double ID[NELEM];
#define Vir_int(ID,NELEM,DV) int ID[NELEM];
#define Vir_log(ID,NELEM,DV) BOOLEAN ID[NELEM];
#define Vir_str(ID,LEN,NELEM) char ID[NELEM][LEN];

#define Structure(T,ID) T ID;
#define Callback(ID,NP,NR) Structure(lua_cb_data, ID)
#define Vstructure(T,ID,FB,CB) T ID[CB];

#endif  // defined IREP_LANG_*
#endif  // ir_macros_h
