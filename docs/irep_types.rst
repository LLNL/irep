.. Copyright 2016-2021 Lawrence Livermore National Security, LLC and other
.. IREP Project Developers. See the top-level LICENSE file for details.
..
.. SPDX-License-Identifier: MIT

=========================
About IREP data types
=========================

.. include:: <isonum.txt>

IREP is a tool for generating an "intermediate representation" of Lua
input decks for simulation codes. It allows Lua inputs to be read as
plain old struct data from multiple languages (C, C++, and Fortran).

This file provides a quick refernce on IREP data types. More on IREP can
be found `on GitHub <https://github.com/llnl/irep>`_.


.. _irep-double:

Doubles
-------

All Lua numbers are double precision floating point numbers, conforming
to the IEEE 754 standard. You can write numeric constants in a Lua input
file using any of the usual forms:

    4 0.4 4.57e-3 0.3e12 5E+20

Although IREP has both C++ and Fortran code internally, Lua input is the
same for both. Note that Lua does *not* accept otherwise standard Fortran
syntax for double precision constants: ``4.57d-3``. You must use ``e`` or
``E`` to indicate the exponent.


.. _irep-vector-double:

Vector doubles
--------------

Like scalar doubles, ``vector doubles`` are stored as double precision
floating point numbers. A vector is a 1-dimensional array of scalars, and
is defined in Lua using the usual syntax:

.. code-block:: lua

    x = { 3, 6, 9 }

This defines an array ``x`` such that ``x[1] == 3``, etc. In Lua, the
length of the array could vary, and is given by the set of values
assigned to it. When using IREP, the maximum length of an input vector is
limited. The limit is denoted in brackets (e.g., ``double[10]``) in the
documentation.

Sometimes the maximum number of elements is a not-to-exceed upper bound.
This is usually some high number like ``double[99]``. For other fields,
the maximum number of elements is exactly the number of elements you are
required to enter, e.g. ``double[3]`` for a 3-dimensional vector. IREP
does not distinguish between the two cases so you'll need to consult the
documentation for each field.


.. _irep-integer:

Integers
--------

Lua integers are represented internally by double precision floating
point numbers. Thus, all integers whose absolute value is less than or
equal to 2^53 can be represented exactly as a Lua number.

In Lua, the literal constant ``37`` is the same value as ``37.0``.
It would not be a (Lua) error to write

.. code-block:: lua

    x = 37
    x = 37.0
    x = 37.000001

However, IREP *does* distinguish integer input parameters from double
precision parameters. If ``x`` is an integer input parameter, the third
line above would produce an error.


.. _irep-vector-integer:

Vector integers
---------------

Integer vector input variables are 1-dimensional arrays of integers:

.. code-block:: lua

   i = { 5, 10, 15, 20, 25, 30 }

In Lua, the length of the array might vary. With IREP, the maximum length
is limited, and is denoted in brackets (e.g., ``integer[10]``) in the documentation.


.. _irep-boolean:

Booleans
--------

The Lua boolean type has two values ``false`` and ``true``. These produce
the expected result when they represent a IREP boolean or logical
parameter.

In Lua, conditional tests consider either ``false`` or ``nil`` as
logically false. Everything else is ``true``, including the number ``0``
and the empty string, ``''``.


.. _irep-vector-boolean:


Vector booleans
---------------

Boolean or logical vector input variables are 1-dimensional arrays of
boolean or logical values:

.. code-block:: lua

    bb = { true, false, true }

In Lua, the length of the array can vary. With IREP, the maximum length
is limited, and is denoted in brackets (e.g., ``boolean[10]``) in the
documentation.


.. _irep-string:

Strings
-------

Lua strings can be written using either single quote or double quote to
delimit them. In Lua, the length of the string varies with the length of
the value assigned to it. However, IREP input parameters do have a
specific maximum length. The maximium length of a string is denoted in
parentheses, e.g. ``string(256)`` in the documentation.


.. _irep-vector-string:

Vector strings
---------------

String vector input variables are 1-dimensional arrays of strings:

.. code-block:: lua

      s = { "abc", "defg", "hijkl" }

In Lua, the length of the array, and the length of each string in the
array might vary. In IREP, the maximum number of elements is limited, and
is denoted in brackets (e.g., ``string(32)[64]``) in the documentation.
The maximum length of each string is shown in parentheses, as with scalar
strings.


.. _irep-callback:

Callback functions
------------------

Lua functions are an important part of IREP interfaces. They can be used
to define fields, e.g., you might define a physical field as a functions
of three space variables and one time variable.

All input parameters for IREP's Lua functions are double precision
numbers, as are all the return values. If the function returns one value,
it defines a *scalar* field. Functions returning multiple values can be
used to define *vector* (or sometimes *tensor*) fields.

In the documentation, the arity (number of parameters) of a function is
denoted with a ``/``, and the number of return values is shown after a
right arrow (|rarr|). For example, a function that takes one parameter
and returns three values would be denoted as:

    callback/1 |rarr| 3

Functions that return *constant* values are common enough to receive
special treatment in IREP. You can always write a constant function as a
normal Lua function:

.. code-block:: lua

    initial_conditions = {
      temperature = function(x,y,z,t) return 0.0 end,
    }

would set the temperature field to zero everywhere, for all times. More
conveniently, in IREP, you can also write it as

.. code-block:: lua

    initial_conditions = {
      temperature = 0.0,
    }

IREP knows that ``initial_conditions.temperature`` is a function, so it
will interpret the second syntax correctly. If the function in question
is vector-valued (and you want to return a constant value), you can
either write a vector function, use an *array*, or use a scalar. A scalar
value will be *broadcast* to all components of the return value. (But
note that broadcasting only applies to constant scalar return values --
if you write a Lua function, it must return all required components.)

.. code-block:: lua

    initial_conditions = {
      velocity = function(x,y,z,t) return 1.0, 2.0, 3.0 end,
    }

    initial_conditions = {
      velocity = { 1.0, 2.0, 3.0 },
    }

    initial_conditions = {
      velocity = 0.0, -- equivalent to { 0,0,0 }
    }

Defining an IREP callback function as a number or array generally
executes faster than calling a regular Lua function, so this is a good
technique to keep in mind.

One other special kind of callback function defines the number of return
values as ``-1`` (e.g., callback/3 |rarr| -1). This means that the
function can return a one-dimensional array of arbitrary length. (Or you
can return a Lua ``table``). Function-specific documentation should
normally give some additional information to help you write an actual
instance of the function.
