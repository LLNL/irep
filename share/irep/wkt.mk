# Copyright 2016-2021 Lawrence Livermore National Security, LLC and other
# IREP Project Developers. See the top-level LICENSE file for details.
#
# SPDX-License-Identifier: MIT

# wkt.mk should be included by makefiles that need to build IREP WKT
# libraries.
#
# To build a wkt library, e.g., `libfoo-wkt.a`, simply include this file,
# define a variable `foo.wkt_src`, and add the WKT library as a make target.
# For example:
#
#     include /path/to/wkt.mk
#     foo.wkt_src = wkt_shapes.h wkt_colors.h wkt_numbers.h
#     all: libfoo-wkt.a
#
# The wkt library will be generated and built from the provided WKT headers.
#
# You can need to set some standard GNU Make variables to control how
# WKT libs are generated:
#
#   CC:       Full path to C compiler.
#   FC:       Full path to Fortran compiler.
#   CFLAGS:   Compiler flags for C.
#   FFLAGS:   Compiler flags for Fortran.
#   CPPFLAGS: Flags for the C preprocessor.
#
# You can use these variables to inject system Lua configuration.
# You can use these in makefiles that import wkt.mk.
#
#   LUA_INCLUDE:   path to directory containing lua.h
#   LUA_LIBRARIES: path to directory containing liblua
#
# It's less likely that you'll need to update these:
#
#   AR: implicitly provided by GNU Make
#   RANLIB: ranlib program (optional)

CC = gcc
FC = gfortran
CFLAGS = -O2 -g
FFLAGS = -O2 -g -cpp -ffree-form -ffree-line-length-0 -fbackslash

# RANLIB can be optionally set
RANLIB = $(shell command -v ranlib 2> /dev/null || true)

# like $(dir), but remove trailing slash and handle /
dirname = $(if $(patsubst %/,%,$(dir $(1))),$(patsubst %/,%,$(dir $(1))),/)

# By default, assume lua is the one in PATH, and that its includes are in
# $lua_root/include and libs are in $lua_root/lib.  Lua varies quite a bit
# from system to system, so you may need to inject LUA_INCLUDE and
# LUA_LIBRARIES yourself.
LUA := $(shell command -v lua 2> /dev/null)
LUA_ROOT := $(call dirname,$(call dirname,$(LUA)))
LUA_INCLUDE := -I$(LUA_ROOT)/include
$(info $(LUA_INCLUDE))
LUA_LIBRARIES := -L$(LUA_ROOT)/lib -llua

# find location of this makefile so we can find irep tools
irep_dir := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))/../..)

# ensure generator can find lua and headers from irep
CPPFLAGS += $(LUA_INCLUDE) -I$(irep_dir)/include -I.
export CPPFLAGS

# utility program for generating code from wkt.h files
irep_generate = $(irep_dir)/bin/irep-generate

# Rules for compiling C and Forran files -- note that the appropriate
# IREP -D flag must be set for files that include IREP headers.
COMPILE.c = $(CC) $(CFLAGS) $(CPPFLAGS)
COMPILE.f = $(FC) $(FFLAGS) $(CPPFLAGS) -DIREP_LANG_FORTRAN
%.o: %.c       ; $(COMPILE.c) -c $<
%.o %.mod: %.f ; $(COMPILE.f) -c $<

# override gmake's default assumption that .mod is a modula file
# without this, you'll get warnings about cicular dependencies and
# gmake will try to run m2c on module files.
%.o: %.mod

# generate fortran from every wkt_%.h file
wkt_%.f: wkt_%.h
	$(irep_generate) --mode fortran $< > $@

# helper function for finding wkt files with absolute paths
containing = $(foreach v,$2,$(if $(findstring $1,$v),$v))

# These functions define the pieces of a wkt library.
# They're defined as functions so they can be used as prerequisites
# in our doubly expanded pattern rule for WKT libs below.
ir-wkt  = $(filter %.h,$(call containing,wkt_,$($(1))))
ir-wobj = $(subst .h,.o,$(foreach v,$(call ir-wkt,$(1)),$(notdir $v)))

# add a little flair to the output
cgreen = "\033[1;32m"
cend    = "\033[0m"

# Any target like libfoo-wkt.a will be built like a wkt library.
# The user needs only define foo.wkt_src to make the magic happen.
.SECONDEXPANSION:
lib%-wkt.a: $$(call ir-wobj,$$*.wkt_src)
	$(if $^,,$(error "No wkt_*.h files were provided for '$@'.  Did you set $*.wtk_src?"))
	$(AR) -rc $@ $^
	$(RANLIB) $@
	@echo -e Successfully created IREP WKT library $(cgreen)$@$(cend).
	$(info $^)

%-wkt-index.c: $$(call ir-wkt,$$*.wkt_index_src)
	$(if $^,,$(error "No wkt_*.h files were provided for '$@'.  Did you set $*.wtk_index_src?"))
	$(irep_generate) $(irep_dir)/ir_std.h $^ > $@

# The index library contains a set of tables that allow us to look up
# wkt structs by name.
lib%-wkt-index.a: $$*-wkt-index.o
	$(AR) -rc $@ $(filter %.o,$^)
	$(RANLIB) $@
	@echo -e Successfully created IREP WKT index library $(cgreen)$@$(cend).
