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

#if defined(__cplusplus)
extern "C" {
using namespace irep;
#endif

// A simple evaluator for IREP callback functions.
int eval_dd(lua_State *L, lua_cb_dd_data *d, double *x, double *v) {
  int i;
  if (d->fref == LUA_NOREF) {
    for (i=0; i < d->nret; ++i) v[i] = d->const_val[i];
    return 0;
  }

  // Push the function onto the stack.
  lua_rawgeti(L, LUA_REGISTRYINDEX, d->fref);
  for (i=0; i < d->nprm; i++) lua_pushnumber(L, x[i]); // Push args.

  if (lua_pcall(L, d->nprm, d->nret, 0) != 0)
    return luaL_error(L, "error: %s", lua_tostring(L,-1));

  for (i=0; i < d->nret; i++) {
    if (lua_type(L, i-d->nret) != LUA_TNUMBER)
      return luaL_error(L, "error: expected number for return value: %d", i+1);
    v[i] = lua_tonumber(L, i - d->nret);
  }
  lua_pop(L, d->nret); // Pop return values.
  return 0;
}

int main(int argc, char *argv[]) {
  lua_State *L = luaL_newstate();
  int ios, i, n;
  double x[3] = { 2.0, 3.0, 4.0 }, v;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s *.lua\n", argv[0]);
    return 0;
  }
  luaL_openlibs(L);
  if (luaL_loadfile(L, argv[1]) || lua_pcall(L, 0, 0, 0))
    return luaL_error(L,
      "cannot run configuration file: %s", lua_tostring(L,-1));

  ios = ir_read(L, "table1");
  printf("\nREAD TABLE1: ios=%d\n",ios);
  printf("table1.i = %d\n", table1.i);
  printf("table1.d = %g\n", table1.d);
  printf("table1.b = %d\n", table1.b);

  char *ss = strndup(IR_STR(table1.s));
  printf("table1.s = \"%s\"\n", ss);

  (void) eval_dd(L, &table1.f1, x, &v);
  printf("table1.f1(%g,%g,%g) = %g\n", x[0],x[1],x[2], v);

  (void) eval_dd(L, &table1.table2[1].f2, x, &v);
  printf("table1.table2[1].f2(%g,%g,%g) = %g\n", x[0],x[1],x[2], v);

  (void) eval_dd(L, &table1.table3.f3, x, &v);
  printf("table1.table3.f3(%g,%g,%g) = %g\n", x[0],x[1],x[2], v);

  n = sizeof(table1.e)/sizeof(*table1.e);
  printf("\ntable1.e: %d elements, %d given\n", n, ir_rtlen(L, "table1.e"));
  for (i=0;i<n;i++)
    printf("table1.e[%d] = %g\n", i, table1.e[i]);

  ios = ir_read(L, "table4");
  printf("\nREAD TABLE4: ios=%d\n",ios);
  for (i=1; i<=3; i++) {
    char *ss = strndup(IR_STR(table4[i].name));
    printf("table4[%d].name = \"%s\"\n", i, ss);
  }

  return 0;
}

#if defined(__cplusplus)
}
#endif
