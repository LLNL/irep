# Copyright 2016-2021 Lawrence Livermore National Security, LLC and other
# IREP Project Developers. See the top-level LICENSE file for details.
#
# SPDX-License-Identifier: MIT

# To use this makefile, lua needs to be in your PATH.
#

# basic C and Fortran compilation rules come from wkt.mk
include wkt.mk

# only target here is the IREP library itself
all: libIR.a

test: libIR.a
	@echo -e $(cgreen)Testing IREP C Executable with GNU make$(cend)
	make -C examples/c test
	@echo -e $(cgreen)Testing IREP C++ Executable with CMake$(cend)
	make -C examples/cxx-cmake test
	@echo -e $(cgreen)Testing IREP Fortran Executable with GNU make$(cend)
	make -C examples/fortran test

# Save the intermediate Fortran source files, for debugging.
.SECONDARY: ir_std.f

libIR.a: ir_extern.o irep.o ir_std.o
	$(AR) -rc $@ $^
	$(RANLIB) $@

ir_std.f: ir_std.h
	$(irep_generate) --mode fortran ir_std.h > $@

ir_extern.f: ir_extern.h
	$(irep_generate) --mode fortran ir_extern.h > $@
ir_extern.o ir_extern.mod: ir_std.mod

.PHONY: clean
clean:
	rm -f *.[afo] *.mod
	make -C examples/c clean
	make -C examples/cxx-cmake clean
	make -C examples/fortran clean
