.. Copyright 2016-2021 Lawrence Livermore National Security, LLC and other
.. IREP Project Developers. See the top-level LICENSE file for details.
..
.. SPDX-License-Identifier: MIT

.. _irep-reference:

==============
IREP Reference
==============

IREP - An Intermediate Representation for Structured Input

.. code-block:: C

   // C/C++
   #include "ir_extern.h"
   #include "wkt_TABLENAME.h"

   int ir_read(lua_State *L, const char *tbl_elem);
   int ir_exists(lua_State *L, const char *t);
   int ir_rtlen(lua_State *L, const char *s);
   int ir_nprm(int npnr);
   int ir_nret(int npnr);
   int ir_unread(lua_State *L, const char *ir_tbl)
   char *ir_get_stringref(lua_State *L, int n, int *len);

.. code-block:: fortran

   ! Fortran
   use ir_extern
   use wkt_TABLENAME

Description
-----------

The IREP is a toolkit and associated C library, to read Lua
tables into a compiled data store.  The toolkit defines the data
store as a set of structured variables in C/C++ and Fortran.  The
library reads Lua tables, comparing table keys with the
corresponding variable in the data store, and puts the table
values into the data store.

Usage
-----

Step one is to design and define one
or more well-known tables for the data store (see below).  After
creating the well-known tables, you can build the IREP library,
and link it to your C/C++ and/or Fortran code - the host code.
Your host code reads Lua tables into the data store:

.. code-block:: C

  int nerr = ir_read(L, "mytable");

You can access a variable from ``mytable`` using native language
operations in the host code:

.. code-block:: C

   // C
   #include "wkt_mytable.h"
   int n = mytable.subtable.n;

.. code-block:: C++

   // C++ (The IREP datastore is in namespace "irep".)
   #include "wkt_mytable.h"
   int n = irep::mytable.subtable.n;

.. code-block:: Fortran

   ! Fortran
   use wkt_mytable
   integer n
   n = mytable%subtable%n

You can query the Lua table to discover whether a particular
element actually was given in the input:

.. code-block:: C

   int e = ir_exists(L, "mytable.subtable.n");
   if (e) {
       printf("User input explicitly defines N\n");
   } else {
       printf("Using IREP default value for N\n");
   }

If a table element is a vector, you can ask how long the actual
input array was:

.. code-block:: C

  int nelem = ir_rtlen(L, "mytable.subtable.v1");

If a table element is a string, you can copy it from the data
store into a local variable using a macro or function from IREP:

.. code-block:: C

   // C
   char *ss = strndup(IR_STR(t.ss));	    // Access scalar string.
   char *vs = strndup(IR_STR(t.vs[0]));	    // First element of vector.

.. code-block:: C++

   // C++
   std::string my_ss(IR_STR(irep::t.ss));    // Access scalar string
   std::string my_vs(IR_STR(irep::t.vs[0])); // First element of vector.

.. code-block:: fortran

   ! Fortran
   character(len=64) :: my_ss, my_vs
   my_ss = trim(fstr(t%ss))		    ! Access scalar string.
   my_vs = trim(fstr(t%vs(:,1)))		    ! First element of vector.

The remaining functions in the IREP API are briefly described:

``int ir_nprm(int npnr); // Return NPRM, given an npnr value.``

``int ir_nret(int npnr); // Return NRET, given an npnr value.``
    These two functions are typically used by the host code during
    evaluation of Lua callback functions (see
    :ref:`lua-callback-functions`).

``int ir_unread(lua_State *L, const char *ir_tbl)``
    This function is experimental at present. It is effectively the
    reverse of ir_read: IREP can read one of the tables in the data
    store, and create a corresponding table in the lua_State.

``char *ir_get_function_name(lua_State *L,void *p);``
    This function is aimed mainly at error reporting, during callback
    function evaluation. When the Lua input file is read, IREP stores the
    full name of each callback function using the address of its
    associated ``lua_cb_data`` structure as a key. The function name can
    thus be accessed later, typically to produce better error messages.

``char *ir_get_stringref(lua_State *L, int n, int *len);``

    This function handles the case where a Lua string of arbitrary length
    has been stored using the IREP ir_reference macro. (See
    ``ir_get_stringref``.) The client code typically calls
    ir_get_stringref as follows:

    .. code-block:: C

       if (ir_exists(L, "physics.foo")) {
          int nn;
          std::string foo = ir_get_stringref(L,irep::physics.foo,&nn);
          lua_pop(L,-1);
       }

The nn parameter returns the length of the string; it can be
passed as ``(char *) NULL`` if you do not need this value.


Defining the Data Store
-----------------------

The IREP data store is a collection of C/C++ "structs" or their
equivalent Fortran "derived type". The code defining each variable in the
data store is written once, using cpp(1) macros that expand into either
C/C++ or Fortran source code, depending on the context:

Example Table in the IREP Data Store::

  IREP Input	       C/C++ Output    Fortran Output
  ---------------------------------------------------------------------
  Beg_struct(irt_t)    struct irt_t {  type, bind(c) :: irt_t
    ir_int(a,1)          int a;          integer(c_int) :: a=1
    Vir_dbl(b,2)         double b[2];    real(c_double) :: b(2)
    ir_str(c,8,"foo")    char c[8];      character(c_char) :: c(8)="foo"
    ir_log(d,true)       _Bool d;        logical(c_bool) :: d=.true.
  End_struct(irt_t)    } irt_t;        end type irt_t

The rules for constructing the IREP data store are precisely the
rules for C and Fortran, constrained by the Fortran
``ISO_C_BINDING``.  Structures can be nested in arbitrary fashion.
Scalar and one-dimensional array variables (vectors) of type
integer, double, boolean, or character (strings) can be defined.
In addition, special types are available to define Lua callback0m
functions and Lua references.

Summary of IREP Macros
----------------------

Beg_struct(T)
    Begin declaration for a table of type T.

End_struct(T)
    End declaration for a table of type T.

ir_wkt(T,ID)
    Define a table named ID, of type T.

Vir_wkt(T,ID,FB,CB)
Define a vector of tables named ID, of type T. The vector has Fortran
bounds FB0m and C bounds CB. E.g., if FB=``0:5'', and CB=``6'', the
vector will have six elements indexed from zero to five. Since Lua
normally indexes from 1, the 0th element will presumably be unused. This
is useful if you want a particular index value to have the same semantics
in all three of C/C++, Fortran, and Lua. Alternatively, if you set
FB=``1:6'', then the first element of the array would have index 0 in
C/C++, but index 1 in Fortran and Lua.

``Structure(T,ID)``
    Declare a table named ID, with typename T.

``Vstructure(T,ID,FB,CB)``
    Declare a vector of tables ID, typename T. The vector has Fortran
    bounds FB and C bounds CB. (See Vir_wkt above.)

``ir_dbl(ID,DV)``
    Declare scalar variable of type double named ID, default value DV.

``ir_int(ID,DV)``
    Declare scalar variable of type integer named ID, default value DV.

``ir_log(ID,DV)``
    Declare scalar variable of type boolean named ID, default value

``ir_str(ID,LEN,DV)``

    Declare scalar string named ID, max len LEN, default value

``Vir_dbl(ID,NELEM,DV)``
    Declare vector variable of type double named ID, with NELEM0m
    elements, default value DV.

``Vir_int(ID,NELEM,DV)``
    Declare vector variable of type integer named ID, with NELEM
    elements, default value DV.

``Vir_log(ID,NELEM,DV)``
    Declare vector variable of type boolean named ID, with NELEM
    elements, default value DV.

``Vir_str(ID,LEN,NELEM)``
    Declare vector string named ID, with NELEM elements, max len LEN.
    Note that string vectors cannot set a default value.

``Callback(ID,NPRM,NRET)``
    Declare a Lua callback function named ID, with NPRM parameters,
    returning NRET double precision values.

``ir_reference(ID)``
    Declare an IREP variable as a Lua reference. In practice, Lua stores
    an (integer) reference to the matching element in the Lua table, for
    later use by the host code. This is effectively an escape mechanism
    to allow embedding of arbitrary Lua tables, functions, or strings
    into the well known table. The host code will need to be specialized
    to handle each ``ir_reference`` value. See also ``ir_get_stringref``.

``ir_ptr(ID)``
    Declare an IREP variable as a C pointer. This will normally require
    some special treatment by the IREP reader. At the present time, the
    only use is internal, by the ``Callback`` macro.

``Doc(( a comment ))``
    Add an inline comment. This macro must occur at the end of a line
    containing one of the other IR macros. It can be multi-line and can
    contain free-form RST.

Reading a Well Known Table
--------------------------

The host code finds and loads (compiles) Lua input using the normal
``luaL_loadfile``, ``lua_pcall`` sequence. If the Lua input contains a
well-known table named, say, ``table1``, that table can be read into the
IREP data store using the call:

.. code-block:: C

   int n = ir_read(L, "table1");

This recursively reads the entire table. If ``table1`` itself contains a
subtable ``table2``, that table could be separately read as:

.. code-block:: C

   int n = ir_read(L, "table1.table2");

That is, ``ir_read`` can start at any point in a Lua table, and reads
recursively from that element down. The syntax of the second argument is
equivalent to the Lua syntax that would reference the same element.
Reading just the subtable ignores elements in ``table1`` that are outside
of ``table2``. If the environment variable ``irep_debug`` is set to a
positive integer value, ``ir_read`` will produce a listing to stderr of
each variable read from the Lua table.

.. _lua-callback-functions:

Lua Callback Functions
^^^^^^^^^^^^^^^^^^^^^^

Lua functions are stored in the IREP data store using a ``struct``:

.. code-block:: C

   Beg_struct(lua_cb_data)
     ir_int(fref, -1) // -1 == LUA_REFNIL
     ir_int(npnr, -9) // packed nprm,nret
     ir_ptr(data)
   End_struct(lua_cb_data)

The ``fref`` component stores a Lua reference to the (Lua)
function.  The ``npnr`` component packs the NPRM and NRET values
into a single integer, to save space.  The packing algorithm
effectively will allow values for NPRM in the range [-9,1014] and
for NRET, the range [-9,2097142].	(But note that the enforced
lower bound for NPRM and NRET is currently -1, not -9.)  Values
for the NPRM parameter have the following meaning:

.. code-block:: C

   nprm == -1 // The Lua function accepts a variable number of arguments.
   nprm >=  0 // The Lua function accepts exactly NPRM arguments.

Values for the NRET parameter have the following meaning:

.. code-block:: C

   nret == -1 // The Lua function will return a table of any length.
   nret >=  0 // The Lua function will return exactly NRET values.

This is important to IREP because, for convenience and
efficiency, IREP allows an extended set of Lua types to be
treated as functions.  Some examples may be useful.

.. code-block:: C

   Callback(f1,2,1)  // A function with NPRM==2, NRET=1.

For f1, either of the following Lua inputs is legal:

.. code-block:: C

   f1 = function(x,y) return math.sin(3) end
   f1 = math.sin(3)

The first input is a true Lua function, that happens to return a
constant value.	 The second input is a number, obviously
constant, which IREP will also allow as the definition of ``f1``.  In
the second case, IREP uses the ``data`` component of the
``lua_cb_data`` structure associated with ``f1``.  It allocates space
for one double precision number, and stores math.sin(3) at that
location.  It sets the ``fref`` component to LUA_NOREF as an
indication that there is no true Lua function to call in this
case.  Later on, the host code may evaluate ``f1``.  If
``fref==LUA_NOREF``, the evaluator can use the stored value.  Here
is a second example:

.. code-block:: C

   Callback(f2,3,3)  // A function with NPRM==3, NRET=3.

For f2, any of the following Lua inputs is legal:

.. code-block:: lua

   f2 = function(x,y,z) return 3,4,5 end
   f2 = { 3,4,5 }
   f2 = 3

The first input is a true Lua function, returning 3 values, which
also happen to be constant in this example.  The second input is
Lua table, containing the same values returned by the function.
IREP treats the table like it did the number in the first
example: It allocates space for the 3 values, and stores them in
the ``data`` component for later use by the host code.  The third
input is also legal.  The scalar number will be treated as if the
input was

.. code-block:: lua

   f2 = { 3,3,3 }.

That is, broadcast to fill the 3 return slots expected for this
function.  Finally, the case where ``NRET == -1``:

.. code-block:: C

   Callback(f3,3,-1) // A function with NPRM==3, NRET=-1.

In this case, IREP expects the function to return a table (a
rank-1 array of double precision numbers) of arbitrary length.	A
true Lua function can of course calculate a new table each time
it is called:

.. code-block:: lua

   f3 = function(x,y,z) return { x, x+y, x+y+z } end

Note that in this case, the function returns a table, distinct
from the list returned by the function in example two above.  Or
the input can be a (constant) Lua table:

.. code-block:: lua

   f3 = { 1,2,3,4,5 }

If it is a table, IREP allocates space and stores the table
values in the ``data`` component, as before.  In this case, it also
modifies the ``npnr`` component to reflect the actual number of
elements read from the Lua table.

Return Values
-------------

* ``ir_read`` returns the number of errors encountered.

* ``ir_exists`` returns 1 if the element is found in the Lua input, 0 if
  not.

* ``ir_rtlen`` returns -1 if the given Lua value is not present, 0 if the
   value is a (scalar) number, or whatever ``lua_objlen`` returns
   otherwise. For a string, this is the length of the string. For an
   array, it is the same as the ``#`` operator in Lua.


Bugs
----

Variables in the IREP data store are generally static. This is a
consequence of current ``ISO_C_BINDING`` rules for sharing data between
Fortran and This means that the maximum length of arrays (and therefore
strings) in the IREP data store is fixed at compile time.


Author
------

IREP was created by Lee Busby at Lawrence Livermore National Laboratory.

See also
--------

See :ref:`irep-overview` for more discussion
of some of the code in IREP.
