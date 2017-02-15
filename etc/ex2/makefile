# Copyright (c) 2016, Lawrence Livermore National Security, LLC. Produced
# at the Lawrence Livermore National Laboratory.  Written by Lee Busby,
# busby1@llnl.gov. LLNL-CODE-702338. All rights reserved.
# See ../../Copyright for additional notices.

# Required: Location of the IREP source.
ir.dir = ../..

# Required: List the well-known tables to include in libIR.a.
ir.wkt := $(wildcard wkt_*.h)

compiler = gnu


include $(ir.dir)/irep.mk

# Required: Locate Lua headers and libraries:
lua = $(HOME)/$(SYS_TYPE)/lua515

Lua_include = $(lua)/include
Lua_lib = $(lua)/lib

.DEFAULT_GOAL := test

obj = f_main.o
prog = fprog

$(obj): libIR.a

test: $(prog)
	./$(prog) input.lua

$(prog): $(obj)
	$(FC) -o $@ $(obj) -L$(Lua_lib) -L. -lIR -llua $($(compiler).lflags)

.PHONY: clean displayC displayF
clean: ir_clean; rm -f $(prog) $(obj)
displayC: ; gcc -E -DLANG_C       -I$(ir.dir) $(ir.wkt)
displayF: ; gcc -E -DLANG_FORTRAN -I$(ir.dir) $(ir.wkt)
