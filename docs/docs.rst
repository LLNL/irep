.. _irep-doc-generation:

Generating IREP documentation
=============================

The ``irep-generate`` command described in the :ref:`build documentation
<irep-generate>` can be used to auto-generate `sphinx
<https://www.sphinx-doc.org/>`_ documentation for WKT headers. You can
include the generated ``.rst`` files in your code's documentation.

``irep-generate --mode rst``
----------------------------

To generate ``.rst`` files for WKT headers, run ``irep-generate --mode
rst`` as follows:

.. code-block:: console

   $ irep-generate --mode rst wkt_foo.h wkt_bar.h wkt_baz.h

This will generate ``foo.rst``, ``bar.rst``, and ``baz.rst``, and it will
also copy in an ``irep_types.rst`` file that describes IREP data types.
``foo.rst`` and friends will contain referencs to type descriptions in
this file.

You can see what the generated documentation looks like here:

.. toctree::

   wkt/mesh

``Doc`` macros
------------------

You can annotate any WKT header with a ``Doc`` macro like this::

   ir_int(nx, 1)        Doc(( Number of grid cells in 1st dimension ))

If ``irep-generate`` finds a ``Doc`` string in an IREP header, it is
carried into the **Description** field in tables in the generated IREP
documentation. ``Doc`` strings can be considered free-form RST, so you
can use inline makup to create more complex descriptions. For example, if
you wanted to use the RST ``::`` construct to include some preformatted
text, you coudl do that like this::

   ir_int(nx, 1)        Doc((

   ::

        This is a longer descrpition:
        - thing one
        - thing two
        - etc.
       ))

Basically, the ``Doc`` string is one paragraph of text, and will be
reformatted (nicely) in the HTML output.

Introductions
-------------

You can have IREP include introductory text in your table definitions. If
you are generating code for ``wkt_mesh.h``, and ``irep-generate finds
``mesh/introduction.rst`` in the current directory, then it will be
included at the top of the documentation for ``wkt_mesh.h``.

By default, ``irep-generate`` searches the current directory, but you can
tell it to look else where with ``--doc-dir <path>``.

Thre is an example ``introduction.rst`` file in the documentation for
IREP itself. See that for an example for how you might set up your own
docs.

Details files
-------------

If additional details for elements, or groups of elements, are desired,
you can write a *details file*. These go alongside the
``introduction.rst`` files described above. If ``irep-generate`` finds a
details file under ``mesh/`` when generating code for ``wkt_mesh.h``, it
will generate a link to it in the generated documentation.

The naming of the details file follows a simple rule. For example,
consider the parameter::

    material.opacity.polynomial.absorption.e_pow

The following filenames are searched in order:

    1. ``mesh/box-xmax.rst``
    2. ``mesh/box.rst``
    3. ``mesh/mesh.rst``

That is, the search first looks for the most specific possible file. If
your tables are deeper than our examples, you can keep adding levels
separate by dashes (``-``). If one of these names exists, it is linked as
the "details" file for that parameter. If not, the search goes to the
closest surrounding subtable. (So you could give details on all the
parameters in the "absorption" table by making a details file with the
second name in the list above.) And so on. The search ends by looking for
a details file "material.rst". If none are found, no details link is made
for the element.

As with ``introduction.rst``, you can use the ``--doc-dir`` parameter to
``irep-generate`` to tell it where to look.
