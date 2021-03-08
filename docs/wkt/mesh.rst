===============================================================
The ``mesh`` table
===============================================================

.. include:: ./mesh/introduction.rst



.. include:: <isonum.txt>

.. |br| raw:: html

   <br/>

.. list-table::
   :widths: 15 5 80
   :header-rows: 1

   * - Name / Type
     - Default
     - Description
   * - .. _mesh.amr:

       **amr** |br| :ref:`boolean <irep-boolean>`
     - ``false``
     - Use amr?
   * - .. _mesh.file:

       **file** |br| :ref:`string(256) <irep-string>`
     - ``'none'``
     - Name of mesh
   * - .. _mesh.refinement_level:

       **refinement_level** |br| :ref:`integer <irep-integer>`
     - ``0``
     - 

.. _mesh.subtable.box:

subtable ``box``
---------------------------------------------------------------

.. list-table::
   :widths: 15 5 80
   :header-rows: 1

   * - Name / Type
     - Default
     - Description
   * - .. _mesh.box.nx:

       **nx** |br| :ref:`integer <irep-integer>`
     - ``1``
     - Number of grid cells in 1st dimension
   * - .. _mesh.box.ny:

       **ny** |br| :ref:`integer <irep-integer>`
     - ``1``
     - Number of grid cells in 2nd dimension
   * - .. _mesh.box.nz:

       **nz** |br| :ref:`integer <irep-integer>`
     - ``1``
     - Number of grid cells in 3rd dimension
   * - .. _mesh.box.xmax:

       **xmax** |br| :ref:`double <irep-double>`
     - ``1.0``
     - Location of right-most cell face
   * - .. _mesh.box.xmin:

       **xmin** |br| :ref:`double <irep-double>`
     - ``0.0``
     - Location of left-most cell face
   * - .. _mesh.box.ymax:

       **ymax** |br| :ref:`double <irep-double>`
     - ``1.0``
     - Location of farthest cell face
   * - .. _mesh.box.ymin:

       **ymin** |br| :ref:`double <irep-double>`
     - ``0.0``
     - Location of nearest cell face
   * - .. _mesh.box.zmax:

       **zmax** |br| :ref:`double <irep-double>`
     - ``1.0``
     - Location of highest cell face
   * - .. _mesh.box.zmin:

       **zmin** |br| :ref:`double <irep-double>`
     - ``0.0``
     - Location of lowest cell face
