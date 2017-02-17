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

# Gnu compilers (gcc, g++, gfortran) require version 4.7.4 or greater.
gnu.cc = gcc
gnu.fc = gfortran
gnu.cflg =
gnu.fflg = -cpp -fbackslash -ffree-form -ffree-line-length-0
gnu.lflags = -lm -lgfortran -ldl
gnu.link = $(gnu.cc)

# Intel-16 is known to work.
intel.cc = icc
intel.fc = ifort
intel.cflg =
intel.fflg = -fpp -assume bscc -free
intel.lflags = -limf -lirc -lifcore
intel.link = $(intel.cc)

# Pgi-16 is known to work.
pgi.cc = pgcc -pgf90libs
pgi.fc = pgfortran
pgi.cflg =
pgi.fflg = -Mfree -Mrecursive -Mpreprocess -Mnobackslash
pgi.lflags = -ldl
pgi.link = $(pgi.cc)

ibm.cc = bgxlc_r
ibm.fc = bgxlf_r -qfree=f90 -qsuffix=cpp=f:f=f -qlanglvl=extended
ibm.cflg =
ibm.fflg =
#ibm.lflags = -L/opt/ibmcmp/xlf/bg/14.1/lib -lxlf90 -lm -lc
ibm.lflags =
ibm.link = $(ibm.fc)

CC = $($(compiler).cc)
FC = $($(compiler).fc)
CFLG = $($(compiler).cflg)
FFLG = $($(compiler).fflg)
LINK = $($(compiler).link)

fppflags_extra = 
Lua_include = $(HOME)/$(SYS_TYPE)/lua515/include
FPPFLAGS = -DIREP_LANG_FORTRAN -I$(Lua_include) -I$(ir.dir) $(fppflags_extra)
COMPILE.c = $(CC) -c $(CFLG) -DIREP_LANG_C -I$(Lua_include) -I$(ir.dir) -I.
COMPILE.f = $(FC) -c $(FFLG) -DIREP_LANG_FORTRAN -I$(Lua_include) -I$(ir.dir) -I.

# Not too much should need to change below this point.
ir.wobj = $(subst .h,.o,$(notdir $(ir.wkt)))
ir.wmod = $(subst .h,.mod,$(notdir $(ir.wkt)))
ir.wfs  = $(subst .h,.f,$(notdir $(ir.wkt)))

vpath %.c $(ir.dir)
vpath %.h $(ir.dir)

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

irep.o : irep.c ir_special.h

$(ir.wfs) : ir_std.mod
wkt_%.f : wkt_%.h ir_macros.h ir_start.h ir_end.h
	$(ccE) -DIR_WKT_NAME=wkt_$* $(FPPFLAGS) $< | \
	  sed -e 's:+/+/://:' -e 's/ *## *//' -e '/^#/d' -e '/^ *$$/d' > $@

ir_std.f : ir_std.h
	$(ccE) -DIR_WKT_NAME=ir_std $(FPPFLAGS) $< | \
	  sed -e 's:+/+/://:' -e 's/ *## *//' -e '/^#/d' -e '/^ *$$/d' > $@

ir_readnml.o ir_readnml.mod : ir_std.mod $(ir.wmod)
ir.wuse = $(addprefix use ,$(addsuffix $(ch_semi),$(basename $(ir.wobj))))
ir.wnml = $(subst $(ch_space),$(ch_comma),$(strip \
	  $(subst wkt_,,$(basename $(ir.wobj)))))
ir_readnml.f : ir_readnml.h
	$(ccE) -DUSE_WKT='$(ir.wuse)' -DWKT_NML='$(ir.wnml)' $(FPPFLAGS) $< | \
	  sed -e 's:+/+/://:' -e 's/ *## *//' -e '/^#/d' -e '/^ *$$/d' > $@

# Construct the IREP library.
libIR.a : ir_readnml.o irep.o ir_std.o $(ir.wobj)
	ar rcv $@ $^

.PHONY: ir_clean
ir_clean:
	rm -f *.o *.mod wkt_*.f ir_std.f ir_readnml.f libIR.a
