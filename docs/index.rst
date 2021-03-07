.. Copyright 2016-2021 Lawrence Livermore National Security, LLC and other
.. IREP Project Developers. See the top-level LICENSE file for details.
..
.. SPDX-License-Identifier: MIT

IREP
====

IREP is a tool that enables mixed-language simulation codes to use a
common, Lua-based format for their input decks. Essentially, the input
format is a set of `tables <https://www.lua.org/pil/2.5.html>`_ -- Lua's
one (and only?) data structure. IREP is an *intermediate representation*
that is used to generate plain-old-data (POD) structures in C, C++,
Fortran, and Lua.

For some background and motivation, see this :download:`presentation
<_static/files/llnl-pres-703906.pdf>` and the corresponding `video
<https://www.youtube.com/watch?v=xTxApP7dPFk>`_.

.. toctree::
   :maxdepth: 3
   :caption: Contents:

   overview
   quick_start
   build
   docs
   reference
   irep_types

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
