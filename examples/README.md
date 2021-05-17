# IREP Examples

This directory contains example IREP programs in C, C++, and Fortran.

The programs themselves just read in and print out data from simple
`input.lua` files. The examples are meant to be minimal working builds,
to help you integrate IREP into your own code.

## `gmake` builds (here, C and Fortran)

The examples in the `c` and `fotran` subdirectories are `gmake`-based.

The WKT library builds in these makefiles follow the
[IREP build documentation](https://irep.readthedocs.io/en/latest/build.html#gnu-make)
closely. Specifically, they do this to build a WKT library and a WKT
index library. `irep.root` should be set to the path to your IREP installation.

```makefile
include $(irep.root)/share/irep/wkt.mk
wkt.lib = libprog-wkt.a libprog-wkt-index.a
prog.wkt_src = $(wildcard wkt_*.h)
prog.wkt_index_src = $(prog.wkt_src)
```

The interesting aspect of this build for an app builder is the final
executable build. In the makefile, that looks like this:

```makefile
$(prog): $(obj) $(wkt.lib)
    $(COMPILE.c) -o $@ $(obj) \
        -L$(irep_dir) -L. \
        -Wl,--start-group -lprog-wkt -lIR -lprog-wkt-index -Wl,--end-group \
        $(LUA_LIBRARIES) -lm -lgfortran -ldl
```

### Compile details

This just says that the main `$(prog)` requires our WKT libraries and
`c_prog.o` (or `f_prog.o` for Fortran). The build for the `.o` file is
actually implicit and comes from this rule in `wkt.mk`:

```makefile
%.o: %.c       ; $(COMPILE.c) -c $<
%.o %.mod: %.f ; $(COMPILE.f) -c $<
```

The `COMPILE.c` and `COMPILE.f` variables are defined by `wkt.mk` as
well; they invoke the C and Fortran compilers with a standard set of
arguments for IREP programs:

```makefile
COMPILE.c = $(CC) $(CFLAGS) $(CPPFLAGS)
COMPILE.f = $(FC) $(FFLAGS) $(CPPFLAGS) -DIREP_LANG_FORTRAN
```

You can (as we do in the examples) reuse `COMPILE.c` in your IREP builds,
or you could write out just the parts you want. If you want to tweak what
happens in them, you can set the standard `CC`, `FC`, `CFLAGS`, `FFLAGS`,
and `CPPFLAGS` variables. In `wkt.mk`, `CPPFLAGS` is pre-populated with
some useful directories:

```makefile
CPPFLAGS += $(LUA_INCLUDE) -I$(irep_dir) -I.
```

This ensures that files implicitly compiled by `wkt.mk` can find Lua and
irep headers, as well as headers in the build directory.

### Link details

When constructing a link line for IREP programs, you'll want to add some
of the components in the examples to your build. `-L$(irep_dir) -L.`
ensures that we can find `libIR.a` and the built WKT libraries. This line:

```
-Wl,--start-group -lprog-wkt -lIR -lprog-wkt-index -Wl,--end-group
```

links in the sturctures in our WKT library (`-lprog-wkt`), the IREP
library itself (`-lIR`), and the WKT index library (`-lprog-wkt-index`).
`ir_read()` in `libIR` needs some symbols from the index to function
properly, but the index contains references to the structures from WKTs,
so we have to put all this inside of `-Wl,--start-group` and
`-Wl,--end-group` to ensure that the circular references are resolved.

The last line of the link line ensures that Lua libraries and Fortran
runtime libraries are included. Which of `-lm -lgfortran -ldl` you
include will depend on your particular application and the linker you
use.

### Lua handling

There is not a great mechanism for finding Lua from makefiles. These can
be set on the command line to override what is in `wkt.mk` -- by default
it uses a very simple autodetection mechanism that assumes that Lua
includes and libraries are in `include` and `lib` alongside the `lua`
executable. If your system's Lua installation uses different directories
(e.g. `$prefix/include/lua5.1`), you should set `LUA_INCLUDE` and
`LUA_LIBRARIES` to point it to the right place.

## `CMake` builds (here, C++)

The example in the `cxx-cmake` subdirectory is `CMake`-based.


The WKT library builds in these makefiles follow the
[IREP build documentation](https://irep.readthedocs.io/en/latest/build.html#cmake)
closely. Specifically, they do this to build a WKT library and a WKT
index library:

```
find_package(irep REQUIRED)
add_wkt_library(prog-wkt wkt_table1.h wkt_table4.h)
add_wkt_index_library(prog-wkt-index wkt_table1.h wkt_table4.h)
```

### Compile and link details

The first thing here is to call `find_package(irep)`.
`find_package` will find IREP if you add the irep package to
`CMAKE_PREFIX_PATH`, e.g. `-DCMAKE_PREFIX_PATH=/path/to/irep`.
It's marked `REQUIRED` because the build depends on it, we
want it to find this IREP in the test environment.

The WKT library creation is simple -- it just lists the WKTs in the
project. If you had more WKT libraries, you would need to ensure that all
of their WKT headers were listed in `add_wkt_index_library()`.

Program creation is somewhat simpler than with `gmake`:

```cmake
add_executable(cxx_prog cxx_main.cpp)
target_link_libraries(
  cxx_prog
  prog-wkt
  ${IREP_LIBRARIES}
  prog-wkt-index
  prog-wkt
  ${LUA_LIBRARIES}
)
```

Aside from adding `cxx_prog` as an executable, which is straightforward, this
also adds IREP and lua libraries to the exe. We didn't have to do this in
`gmake` because it was implicit in the C build rule, but CMake is more explicit
than `gmake` in this regard.

The libraries linked with `cxx_prog` are essentially the same as those
from `gmake`. However, `CMake` has no portable
`--start-group`/`--end-group` handling, so we just list `prog-wkt` twice
to handle the circular references.

### Lua handling

`LUA_LIBRARIES` needs to be on the link line, and it is set by this line
in the example:

```cmake
find_package(Lua REQUIRED)
```

CMake's
[FindLua](https://cmake.org/cmake/help/latest/module/FindLua.html) module
is pretty good about detecting all the nuances of Lua installations, so
users shoudl use `find_package(Lua)` like we do here, instead of relying
on the IREP build to do it.  Likewise, we set:

```cmake
include_directories("${LUA_INCLUDE_DIR}")
```

directly in the CMake build rather than relying on the IREP build to do
it.
