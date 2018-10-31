# Copyright (c) 2016, Lawrence Livermore National Security, LLC. Produced
# at the Lawrence Livermore National Laboratory.  Written by Lee Busby,
# busby1@llnl.gov. LLNL-CODE-702338. All rights reserved.
# See ../../Copyright for additional notices.

# Required: Location of the IREP source.
ir.dir = ../..

# Required: List the well-known tables to include in libIR.a.
ir.wkt := $(wildcard wkt_*.h)

include $(ir.dir)/irep.mk

# Required: Locate Lua headers and libraries:
lua = $(HOME)/$(SYS_TYPE)/lua515

Lua_include = $(lua)/include
Lua_lib = $(lua)/lib

# Gnu compilers (gcc, g++, gfortran) require version 4.7.4 or greater.

.DEFAULT_GOAL := test

obj = f_main.o
prog = f_prog
compile = $(FC)
input = input.lua

$(obj): libIR.a

test: $(prog)
	./$(prog) $(input)

$(prog): $(obj)
	$(compile) -o $@ $(obj) -L$(Lua_lib) -L. -lIR -llua -lm -lgfortran -ldl

.PHONY: clean
clean: ir_clean; rm -f $(prog) $(obj) *.core
