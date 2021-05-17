===============================
Building IREP into applications
===============================

Integrating IREP into an application requires that you write ``wkt_*.h``
headers (as described in the :ref:`overview <irep-overview>`) *and* that
you generate libraries from those headers, which you can then link into
your application. Since IREP's internal data structures are statically
defined, and since IREP's library routines (like ``ir_read()``) need to
look up tables dynamically, linking IREP into an application requires
some special steps.

IREP's :ref:`build system integration <build-system-integration>` can
ease this process. We'll describe that later, but before we do, we'll
define some terms.

``wkt`` and ``wkt-index`` libraries
-----------------------------------

To use IREP, you'll need to generate at least two types of libraries:

1. One or more ``wkt`` libraries that define the static data structures
   in particular WKT headers, and
2. A ``wkt-index`` library that tells IREP how to look up data by name in
   those structures.

A simple example
^^^^^^^^^^^^^^^^

Suppose that you have an application with modules `lib1` and `lib2`.

.. graphviz::

   digraph {
     "app" -> "liba"
     "app" -> "libb"
   }

And suppose that your application has a WKT header, ``wkt_app.h``, that
defines its input format. To use IREP, you'll need to generate a WKT
library and a WKT index from ``wkt_app.h``, then link everything together
with IREP. The figure below shows the dependency relationships between
your application components and the various IREP libraries:

.. graphviz::

   digraph {
     irep   [shape=record,
            label="{libIR.a}"]

     "app" -> "liba"
     "app" -> "libb"

     "libapp-wkt.a" [shape=record,
            label="{libapp-wkt.a | {wkt_app.h}}"]

     "libapp-wkt-index.a" [shape=record,
            label="{libapp-wkt-index.a | {wkt_app.h}}"]

     "app" -> "liba"
     "app" -> "libb"

     "app" -> "irep"
     "app" -> "libapp-wkt.a"
     "app" -> "libapp-wkt-index.a"
   }


Adding components to larger applications
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

IREP is designed to be modular, so that different components in an
application can define different parts of an input format, or so that
components can share an input format. Suppose that you want to introduce
``wkt_physics.h`` and ``wkt_geometry.h`` that define settings for physics
and geometry in your application, *and* in ``liba`` and ``libb``. That
is, ``app``, ``liba``, and ``libb`` need to include these headers and
access data from these parts of the input format.

To deal with this, you need only introduce one more WKT library, and
include the new headers in your WKT index. Let's call it
``libshared-wkt.a``, as it is shared by all three of your app's
components. Here are the new dependency relationships:

.. graphviz::

   digraph {
     irep   [shape=record,
            label="{libIR.a}"]

     "libshared-wkt.a" [shape=record,
            label="{libshared-wkt.a | {wkt_physics.h} | {wkt_geometry.h}}"]

     "libapp-wkt.a" [shape=record,
            label="{libapp-wkt.a | {wkt_app.h}}"]

     "libapp-wkt-index.a" [shape=record,
            label="{libapp-wkt-index.a | {wkt_app.h} | {wkt_physics.h} | {wkt_geometry.h}}"]

     "app" -> "liba"
     "app" -> "libb"

     "app" -> "irep"
     "app" -> "libapp-wkt.a"
     "app" -> "libapp-wkt-index.a"
     "app" -> "libshared-wkt.a"

     "liba" -> "libshared-wkt.a"
     "liba" -> "irep"

     "libb" -> "libshared-wkt.a"
     "libb" -> "irep"
   }

All the dependencies that were present when only ``app`` was using IREP
are still there, but now all components need to link to ``libIR.a`` (so
that they can call routines like ``ir_read()``), and all components link
to ``libshared-wkt.a``, as it contains the ``struct`` definitions for the
new WKTs. Generating WKT libraries also creates the Fortran ``.mod``
files needed to include WKT headers into Fortran programs.

``libapp-wkt-index.a`` has now expanded -- it is now generated from
``wkt_app.h``, ``wkt_physics.h``, *and* ``wkt_geometry.h`` -- all the
WKTs known to any part of the application. Again, this is necessary
because this library tells ``ir_read()`` where to find data. Without it,
``ir_read()`` cannot look up IREP fields by name.

If you expand your application to include more components, you can add
new WKT libraries as needed by new components, and you'll need to
remember to update the list of WKTs the index with each new component.

.. _build-system-integration:

Build system integration
------------------------

It may seem daunting to write the code for these libraries, but IREP
includes integration for GNU Make and for CMake that makes it simple to
create them.

GNU Make
^^^^^^^^

To use IREP from GNU make, you simply import the ``wkt.mk`` file that
comes with IREP, and it defines implicit rules for building WKT and WKT
index libraries. You need only say what to build and what source files
(WKT headers) to include.

Let's look at an example ``GNUMakefile`` based on our example above.
Assume that ``irep.root`` is set to the path to your IREP installation:

.. code-block:: makefile

    # include this file to use IREP's gmake integration
    include $(irep.root)/share/irep/wkt.mk

    # define the libraries you want with -wkt.a and -wkt-index.a suffixes
    all: libapp-wkt.a libapp-wkt-index.a

    # index libraries must define <name>.wkt_index_src to include all WKT
    # headers from the application
    app.wkt_index_src = wkt_app.h wkt_physics.h wkt_geometry.h

    # WKT libraries need <name>.wkt_src defined.
    # libshared-wkt.a (w/geometry and physics) would be defined elsewhere
    app.wkt_src = wkt_app.h

    # You can add defines to CPPFLAGS so that WKT headers can use them
    # You must export CPPFLAGS so that irep-generate can see it.
    CPPFLAGS += -DSPECIAL_OPTION -I$(shared_wkt_dir)
    export CPPFLAGS

    # this rule cleans up the generated .a, .f, .o, and .mod files
    .PHONY: clean
    clean:
        rm -f *.[afo] *.mod

If you only wanted to generate a WKT library, you could remove the
required ``libapp-wkt-index.a`` target and not set ``app.wkt_index.src``.

The Makefile above generates the libraries for you, but you'll still need
to ensure that they and ``libIR.a`` are linked into your application.

CMake
^^^^^

To use IREP from CMake, you'll first need to include the IREP directory
in `CMAKE_PREFIX_PATH
<https://cmake.org/cmake/help/latest/variable/CMAKE_PREFIX_PATH.html>`_.
You probably want to include this on the command line when you invoke
``cmake``, like this:

.. code-block:: console

   $ cmake -DCMAKE_PREFIX_PATH=/path/to/irep;/other/path ...

Once that is done, you'll be able to call ``find_package(irep)`` in your
CMake code. This will import two useful CMake functions:
``add_wkt_library()`` and ``add_wkt_index_library()``. Here's a CMake
example that is equivalent to the GMake one above:

.. code-block:: cmake

    # find irep
    find_package(irep REQUIRED)

    add_wkt_library(app-wkt wkt_app.h)
    target_include_directories(app-wkt PUBLIC -I${shared_wkt_dir})
    target_compile_definitions(app-wkt PUBLIC -DSPECIAL_OPTION)

    add_wkt_index_library(app-wkt-index wkt_app.h wkt_physics.h wkt_geometry.h)
    target_include_directories(app-wkt-index PUBLIC -I${shared_wkt_dir})
    target_compile_definitions(app-wkt-index PUBLIC -DSPECIAL_OPTION)

The libraries are created as CMake targets with calls to
``add_wkt_library()`` and ``add_wkt_index_library()``, and what was
handled with ``CPPFLAGS`` in GMake is handled for each library with
``target_include_directories`` and ``target_compile_definitions``.

Now you can use the ``app-wkt`` and ``app-wkt-index`` targets as you
would any other in CMake -- you'll need to add them to your application
to ensure that it links correctly.

.. _irep-generate:

The ``irep-generate`` command
-----------------------------

You've already seen how IREP integrates with build systems, but if you
need more than this, you can use the ``irep-generate`` command to handle
all the magic yourself. ``wkt.mk`` and IREP's CMake integration use this
command to generate C and Fortran code for WKT and WKT index libraries.
It is also used to :ref:`generate documentation <irep-doc-generation>`
from WKT headers.

.. it would be great to use command-output here, but we can't because
.. readthedocs does not have Lua in its environment, and we cannot
.. install system packages on RTD.  If that becomes possible, this
.. should just invoke irep-generate.

``irep-generate`` usage is pretty simple -- it looks like this::

    ./irep-generate --help
    irep-generate [-h] [--mode PART] HEADERS

    This script generates source code for IREP well-known-table (WKT)
    libraries.

    Generated code is printed to standard out.

    Positional Arguments:
      HEADERS            wkt_*.h files from which to generate code, e.g.:
                           wkt_geometry.h wkt_physics.h ...

    Generation options
      --mode index       generate only index (default)
      --mode fortran     generate fortran module from a single wkt header
      --mode lua         generate loadable, nested lua tables
      --mode rst         generate restructured text (.rst) documentation

      --module-name      name for generated module (fortran mode only,
                         inferred from header name by default)

    Documentation options (for use with --mode rst)
      --doc-dir DIR      documentation directory where we look for
                         details/intros for WKTs (default: .)

    Environment variables:
      CPP                C preprocessor to use (default gcc -E)
      CPPFLAGS           flags for the C preprocessor

    Help:
      -h, --help         display this message

``irep-generate`` can be used to translate ``wkt*.h`` files into C, Fortran,
Lua code, and it can also generate RST documentation. The particular mode
of generation used is controlled by the ``--mode`` flag.

The most interesting options here are ``--mode index`` and ``--mode
fortran``.

Fortran generation
^^^^^^^^^^^^^^^^^^

Running:

.. code-block:: console

   $ irep-generate --mode fortran wkt_foo.h

will generate ``wkt_foo.f``, which can be compiled into a Fortran
``.mod`` file and a ``.o`` file. The ``.o`` file contains the definition
for the table structures, and the ``.mod`` file is includable by Fortran
code. Createing a WKT library is just a matter of including all these
``.o`` files into a single library. So, WKT libraries just contain
structure definitions for your IREP data -- nothing else.

Index generation
^^^^^^^^^^^^^^^^

Running:

.. code-block:: console

   $ irep-generate --mode index wkt_foo.h wkt_bar.h wkt_baz.h

Will generate C code that IREP uses to look up the location of fields
from well known tables. This needs to be run on by each application using
IREP, over all of the ``wkt*.h`` files included in that application. If
WKT files are not included, bad things can happen, so you should ensure
that you use the same ``CPPFLAGS`` and the same headers that you did when
you generated your WKT libraries.

Controlling code generation
^^^^^^^^^^^^^^^^^^^^^^^^^^^

``irep-generate`` invokes the C preprocessor to translate ``wkt_*.h``
files into something easy to parse. This adds some flexibility to the
IREP model, because your table definitions can include other code,
optional segments using ``#ifdef``, etc. You can control how the
preprocessor is invoked through the following environment variables:

* ``CPP``: Set this to the preprocessor you want to use. The default is
  ``gcc -E``, but if you build with another compiler you may want to swap
  it in.

* ``CPPFLAGS``: To pass flags to ``irep-generate``, you need only ensure
  that the standard ``CPPFLAGS`` variable is set when you run
  ``irep-generate``.  You can add ``-I`` and ``-D`` directives here.

As mentioned above, it is important to ensure that you use the same
``CPPFLAGS`` in all the places where you call ``irep-generate``. Not
doing so can cause IREP to be confused when it tries to find fields in
its data structures.

Lua code generation
^^^^^^^^^^^^^^^^^^^

Running:

.. code-block:: console

   $ irep-generate --mode lua wkt_foo.h

Will generate importable Lua code for a WKT module. This is useful for
seeing, in simple Lua tables, what the data looks like in a WKT. You can
also use it to generate a skeleton input deck.
