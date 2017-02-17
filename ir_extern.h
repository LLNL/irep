// Copyright (c) 2016, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory.  Written by Lee Busby,
// busby1@llnl.gov. LLNL-CODE-702338. All rights reserved.
// See ./Copyright for additional notices.

#ifndef ir_extern_h
#define ir_extern_h

#if defined(__cplusplus)
extern "C" {
#endif

// These IR functions are intended to be visible in client C/C++ code.
// (Fortran gets access to them via ir_readnml interface declarations.)
extern int ir_nblen(char *s, int n);
extern int ir_read(lua_State *L, const char *t);
extern int ir_exists(lua_State *L, const char *t);
extern int ir_rtlen(lua_State *L, const char *s);

#if defined(__cplusplus)
}
#endif

#endif
