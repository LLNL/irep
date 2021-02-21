# To use this makefile, lua needs to be in your PATH.
#

# basic C and Fortran compilation rules come from wkt.mk
include wkt.mk

# only target here is the IREP library itself
all: libIR.a

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
clean: ; rm -f *.[afo] *.mod
