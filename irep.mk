# Copyright (c) 2016, Lawrence Livermore National Security, LLC. Produced
# at the Lawrence Livermore National Laboratory.  Written by Lee Busby,
# busby1@llnl.gov. LLNL-CODE-702338. All rights reserved.
# See ./Copyright for additional notices.

# makefile for irep.

SHELL = /bin/bash

ch_null  :=
ch_space := $(ch_null) #
ch_comma := ,
ch_semi  := ;
ccE = gcc -E

blue_xl.fflg = -qsuppress=cmpmsg -qsuppress=1500-036 -qxlf2003=polymorphic -qnosave -qfree=f90 -qsuffix=cpp=f:f=f -qmaxmem=-1

# Override these variables in the client makefile as needed.
CC = gcc
FC = gfortran
CXX = g++
CFLG = -Wall -O2
CXXFLG = -Wall -O2
FFLG = -O2 -cpp -ffree-form -ffree-line-length-0 -fbackslash
#FFLG = -O2 $(blue_xl.fflg)
fppflags_extra = 
Lua_include = $(HOME)/$(SYS_TYPE)/lua515/include
FPPFLAGS = -DIREP_LANG_FORTRAN -I$(Lua_include) -I$(ir.dir) $(fppflags_extra)
COMPILE.c = $(CC) -c $(CFLG) -DIREP_LANG_C -I$(Lua_include) -I$(ir.dir) -I.
COMPILE.f = $(FC) -c $(FFLG) -DIREP_LANG_FORTRAN -I$(Lua_include) -I$(ir.dir) -I.
COMPILE.cxx = $(CXX) -c $(CXXFLG) -DIREP_LANG_C -I$(Lua_include) -I$(ir.dir) -I.

# Not too much should need to change below this point.
ir.wobj := $(subst .h,.o,$(notdir $(ir.wkt)))
ir.wmod := $(subst .h,.mod,$(notdir $(ir.wkt)))
ir.wfs  := $(subst .h,.f,$(notdir $(ir.wkt)))
ir.gen  := $(ir.wkt)

vpath %.c $(ir.dir)
vpath %.h $(ir.dir)

%.o: %.cpp
	@echo -e "$(COMPILE.cxx) $($*_flags.cxx) $<";\
	$(COMPILE.cxx) $($*_flags.cxx) $<

%.o: %.c
	@echo -e "$(COMPILE.c) $($*_flags.c) $<";\
	$(COMPILE.c) $($*_flags.c) $<

%.o %.mod : %.f
	@if [[ ! -e $@ || $*.o -ot $(word 1,$?) ]]; then\
	  echo -e "$(COMPILE.f) $($*_flags.f) $<";\
	  $(COMPILE.f) $($*_flags.f) $<;\
	else true; fi

# Save the intermediate Fortran source files, for debugging.
.SECONDARY: $(ir.wfs) ir_std.f

Gflg := -DIREP_GENERATE -I$(Lua_include) -I$(ir.dir) $(fppflags_extra)
ir_generate.h: ir_macros.h ir_std.h $(ir.gen)
	cat $(sort $^) | $(ir.dir)/etc/ir_generate.lua $(Gflg) $(sort $^) > $@

irep.o : irep.c ir_generate.h

$(ir.wfs) : ir_std.mod
wkt_%.f : wkt_%.h ir_macros.h ir_start.h ir_end.h
	$(ccE) -DIR_WKT_NAME=wkt_$* $(FPPFLAGS) $< | \
	  sed -e 's:+/+/://:' -e 's/ *\#\# *//' -e '/^\#/d' -e '/^ *$$/d' > $@

ir_std.f : ir_std.h
	$(ccE) -DIR_WKT_NAME=ir_std $(FPPFLAGS) $< | \
	  sed -e 's:+/+/://:' -e 's/ *\#\# *//' -e '/^\#/d' -e '/^ *$$/d' > $@

ir_extern.o ir_extern.mod : ir_std.mod $(ir.wmod)
ir.wuse = $(addprefix use ,$(addsuffix $(ch_semi),$(basename $(ir.wobj))))
ir.wnml = $(subst $(ch_space),$(ch_comma),$(strip \
	  $(subst wkt_,,$(basename $(ir.wobj)))))
ir_extern.f : ir_extern.h
	$(ccE) $(FPPFLAGS) $< | \
	  sed -e 's:+/+/://:' -e 's/ *\#\# *//' -e '/^\#/d' -e '/^ *$$/d' > $@

# Construct the IREP library.
libIR.a : ir_extern.o irep.o ir_std.o $(ir.wobj)
	ar rcv $@ $^

.PHONY: ir_clean
ir_clean:
	rm -f *.o *.mod wkt_*.f ir_std.f ir_extern.f libIR.a ir_generate.h
