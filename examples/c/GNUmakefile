# Copyright 2016-2021 Lawrence Livermore National Security, LLC and other
# IREP Project Developers. See the top-level LICENSE file for details.
#
# SPDX-License-Identifier: MIT

include ../../wkt.mk

# this build requires lua
lua := $(shell command -v lua 2> /dev/null)
lua_root = $(dir $(dir $(lua)))
lua_include = $(lua_root)/include
lua_lib = $(lua_root)/lib

# Gnu compilers (gcc, g++, gfortran) require version 4.7.4 or greater.

.DEFAULT_GOAL := test

obj = c_main.o
prog = c_prog
compile = $(CC)
input = input.lua

wkt_table4.mod: wkt_table1.mod

wkt.lib = libprog-wkt.a libprog-wkt-index.a
prog.wkt_src = $(wildcard wkt_*.h)
prog.wkt_index_src = $(prog.wkt_src)
CPPFLAGS += -I. -I$(irep_dir)
export CPPFLAGS

test: $(prog)
	./$(prog) $(input)

$(prog): $(obj) $(wkt.lib)
	$(compile) -o $@ $(obj) \
		-I. \
		-L$(lua_lib) -L$(irep_dir) -L. \
		-Wl,--start-group -lprog-wkt -lIR -lprog-wkt-index -Wl,--end-group \
		-llua -lm -lgfortran -ldl

.PHONY: clean
clean:
	rm -f $(prog) $(obj) $(wkt.lib) *.mod *.o
