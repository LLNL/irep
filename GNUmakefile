# To use this makefile, lua needs to be in your PATH.
#
# You may need to set some standard GNU Make variables:
#
#   CC:       Full path to C compiler.
#   FC:       Full path to Fortran compiler.
#   CFLAGS:   Compiler flags for C.
#   FFLAGS:   Compiler flags for Fortran.
#   CPPFLAGS: Flags for the C preprocessor.
#
# It's less likely that you'll need to update these:
#
#   AR: implicitly provided by GNU Make
#   RANLIB: ranlib program (optional)

CC = gcc
FC = gfortran
CFLAGS = -O2 -g
FFLAGS = -O2 -g -cpp -ffree-form -ffree-line-length-0 -fbackslash

RANLIB = $(shell command -v ranlib 2> /dev/null || true)

# basic C and Fortran compilation rules come from wkt.mk
include wkt.mk

# only target here is the IREP library itself
all: libIR.a

# Save the intermediate Fortran source files, for debugging.
.SECONDARY: ir_std.f

libIR.a: ir_extern.o irep.o ir_std.o
	$(AR) -rc $@ $^
	$(RANLIB) $@

ir_generate.h: ir_macros.h ir_std.h
	CPPFLAGS=-I. $(irep_generate) $(sort $^) > $@

irep.o: irep.c ir_generate.h

ir_std.f: ir_std.h
	$(irep_generate) --mode fortran ir_std.h > $@

ir_extern.f: ir_extern.h
	$(irep_generate) --mode fortran ir_extern.h > $@
ir_extern.o ir_extern.mod: ir_std.mod

.PHONY: clean
clean: ; rm -f *.[afo] *.mod ir_generate.h
