# IREP

IREP is a tool that enables mixed-language simulation codes to use a
common, Lua-based format for their input decks. It generates an
*intermediate representation* for the input data, which can then be read
conveniently from multiple languages (C, C++, Fortran, and Lua).

## QUICK START

A quick way to see what is happening is to run the C pre-processor by hand:

    gcc -E -DIREP_LANG_C       wkt_material.h | more
    gcc -E -DIREP_LANG_FORTRAN wkt_material.h | more
    gcc -E -DIREP_LANG_LUA     wkt_material.h | more
    gcc -E -DIREP_GENERATE     wkt_material.h | more
    ./etc/ir2l wkt_material.h | more

In other words, the wkt_* files produce output that can be read from
C/C++, Fortran, or by a script (ir2rst, in marbl) to produce RST
documentation (case 3), or to produce code for wrapping the structures
themselves (case 4, etc/ir_generate).  The output is similar in meaning,
although not necessarily identical, in each language.  (Ir2l is a Lua
script that reassembles the pieces of a table into a nested structure
for display.  For now, it is a toy.)

The Intermediate Representation (IR) is a tool for constructing a set
of C/C++ and Fortran data structures, and a tool for reading Lua tables
into those structures.  It is built around the observation that the
textual representation of Lua table elements can frequently be mapped
directly into a C/C++ struct, or a Fortran derived type.  Suppose a Lua
table constructor is given as follows:

    t1 = {
      t2 = {
        x = 42,
      }
    }

With appropriate prior definitions, we could alternatively write:

    t1.t2.x = 42 -- Lua
    t1.t2.x = 42 // C/C++
    t1%t2%x = 42  ! Fortran

The fundamental operation of the IR reader is to construct text of
the second form above, and pass it to an interpreter which executes an
assignment statement in the compiled languages.  This particular instance
of the IR is designed to support combined input for the Blast and Miranda
codes.  See etc/doc/irep.pdf for additional information about the system.


## L2IR and IR2L (See etc/ directory.)

L2ir is a Lua script that takes a Lua table as input, and produces the
declarations that represent it as IR macros.  Ir2l does the opposite: It
turns a set of IR macros back into a nested Lua table, and displays it.
These scripts are mostly a proof of principle for now.  They have many
gaps and bugs.  But they serve to demonstrate that there is indeed a
rough isomorphism between Lua tables and C/C++ or Fortran structures.


## TYPE NAME CONVENTION

I (L. Busby) have adopted the general convention of naming types by
prepending "irt_" to the name of a given table.  So if "material" is the
name of a Lua table, the type name of its associated structure in the
host code (C/C++ or Fortran) will be "irt_material".  This convention
extends to the subtables.  Materials has a subtable "opacity", which has
a subtable "server".  Suppose you want to work with the contents of a
particular "server" table.  The naming convention makes it easy to get
a reference to any structure in the IR:

    irt_server *my_svr = &material[1].opacity.server;
    printf("Library: %s\n", strndup(IR_STR(my_svr->library)));


## LIMITATIONS, CONSTRAINTS

Lua tables can express combinations of data and data types in ways that
are not easy to represent in C/C++ or Fortran structured data objects.
The IR itself is shared between C/C++ and Fortran using Fortran's
ISO_C_BINDING machinery.  This is not generally an issue for integer,
double, and logical variables, but it does affect the representation
of character strings.  There are some additional limitations imposed
by the Fortran namelist statement, and limitations in how well actual
compilers support all the features.  Assignments are effectively parsed
according to Fortran rules.  The major difference for C/C++ (and Lua)
programmers to recall is that Fortran identifiers are case-insensitive.


## STATIC DECLARATION

The IR is statically declared, so the size of all array types is given
and fixed at compile time.


## STRINGS

Character strings are stored in the IR as blank-delimited arrays, one
character per array element.  String vectors are stored as two-dimensional
arrays of characters.  Strings are NULL delimited.  String length is
currently limited to 512 characters.  It is usually best to access
character strings using one of the convenience functions provided.
If table "t" contains a scalar string "ss" and a vector of strings "vs":

    // C++
    std::string my_ss(IR_STR(t.ss));    // Access scalar string
    std::string my_vs(IR_STR(t.vs[0])); // First element of vector string

    ! Fortran
    character(len=64) :: my_ss, my_vs
    my_ss = trim(ir_fstr(t%ss))          ! Access scalar string
    my_vs = trim(ir_fstr(t%vs(:,1)))     ! First element of vector string

IR_STR and ir_fstr basically depend on "sizeof", or its Fortran
equivalent.  So they can be used (only) with operands for which sizeof
will return a sensible value.  Note that default values can be given
only for scalar strings in the IR header files.


## COMPILERS

Gfortran received a critical patch at version 4.7.4, that allowed
it to properly read derived type components in namelist statements.
That version or later is necessary for the IR to function correctly.
Portland Group compilers 14.7 and 15.1 were tested, and appear to
function correctly.  IBM bgxlf and bgxlc, version 14.1, were tested,
and appear correct.  Intel ifort 14.0 was tested, and appears to give
correct results.


## CALLBACK FUNCTIONS

The IREP lua_cb_data struct has an integer component "fref", which
nominally contains a Lua reference to the associated Lua callback
function.  We overload the meaning of the possible fref values to cover
several cases of interest:

    Value of fref     Meaning
    ---------------------------------------------------------------------
    LUA_REFNIL        No entry was in the Lua input.
    LUA_NOREF         Lua input is a constant function, number or vector.
    <something else>  Lua input is a real Lua function

Fref is statically initialized to LUA_REFNIL.  If no actual
Lua input for the given function name is found, this will therefore be
the value of fref.

If an input entry is found, we check the type.  If it is a number,
the value of fref is set to LUA_NOREF, and the number is stored in
the structure's "const_val" component.  If the entry is instead a Lua
function, we put the function in the Lua registry and store a reference
to it in fref.

In particular, we can use the value of fref to infer whether or not
any number or function was read:

    fref == LUA_REFNIL ==> No entry of any kind was available;
    fref != LUA_REFNIL ==> Some entry (number or function) was read.

## AUTHORS

IREP was created by Lee Busby, busby1@llnl.gov.

## LICENSE

IREP is distributed under the terms of the MIT license. Copyrights in the
IREP project are retained by contributors. No copyright assignment is
required to contribute to IREP. All new contributions must be made under
the MIT license.

See [LICENSE](https://github.com/LLNL/irep/blob/master/LICENSE) and
[NOTICE](https://github.com/LLNL/irep/blob/master/NOTICE) for details.

SPDX-License-Identifier: MIT

LLNL-CODE-702338
