# IREP

[![linux builds](https://github.com/LLNL/irep/actions/workflows/linux_build_tests.yaml/badge.svg)](https://github.com/LLNL/irep/actions/workflows/linux_build_tests.yaml)
[![Documentation Status](https://readthedocs.org/projects/irep/badge/?version=latest)](https://irep.readthedocs.io/en/latest/?badge=latest)

IREP is a tool that enables mixed-language simulation codes to use a
common, Lua-based format for their input decks. Essentially, the input
format is a set of [tables](https://www.lua.org/pil/2.5.html) -- Lua's
one (and only?) data structure. IREP is an *intermediate representation*
that is used to generate plain-old-data (POD) structures in C, C++,
Fortran, and Lua.

## Documentation

There are some high-level docs here to get you started, but you can find
more in the [full IREP Documentation](https://irep.readthedocs.io/).

## Installation

To build IREP, run `make` in the root directory. This will create
`libIR.a`.  IREP does not (yet) have a proper installation target.

## Building with IREP

IREP has integration for both CMake and gmake-based builds. You can find
more about how to intgrate IREP into your project here:

* [IREP Build Docs](https://irep.readthedocs.io/en/latest/build.html) on ReadTheDocs.
* [IREP Examples](https://github.com/LLNL/irep/tree/master/examples) in this repository.

You can build and run the examples by running running `make test` in the
root of this repository.

## Basics

To use IREP, you can write a header, like this one for defining a
very simple structured mesh. Let's call it `wkt_mesh.h`:

```c
#ifndef wkt_mesh_h
#define wkt_mesh_h
#include "ir_start.h"

Beg_struct(irt_box)
  ir_int(nx, 1)        Doc(( Number of grid cells in 1st dimension ))
  ir_int(ny, 1)        Doc(( Number of grid cells in 2nd dimension ))
  ir_int(nz, 1)        Doc(( Number of grid cells in 3rd dimension ))
  ir_dbl(xmin, 0.0)    Doc(( Location of left-most cell face ))
  ir_dbl(xmax, 1.0)    Doc(( Location of right-most cell face ))
  ir_dbl(ymin, 0.0)    Doc(( Location of nearest cell face ))
  ir_dbl(ymax, 1.0)    Doc(( Location of farthest cell face ))
  ir_dbl(zmin, 0.0)    Doc(( Location of lowest cell face ))
  ir_dbl(zmax, 1.0)    Doc(( Location of highest cell face ))
End_struct(irt_box)

Beg_struct(irt_mesh)
  ir_str(file, FILENAMESIZE, 'none')  Doc(( Name of mesh ))
  ir_int(refinement_level, 0)
  ir_log(amr, false)                  Doc(( Use amr? ))
  Structure(irt_box,box)
End_struct(irt_mesh)

// Declare the structure
ir_wkt(irt_mesh, mesh)

#include "ir_end.h"
#endif  // wkt_mesh_h
```

In IREP, headers like this are called "Well Known Tables", or "WKTs". WKT
headers are includable directly into C and C++ code, and you can refer to
elements of the defined structures like this:

```c
#include "wkt_mesh.h"

void do_something() {
    double xrange = mesh.box.xmax - mesh.xmin;
    double yrange = mesh.box.ymax - mesh.ymin;
    double zrange = mesh.box.zmax - mesh.zmin;
}
```

IREP can also be used to generate Fortran modules and Lua code for WKTs,
and you can refer to their elements in a similarly direct way in those
languages, e.g.:

```
mesh.box.xmax  // C, C++
mesh.box.xmax  -- Lua C
mesh.box.xmax  ! Fortran
```

## Using IREP for input decks

IREP's main use is to allow multi-language (usually C, C++, and Fortran)
integrated codes to read Lua input files ("input decks" for those of use
who've been in the simulation field for a while). With the `wkt_mesh.h`
we've seen so far, we could write an input file like this:

```lua
mesh = {
   file = 'my_mesh',
   refinement_level = 1,
   amr = true,
   box = {
      nx = 100,
      ny = 100,
      nz = 100,
   },
}
```

Note that the `xmin`, `xmax`, `ymin`, `ymax`, `zmin`, and `zmax` fields
in `box` are not defined in the input, but they will take on the default
values from the `wkt_mesh.h` header.

If you want to read this input file from `C`, you could write some code
like this:

```c
#include "ir_extern.h"
#include "wkt_mesh.h"
#include "lua.h"

int main(int argc, char **argv) {
    // set up lua and load some lua code
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaL_loadfile(L, "mesh_example.lua");

    ir_read(L, "mesh");
    printf("mesh.box.xmax = %d\n", mesh.box.xmax);
    printf("mesh.box.ymax = %d\n", mesh.box.ymax);
    printf("mesh.box.zmax = %d\n", mesh.box.zmax);
```

This sets up an embedded Lua interpreter, loads the Lua input file, and
then reads WKT values out of the native Lua tables and into a global
`mesh` structure that can be accessed from `C`.

## Authors

IREP was created by Lee Busby, busby1@llnl.gov.

Thanks also to `irep`'s
[contributors](https://github.com/LLNL/irep/graphs/contributors)!.

## License

IREP is distributed under the terms of the MIT license. Copyrights in the
IREP project are retained by contributors. No copyright assignment is
required to contribute to IREP. All new contributions must be made under
the MIT license.

See [LICENSE](https://github.com/LLNL/irep/blob/master/LICENSE) and
[NOTICE](https://github.com/LLNL/irep/blob/master/NOTICE) for details.

SPDX-License-Identifier: MIT

LLNL-CODE-702338
