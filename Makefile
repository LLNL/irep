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
FFLAGS = -O2 -g -ffree-form
RANLIB = $(shell command -v ranlib 2> /dev/null || true)

src.all := $(wildcard *.[ch] blast/*.[ch])
src.c   := $(filter %.c,$(src.all))
src.h   := $(filter %.h,$(src.all))

vpath %.c $(sort $(dir $(src.c)))
vpath %.h $(sort $(dir $(src.h)))

COMPILE.c = $(CC) $(CFLAGS) $(CPPFLAGS) -DIREP_LANG_C       -I.
COMPILE.f = $(FC) $(FFLAGS) $(CPPFLAGS) -DIREP_LANG_FORTRAN -I.
%.o : %.c       ; $(COMPILE.c) -c $<
%.o %.mod : %.f ; $(COMPILE.f) -c $<

# Save the intermediate Fortran source files, for debugging.
.SECONDARY: ir_std.f

# ir-prep turns *.h files into *.f files, with the C preprocessor and
# a little help from sed.
ir-prep = $(CC) -E - -DIR_WKT_NAME=$1 $(CPPFLAGS) -DIREP_LANG_FORTRAN < $2 \
	| sed -e 's:+/+/://:' -e 's/ *\#\# *//' -e '/^\#/d' -e '/^ *$$/d' > $3

libIR.a: ir_extern.o irep.o ir_std.o
	$(AR) -rc $@ $^
	$(RANLIB) $@

ir_generate.h: ir_macros.h ir_std.h $(ir.gen)
	cat $(sort $^) \
	| ./etc/ir_generate.lua -DIREP_GENERATE $(CPPFLAGS) $(sort $^) > $@

irep.o: irep.c ir_generate.h

ir_std.f: ir_std.h
	$(call ir-prep,ir_std,$<,$@)

ir_extern.o ir_extern.mod: ir_std.mod
ir_extern.f: ir_extern.h
	$(call ir-prep,unused,$<,$@)

.PHONY: clean
clean: ; rm -f *.[afo] *.mod
