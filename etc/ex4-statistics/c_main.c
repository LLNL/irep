// Copyright (c) 2016, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory.  Written by Lee Busby,
// busby1@llnl.gov. LLNL-CODE-702338. All rights reserved.
// See ../../Copyright for additional notices.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__cplusplus)
#include "lua.hpp"
#else
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#endif
#include "ir_extern.h"
#include "wkt_table1.h"
#include "wkt_table4.h"
#include "wkt_statistics.h"
#include <time.h>
#include <sys/time.h>

#if defined(__cplusplus)
extern "C" {
using namespace irep;
#endif

int main(int argc, char *argv[]) {
  lua_State *L = luaL_newstate();
  int ios;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s *.lua\n", argv[0]);
    return 0;
  }
  double start = ir_clock();
  luaL_openlibs(L);
  if (luaL_loadfile(L, argv[1]) || lua_pcall(L, 0, 0, 0))
    return luaL_error(L,
      "cannot run configuration file: %s", lua_tostring(L,-1));
  ir_add_time(1, ir_clock() - start, &statistics.item1);

  ios = ir_unread(L, "statistics");
  printf("ios 1: %d\n", ios);
  ios = luaL_loadstring(L, "print_table(\"statistics\")");
  ios = lua_pcall(L, 0,0,0);
  printf("ios 3: %d\n", ios);

  return 0;
}

#if defined(__cplusplus)
}
#endif
