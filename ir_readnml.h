! Copyright (c) 2016, Lawrence Livermore National Security, LLC. Produced
! at the Lawrence Livermore National Laboratory.  Written by Lee Busby,
! busby1@llnl.gov. LLNL-CODE-702338. All rights reserved.
! See ./Copyright for additional notices.

module ir_readnml
  use, intrinsic :: iso_c_binding
  use ir_std
  ! Macro below expands ==> use wkt_table1; use wkt_table2; ...
  USE_WKT
  implicit none
  private
  public :: ir_read, ir_exists, ir_rtlen, ir_fstr

  public :: lua_cb_dd_data

interface ! Let Fortran call C functions ir_read and ir_exists.
  integer(c_int) function ir_read(L, t) bind(c, name="ir_read")
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
end interface

contains

! Convert a Fortran array of characters to a scalar Fortran string.
function ir_fstr(s) result(fs)
  use, intrinsic :: iso_c_binding, only : c_char
  character(kind=c_char, len=1), intent(in) :: s(:)
  character(kind=c_char, len=size(s)) :: fs
  integer :: i
  do i = 1, len(fs)
    fs(i:i) = s(i)
  enddo
end function

! This function is called only from C.
integer(c_int) function cir_rd(s,n) bind(c, name="ir_rd_nml")
  integer(c_int), value, intent(in) :: n
  character(len=1,kind=c_char), intent(in) :: s(n)
  character(kind=c_char,len=n) :: fs
  ! Macro below expands ==> table1, table2, ...
  namelist /ir_input/ WKT_NML
  fs = transfer(s,fs)
  read(fs, nml=ir_input, iostat=cir_rd)
end function

end module
