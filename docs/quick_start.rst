.. Copyright 2016-2021 Lawrence Livermore National Security, LLC and other
.. IREP Project Developers. See the top-level LICENSE file for details.
..
.. SPDX-License-Identifier: MIT

.. _quick-start:

===========
Quick Start
===========

A quick way to see what is happening is to run the C pre-processor by
hand:

.. code-block:: console

   gcc -E wkt_material.h | more
   gcc -E -DIREP_LANG_FORTRAN wkt_material.h | more
   gcc -E -DIREP_LANG_LUA     wkt_material.h | more
   gcc -E -DIREP_GENERATE     wkt_material.h | more
   irep-generate --mode lua wkt_material.h | more

In other words, the ``wkt_*`` files produce output that can be read from
C/C++, Fortran, or RST documentation. The output is similar in meaning,
although not necessarily identical, in each language.

By default, running a ``wkt`` header through the C preprocessor will generate C
output. This allows them to be included without any special handling in C and
C++ code. IREP's tooling (specifically, the ``irep-generate script`` uses the
``IREP_LANG_*`` macros to generate code in other languages from ``wkt`` headers
writtein in C.

The Intermediate Representation (IR) is a tool for constructing a set of
C/C++ and Fortran data structures, and a tool for reading Lua tables into
those structures. It is built around the observation that the textual
representation of Lua table elements can frequently be mapped directly
into a C/C++ struct, or a Fortran derived type. Suppose a Lua table
constructor is given as follows:

.. code-block:: lua

   t1 = {
     t2 = {
       x = 42,
     }
   }

With appropriate prior definitions, we could alternatively write:

.. code-block::

   t1.t2.x = 42 -- Lua
   t1.t2.x = 42 // C/C++
   t1%t2%x = 42  ! Fortran

The fundamental operation of the IR reader is to construct text of the
second form above, and pass it to an interpreter which executes an
assignment statement in the compiled languages. See :ref:`irep-reference`
for additional information about the system.

Naming conventions
------------------

I (L. Busby) have adopted the general convention of naming types by
prepending ``irt_`` to the name of a given table. So if ``material`` is
the name of a Lua table, the type name of its associated structure in the
host code (C/C++ or Fortran) will be ``irt_material``. This convention
extends to the subtables. Materials has a subtable "opacity", which has a
subtable "server". Suppose you want to work with the contents of a
particular "server" table. The naming convention makes it easy to get a
reference to any structure in the IR:

.. code-block:: C

   irt_server *my_svr = &material[1].opacity.server;
   printf("Library: %s\n", strndup(IR_STR(my_svr->library)));

Limitations and constraints
---------------------------

Lua tables can express combinations of data and data types in ways that
are not easy to represent in C/C++ or Fortran structured data objects.
The IR itself is shared between C/C++ and Fortran using Fortran's
``ISO_C_BINDING`` machinery.  This is not generally an issue for integer,
double, and logical variables, but it does affect the representation
of character strings.  There are some additional limitations imposed
by the Fortran namelist statement, and limitations in how well actual
compilers support all the features.  Assignments are effectively parsed
according to Fortran rules.  The major difference for C/C++ (and Lua)
programmers to recall is that Fortran identifiers are case-insensitive.

The global structs IREP generates are statically declared, so the size of all
array types is given and fixed at compile time.

Strings
-------

Character strings are stored in the IR as blank-delimited arrays, one
character per array element.  String vectors are stored as two-dimensional
arrays of characters.  Strings are null-terminated.  String length is
currently limited to 512 characters.  It is usually best to access
character strings using one of the convenience functions provided.
If table ``t`` contains a scalar string ``ss`` and a vector of strings ``vs``:

    // C++
    std::string my_ss(IR_STR(t.ss));    // Access scalar string
    std::string my_vs(IR_STR(t.vs[0])); // First element of vector string

    ! Fortran
    character(len=64) :: my_ss, my_vs
    my_ss = trim(ir_fstr(t%ss))          ! Access scalar string
    my_vs = trim(ir_fstr(t%vs(:,1)))     ! First element of vector string

``IR_STR`` and ``ir_fstr`` basically depend on ``sizeof``, or its Fortran
equivalent. So they can be used (only) with operands for which ``sizeof``
will return a sensible value. Note that default values can be given only
for scalar strings in the IR header files.


Compilers
---------

Gfortran received a critical patch at version 4.7.4, that allowed
it to properly read derived type components in namelist statements.
That version or later is necessary for the IR to function correctly.
Portland Group compilers 14.7 and 15.1 were tested, and appear to
function correctly.  IBM bgxlf and bgxlc, version 14.1, were tested,
and appear correct.  Intel ifort 14.0 was tested, and appears to give
correct results.


Callback functions
------------------

The IREP ``lua_cb_data`` struct has an integer component ``fref``, which
nominally contains a Lua reference to the associated Lua callback
function.  We overload the meaning of the possible fref values to cover
several cases of interest::

    Value of fref     Meaning
    ---------------------------------------------------------------------
    LUA_REFNIL        No entry was in the Lua input.
    LUA_NOREF         Lua input is a constant function, number or vector.
    <something else>  Lua input is a real Lua function

Fref is statically initialized to ``LUA_REFNIL``.  If no actual
Lua input for the given function name is found, this will therefore be
the value of fref.

If an input entry is found, we check the type.  If it is a number,
the value of fref is set to ``LUA_NOREF``, and the number is stored in
the structure's ``const_val`` component.  If the entry is instead a Lua
function, we put the function in the Lua registry and store a reference
to it in ``fref``.

In particular, we can use the value of fref to infer whether or not
any number or function was read::

    fref == LUA_REFNIL ==> No entry of any kind was available;
    fref != LUA_REFNIL ==> Some entry (number or function) was read.
