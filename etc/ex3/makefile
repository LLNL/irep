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
lua = ../lua515

Lua_include = $(lua)/include
Lua_lib = $(lua)/lib

# Gnu compilers (gcc, g++, gfortran) require version 4.7.4 or greater.

.DEFAULT_GOAL := test

obj = c_main.o
prog = c_prog
compile = $(CC)
input = input.lua

$(obj): libIR.a
ir_extern.f: ../lua515/include/lua.h

test: $(prog)
	./$(prog) $(input)

$(prog): $(obj)
	$(compile) -o $@ $(obj) -L$(Lua_lib) -L. -lIR -llua -lm -lgfortran -ldl

.PHONY: clean distclean
clean: ir_clean; rm -f $(prog) $(obj)
distclean: clean; rm -rf ../lua515

lua_src = https://www.lua.org/ftp/lua-5.1.5.tar.gz
../lua515/lua-5.1.5.tgz:
	mkdir -p ../lua515
	cd ../lua515 && curl -s $(lua_src) -o $(notdir $@)

../lua515/include/lua.h: ../lua515/lua-5.1.5.tgz
	cd ../lua515 && tar xfz lua-5.1.5.tgz && cd lua-5.1.5 && \
	$(MAKE) posix install CC=$(CC) \
	CFLAGS="$(CPPFLAGS) $(CFLAGS) -DLUA_USE_POSIX" INSTALL_TOP=../..
	touch -r $@ $<
