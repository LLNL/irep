// Copyright (c) 2016, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory.  Written by Lee Busby,
// busby1@llnl.gov. LLNL-CODE-702338. All rights reserved.
// See ./Copyright for additional notices.

#ifndef ir_macros_h
#define ir_macros_h

// Macros to help define the Intermediate Representation (IR).

// ==================================================================
// ========================  FORTRAN SECTION  =======================
// ==================================================================
#if defined(LANG_FORTRAN)
#define Doc(a)

#define ir_wkt(T,ID) type(T), public, target, bind(c) :: ID

#define Beg_struct(ID) type, bind(c) :: ID
#define End_struct(ID) end type ID

// ir_{dbl,int,log,str}: Scalar double, integer, logical, string.
// ID: the name of the variable.
// LEN: the number of characters in a string.
// DV: default value (for ir_log, use "true" or "false".)
#define ir_dbl(ID,DV) real(c_double) :: ID = DV##_c_dbl
#define ir_int(ID,DV) integer(c_int) :: ID = DV
#define ir_log(ID,DV) logical(c_bool) :: ID = .DV.
#define ir_str(ID,LEN,DV) character(c_char) :: ID(LEN) = \
   transfer([character(kind=c_char,len=LEN)::DV],[c_char_'a'])

// Vir_{dbl,int,log,str}: Vector double, integer, logical, string.
// NELEM: number of elements in the vector.
// A "vector" is also known as a one-dimensional array.
// Note that string vectors cannot set a default value.
#define Vir_dbl(ID,NELEM,DV) real(c_double),dimension(NELEM) :: ID = DV##_c_dbl
#define Vir_int(ID,NELEM,DV) integer(c_int),dimension(NELEM) :: ID = DV
#define Vir_log(ID,NELEM,DV) logical(c_bool),dimension(NELEM) :: ID = .DV.
#define Vir_str(ID,LEN,NELEM) character(c_char),dimension(LEN,NELEM) :: ID

// Structure: Declare a variable ID of type T.
#define Structure(T,ID) type(T) :: ID
#define Callback_dd(ID) Structure(lua_cb_dd_data, ID)

// Vstructure: Declare a vector of structures.
// FB: Fortran bounds; CB: C bounds
#define Vstructure(T,ID,FB,CB) type(T) :: ID(FB)

// ==================================================================
// ===========================  C SECTION  ==========================
// ==================================================================
#elif defined(LANG_C)
#define Doc(a)

#define IR_LEN(s) ir_nblen(s,(int)sizeof(s))
#define IR_STR(s) s,IR_LEN(s)

#define ir_wkt(T,ID) extern T ID;

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

// Vector double, integer, logical, string.
#define Vir_dbl(ID,NELEM,DV) double ID[NELEM];
#define Vir_int(ID,NELEM,DV) int ID[NELEM];
#define Vir_log(ID,NELEM,DV) BOOLEAN ID[NELEM];
#define Vir_str(ID,LEN,NELEM) char ID[NELEM][LEN];

#define Structure(T,ID) T ID;
#define Callback_dd(ID) Structure(lua_cb_dd_data, ID)
#define Vstructure(T,ID,FB,CB) T ID[CB];

// ==================================================================
// ===========================  ERROR ===============================
// ==================================================================
#else
#error No recognized language is defined.
#endif

#endif
