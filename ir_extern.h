// Copyright (c) 2016, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory.  Written by Lee Busby,
// busby1@llnl.gov. LLNL-CODE-702338. All rights reserved.
// See ./Copyright for additional notices.

#ifndef ir_extern_h
#define ir_extern_h

#if defined(IREP_LANG_FORTRAN)
module ir_extern
  use, intrinsic :: iso_c_binding
  use ir_std
  implicit none
  private
  public :: ir_read, ir_exists, ir_rtlen, ir_nprm, ir_nret, ir_unread
  public :: ir_clock, ir_add_time, ir_get_function_name
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
  function ir_clock() bind(c, name="ir_clock")
    use iso_c_binding
    real(c_double) :: ir_clock
  end function
  subroutine ir_add_time(cnt,delta,tp) bind(c, name="ir_add_time")
    use iso_c_binding
    integer(c_int), value :: cnt
    real(c_double), value :: delta
    type(c_ptr), value :: tp
  end subroutine
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
extern double ir_clock(void);
extern void ir_add_time(int count, double delta, ir_time_item *tp);
extern char *ir_get_function_name(lua_State *L,void *p);

#if defined(__cplusplus)
}
#endif

#endif

#endif
