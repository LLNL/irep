! Copyright (c) 2016, Lawrence Livermore National Security, LLC. Produced
! at the Lawrence Livermore National Laboratory.  Written by Lee Busby,
! busby1@llnl.gov. LLNL-CODE-702338. All rights reserved.
! See ../../Copyright for additional notices.

module mainmod
  use, intrinsic :: iso_c_binding
  implicit none

  integer(kind=c_int), parameter :: LUA_NOREF          =     -2
  integer(kind=c_int), parameter :: LUA_REGISTRYINDEX  = -10000
  integer(kind=c_int), parameter :: LUA_TNUMBER        =      3

  interface fstr
    module procedure fstr_arr, fstr_ptr
  end interface

  interface

  pure function strlen_ptr(s) result(result) bind(c,name="strlen")
    use, intrinsic :: iso_c_binding
    integer(c_int) :: result
    type(c_ptr), value, intent(in) :: s
  end function strlen_ptr

  function luaL_newstate() bind(c, name="luaL_newstate")
    use, intrinsic :: iso_c_binding, only : c_ptr
    type(c_ptr) :: luaL_newstate
  end function

  function luaL_loadfile(L, filename) bind(c, name="luaL_loadfile")
    use, intrinsic :: iso_c_binding, only : c_ptr, c_int, c_char
    type(c_ptr), value :: L
    character(kind=c_char), dimension(*) :: filename
    integer(kind=c_int) :: luaL_loadfile
  end function

  function lua_tolstring(L, index, len) bind(c, name="lua_tolstring")
    use, intrinsic :: iso_c_binding, only : c_ptr, c_int
    type(c_ptr), value :: L
    integer(kind=c_int), value :: index
    type(c_ptr), value :: len
    type(c_ptr) :: lua_tolstring
  end function

  function lua_pcall(L, nargs, nresults, errfunc) bind(c, name="lua_pcall")
    use, intrinsic :: iso_c_binding, only : c_int, c_ptr
    type(c_ptr), value :: L
    integer(kind=c_int), value :: nargs
    integer(kind=c_int), value :: nresults
    integer(kind=c_int), value :: errfunc
    integer(kind=c_int) :: lua_pcall
  end function

  subroutine luaL_openlibs(L) bind(c, name="luaL_openlibs")
    use, intrinsic :: iso_c_binding, only : c_ptr
    type(c_ptr), value :: L
  end subroutine

  subroutine lua_rawgeti(L, index, n) bind(c, name="lua_rawgeti")
    use, intrinsic :: iso_c_binding, only : c_ptr, c_int
    type(c_ptr), value :: L
    integer(kind=c_int), value :: index, n
  end subroutine

  subroutine lua_pushnumber(L, n) bind(c, name="lua_pushnumber")
    use, intrinsic :: iso_c_binding, only : c_ptr, c_double
    type(c_ptr), value :: L
    real(kind=c_double), value :: n
  end subroutine

  function lua_type(L, index) bind(c, name="lua_type")
    use, intrinsic :: iso_c_binding, only : c_ptr, c_int
    type(c_ptr), value :: L
    integer(kind=c_int), value :: index
    integer(kind=c_int) :: lua_type
  end function

  function lua_tonumber(L, index) bind(c, name="lua_tonumber")
    use, intrinsic :: iso_c_binding, only : c_ptr, c_int, c_double
    type(c_ptr), value :: L
    integer(kind=c_int), value :: index
    real(kind=c_double) :: lua_tonumber
  end function

  subroutine lua_settop(L, index) bind(c, name="lua_settop")
    use, intrinsic :: iso_c_binding, only : c_ptr, c_int
    type(c_ptr), value :: L
    integer(kind=c_int), value :: index
  end subroutine

  end interface

! ----------------------------------------------------------------------
contains

subroutine lua_pop(L, index)
  use, intrinsic :: iso_c_binding, only : c_ptr, c_int
  type(c_ptr), value :: L
  integer(kind=c_int), value :: index
  call lua_settop(L, -(index)-1)
end subroutine

function lua_tostring(L, index)
  use, intrinsic :: iso_c_binding, only : c_ptr, c_int, c_null_ptr
  type(c_ptr), value :: L
  integer(kind=c_int), value :: index
  type(c_ptr) :: lua_tostring, x = c_null_ptr
  lua_tostring = lua_tolstring(L, index, x)
end function

! Convert a null-terminated C "char *" pointer to a scalar Fortran string.
function fstr_ptr(s) result(fs)
  use, intrinsic :: iso_c_binding, only: c_char, c_ptr, c_f_pointer
  type(c_ptr), intent(in) :: s
  character(kind=c_char, len=strlen_ptr(s)) :: fs
  character(kind=c_char), pointer :: cptr(:)
  integer :: i
  call c_f_pointer(s, cptr, [len(fs)])
  do i=1, len(fs)
    fs(i:i) = cptr(i)
  enddo
end function

! Convert a null-terminated array of characters to a scalar Fortran string.
function fstr_arr(s) result(fs)
  use, intrinsic :: iso_c_binding, only : c_char, c_null_char
  character(kind=c_char, len=1), intent(in) :: s(:)
  character(kind=c_char, len=size(s)) :: fs
  integer :: i
  fs = ' '
  do i = 1, len(fs)
    if (s(i) == c_null_char) exit
    fs(i:i) = s(i)
  enddo
end function

! Convert a fortran string in 's' to a null-terminated array of characters.
pure function cstr(s)
  use, intrinsic :: iso_c_binding, only : c_char, c_null_char
  character(len=*), intent(in) :: s
  character(kind=c_char, len=1) :: cstr(len_trim(s)+1)
  integer :: i
  !i=0 ! Some compilers warn about uninitialized variable without this.
  if (len_trim(s) > 0) cstr = [ (s(i:i), i=1,len_trim(s)) ]
  cstr(len_trim(s)+1) = c_null_char
end function

! A simple evaluator for IREP callback functions.
subroutine eval_dd(L, d, x, v)
  use, intrinsic :: iso_c_binding
  use, intrinsic :: iso_fortran_env, only : error_unit
  use ir_std
  implicit none
  type(c_ptr), value, intent(in) :: L
  type(lua_cb_dd_data), intent(in) :: d
  real(c_double), intent(in) :: x(:)
  real(c_double), intent(out) :: v(:)
  integer :: i

  if (d%fref == LUA_NOREF) then
    do i=1,d%nret
      v(i) = d%const_val(i)
    enddo
    return
  endif

  ! Push the function and arguments onto the stack.
  call lua_rawgeti(L, LUA_REGISTRYINDEX, d%fref)
  do i = 1, d%nprm
    call lua_pushnumber(L, x(i))
  enddo
  if (lua_pcall(L, d%nprm, d%nret, 0) /= 0) then
    write(error_unit,'(2a)') "LUA ERROR: ", fstr(lua_tostring(L,-1))
    stop 1
  endif

  do i=1,d%nret
    if (lua_type(L, i - d%nret - 1) /= LUA_TNUMBER) then
      write(error_unit,'(2a)') "error: expected number for return value: ", i
      stop 1
    endif
    v(i) = lua_tonumber(L, i - d%nret - 1)
  enddo
  call lua_pop(L, d%nret) ! Pop args + function.
end subroutine

end

! ------------------------------------------------------------------------
program main
  use, intrinsic :: iso_c_binding
  use ir_readnml
  use wkt_table1
  use wkt_table4
  use mainmod
  implicit none

  integer :: ios, i, n, ng
  real(c_double) :: v(1), x(3) = [ 2.0, 3.0, 4.0 ]
  type(c_ptr) :: L
  character(len=64) :: arg

  L = luaL_newstate()
  call luaL_openlibs(L)

  if (command_argument_count() .lt. 1) then
    print *, "Usage: fprog *.lua"
    stop 0
  endif
  call get_command_argument(1, arg)
  if (luaL_loadfile(L, cstr(arg)) .ne. 0) then
    print *, "cannot load file: ", fstr(lua_tostring(L,-1))
    stop 1
  endif
  if (lua_pcall(L, 0, 0, 0) .ne. 0) then
    print *, "cannot run file: ", fstr(lua_tostring(L,-1))
    stop 1
  endif

  if (ir_read(L, cstr("table1")) .ne. 0) stop
  print *,"READ TABLE1"
  write(*,"(a,i4)") "table1.i = ", table1%i
  write(*,"(a,f6.2)") "table1.d = ", table1%d
  write(*,"(a,l2)") "table1.b = ", table1%b
  write(*,"(a,a)") "table1.s = ", trim(ir_fstr(table1%s))

  call eval_dd(L, table1%f1, x, v)
  write(*,"(a,f3.1,a,f3.1,a,f3.1,a,f6.2)") &
    "table1.f1(",x(1),",",x(2),",",x(3),") = ",v(1)

  call eval_dd(L, table1%table2(1)%f2, x, v)
  write(*,"(a,f3.1,a,f3.1,a,f3.1,a,f6.2)") &
    "table1.table2[1].f2(",x(1),",",x(2),",",x(3),") = ",v(1)

  call eval_dd(L, table1%table3%f3, x, v)
  write(*,"(a,f3.1,a,f3.1,a,f3.1,a,f6.2)") &
    "table1.table3.f3(",x(1),",",x(2),",",x(3),") = ",v(1)

  print *, ""
  n = size(table1%e)
  ng = ir_rtlen(L, cstr("table1.e"))
  write(*, "(a,i2,a,i2,a)") "table1.e: ",n, " elements, ",ng, " given"
  do i=1,n
    write(*,"(a,i1,a,f6.2)") "table1.e(",i,") = ", table1%e(i)
  enddo

  if (ir_read(L, cstr("table4")) .ne. 0) stop
  print *, ""
  print *,"READ TABLE4"
  do i=1,3
    write(*,"(a,i1,a,a)") "table4(",i,").name = ", trim(ir_fstr(table4(i)%name))
  enddo

end
