.. Copyright 2016-2021 Lawrence Livermore National Security, LLC and other
.. IREP Project Developers. See the top-level LICENSE file for details.
..
.. SPDX-License-Identifier: MIT

.. _irep-overview:

===================
Overview of IREP
===================

The Intermediate Representation (or IR or IREP) is a tool to simplify
reading some kinds of external program input, a means to organize the
input into native language structures, and a way to share those
structures among program modules written in either C/C++ or Fortran. It
can handle several varieties of ``Plain Old Data`` and has mechanisms to
extend its abilities beyond POD. It requires access to a Lua 5.1
interpreter, C and Fortran compilers, and follows standards for those
languages. A reader for tables written in the Lua programming language
is included.

Introduction
------------

Sharing Data Among Lua, C/C++ and Fortran
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Lua tables and constructors are a flexible way to define program input.
It is usually easy to roughly map a Lua table into structured data types
for C/C++ and Fortran::

    Lua             C/C++                    Fortran
    ____________________________________________________________
    t = {           typedef struct irt_t {   type, bind(c) :: irt_t
      a = 1,          int a;                   integer(c_int) :: a = 1
      b = {2.2,3.3},  double b[2];             real(c_double) :: b(2)
      c = "foo",      char c[16];              character(c_char) :: c(16)
      d = true,       _Bool d;                 logical(c_bool) :: d = .true.
    }               } irt_t;                 end type irt_t

This works best for, but is not strictly limited to, ``Plain Old Data``
(POD). For our purposes, POD is a scalar variable or 1D vector of type
integer, double, string, or boolean. The Fortran ``ISO_C_BINDING`` lets
us map structures between C/C++ and Fortran. The C preprocessor specifies
the IR in a language-neutral way. Here is the same structure as above,
written using the IR macros:

.. code-block:: C

   Beg_struct(irt_t)      Doc(( Begin structure declaration ))
      ir_int(a,1)         Doc(( Scalar integer with default value = 1 ))
      Vir_dbl(b,2,3.0)     Doc(( Vector double, 2 elements, default = 3.0 ))
      ir_str(c,16,"foo")  Doc(( Scalar string, maxlen=16, default = "foo" ))
      ir_log(d,true)      Doc(( Scalar boolean, default = true ))
   End_struct(irt_t)      Doc(( End structure ))

Structures in the IR are defined in a C header. When the file is included
from C/C++ source, the macros output a C/C++ struct declaration. From
Fortran, the output is the equivalent derived0m type. The Fortran version
defines and initializes the structures, while C/C++ declares them as
extern objects. There is one shared instance of any IR variable in the
data segment of a running program.

Nested Tables
^^^^^^^^^^^^^

The IR includes a Structure macro to allow nested C/C++ and Fortran
structures::

    IREP                    C/C++            Fortran
    __________________________________________________
    Beg_struct(irt_T1)      ...              ...
      Structure(irt_T2,x)   irt_T2 x;        type(irt_T2) :: x
    End_struct(irt_T1)      ...              ...

Nested structures follow C ordering rules: ``irt_T2`` must be declared
before ``irt_T1``, etc. After the IR has been defined and compiled, and after
data has been read from the Lua input file into the IR, the host code can
gaccess data using native language operations::

    -- First, set the value by reading the Lua table.
    material[1].EOS.table.number = 1018

    // Later, read the value from C/C++.
    int n = material[1].EOS.table.number; // 1018

    ! Or read the value from Fortran.
    integer :: n
    n = material(1)%EOS%table%number ! 1018

Notice that the example above uses index ``1`` for all three languages.  This is a consequence of the way that the material table
was  declared  in  the  IREP.   See the discussion of the Vir_wkt0m
macro in the IREP man page that accompanies this paper  for  more
information.

Reading Data into the IR
^^^^^^^^^^^^^^^^^^^^^^^^

The compiled data store in the IR is independent of any means to load or
change its values. We provide a Lua table reader for that purpose. If the
IR data store defines a top-level structure named material, as in the
example above, we expect the Lua input to contain a global table also
named material, and call this a well known table (WKT). The elements of
the Lua table must match the elements of the IR structure. Whenever the
Lua reader reaches a leaf node in its table, it looks up the address of
the matching IR element, and places the value at that memory location.

Designing the User Interface
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Lua tables and Lua variables can be quite different from C/C++ or Fortran
structures and variables. Here are some Lua idioms that are difficult to
model in the IR data store:

1. Polymorphic element values: Lua is happy to allow both ``x=1`` and
   ``x="one"`` (at different times) in the same table. This is hard to do
   in the IR, because C/C++ and Fortran variables can't change type. One
   place where we do allow polymorphic elements is our implementation of
   callback functions:

   .. code-block:: lua

       f = 3
       f = { 1, 2, 3 } -- max len is 9 elements
       f = function() return 3 end

   We frequently use constant callback functions, so it is convenient to
   allow scalar or array values in place of a true Lua function, as shown
   in the first two examples above. Callback functions require some
   special handling already, so this case does not present much extra
   trouble for the Lua reader. In general, however, it's a good idea to
   limit polymorphic elements in the user interface.

2. Lua tables with both record and array parts:

   .. code-block:: lua

       a = { 3,2,1, x=4 }

   That is, ``a[1] == 3``, and ``a.x == 4``. There is no easy way to
   handle this case with the IR, because ``a`` is neither a vector, nor
   is it a structured type. It's some of both. Don't do this, or use an
   ir_reference if you must.

3. Lua arrays with mixed types:

   .. code-block:: lua

      a = { 3,2,1, "liftoff" }

   In  this case, ``a`` is a vector, but its elements don't have any
   single type.  Arrays in C/C++ and Fortran cannot have elements of
   more  than  one type.  So don't do this either.

In designing the user interface, we can choose, case by case, whether to
let the constraints imposed by the IR also constrain or shape the Lua
interface. By its nature, the IR rewards simplicity in user interface
design.

Some usage notes
----------------

The ``ir_read`` and ``ir_exists`` functions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In addition to defining IR input data, the host code is responsible for
reading the well known tables. The function provided for this task is
ir_read. For example, in C, we might read the materials table:

.. code-block:: C

    int n = ir_read(L, "materials");
    if (n != 0) {
        printf("ERROR: %d\n", n);
        exit(1);
    }

``ir_read`` returns the total number of errors it  encountered.   For
well known tables that may be optional, the ``ir_exists`` function is
provided to allow the host code to query whether  a  given  table
element  is in the Lua state before attempting to read it.

Strings
^^^^^^^

Sharing strings between Fortran and C/C++ is easier than it once was.
That said, reading strings from the IR requires extra care for both
languages. Strings are stored in the IR as an array of characters,
``NULL`` terminated. The max length is part of the original declaration,
and it includes the ``NULL`` character. The IR provides convenience
functions and macros to simplify some common operations::

    std::string myfoo(IR_STR(t.c));    // C++: Declare/initialize.
    character(len=64) :: myfoo         ! Fortran: Declare scalar.
    myfoo = trim(fstr(IR_STR(t.c))) ! Initialize it.
