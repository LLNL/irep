// Copyright 2016-2021 Lawrence Livermore National Security, LLC and other
// IREP Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

#ifndef ir_extern_h
#define ir_extern_h

#if defined(IREP_LANG_FORTRAN)
module ir_extern
  use, intrinsic :: iso_c_binding
  use ir_std
  implicit none
  private
  public :: ir_read, ir_exists, ir_rtlen, ir_nprm, ir_nret, ir_unread
  public :: ir_get_function_name
  public :: lua_cb_data

interface ! Let Fortran call C functions ir_read, ir_exists, ir_rtlen.
  integer(c_int) function ir_read(L, t) bind(c, name="ir_read")
    use iso_c_binding
    type(c_ptr), value :: L
    character(kind=c_char), dimension(*) :: t
  end function
  integer(c_int) function ir_unread(L, t) bind(c, name="ir_unread")
    use iso_c_binding
    type(c_ptr), value :: L
    character(kind=c_char), dimension(*) :: t
  end function
  integer(c_int) function ir_exists(L, t) bind(c, name="ir_exists")
    use iso_c_binding
    type(c_ptr), value :: L
    character(kind=c_char), dimension(*) :: t
  end function
  function ir_get_stringref(L, n, len) bind(c, name="ir_get_stringref")
    use, intrinsic :: iso_c_binding, only : c_ptr, c_int
    type(c_ptr), value :: L
    integer(kind=c_int), value :: n
    type(c_ptr), value :: len
    type(c_ptr) :: ir_get_stringref
  end function
  integer(c_int) function ir_rtlen(L, t) bind(c, name="ir_rtlen")
    use iso_c_binding
    type(c_ptr), value :: L
    character(kind=c_char), dimension(*) :: t
  end function
  integer(c_int) function ir_nprm(npnr) bind(c, name="ir_nprm")
    use iso_c_binding
    integer(c_int), value :: npnr
  end function
  integer(c_int) function ir_nret(npnr) bind(c, name="ir_nret")
    use iso_c_binding
    integer(c_int), value :: npnr
  end function
  function ir_get_function_name(L, p) bind(c, name="ir_get_function_name")
    use iso_c_binding
    type(c_ptr), value :: L
    type(c_ptr), value :: p
    type(c_ptr) :: ir_get_function_name
  end function
end interface

end module

#else
#include "ir_std.h"

#if defined(__cplusplus)
extern "C" {
#endif

// These IR functions are intended to be visible in client C/C++ code.
extern int ir_read(lua_State *L, const char *t);
extern int ir_unread(lua_State *L, const char *t);
extern int ir_exists(lua_State *L, const char *t);
extern int ir_rtlen(lua_State *L, const char *s);
extern int ir_nprm(int npnr);
extern int ir_nret(int npnr);
extern char *ir_get_function_name(lua_State *L,void *p);
extern char *ir_get_stringref(lua_State *L,int n, int *len);

#if defined(__cplusplus)
}
#endif

#endif

#endif
