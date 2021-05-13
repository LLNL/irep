# Copyright 2016-2021 Lawrence Livermore National Security, LLC and other
# IREP Project Developers. See the top-level LICENSE file for details.
#
# SPDX-License-Identifier: MIT

include ../irep/share/irep/wkt.mk

# Gnu compilers (gcc, g++, gfortran) require version 4.7.4 or greater.

.DEFAULT_GOAL := test

obj = f_main.o
prog = f_prog
input = input.lua

f_main.o: wkt_table4.mod wkt_table1.mod

wkt.lib = libprog-wkt.a libprog-wkt-index.a
prog.wkt_src = $(wildcard wkt_*.h)
prog.wkt_index_src = $(prog.wkt_src)

test: $(prog)
	./$(prog) $(input)

$(prog): $(obj) $(wkt.lib)
	$(COMPILE.f) -o $@ $(obj) \
		-L$(irep_dir)/lib -L. \
		-Wl,--start-group -lprog-wkt -lIR -lprog-wkt-index -Wl,--end-group \
		$(LUA_LIBRARIES) -lm -lgfortran -ldl

.PHONY: clean
clean:
	rm -f $(prog) $(obj) $(wkt.lib) *.mod *.o
