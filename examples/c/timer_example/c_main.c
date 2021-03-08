// Copyright 2016-2021 Lawrence Livermore National Security, LLC and other
// IREP Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

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
#include <time.h>
#include <sys/time.h>

#if defined(__cplusplus)
extern "C" {
using namespace irep;
#endif

#if 0
static double msec_clock(void) {
  struct timespec ts;
  (void) clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
  return (double)(1000.0*ts.tv_sec + ts.tv_nsec/1000000.0);
}

int eval_d4(lua_State *L, lua_cb_dd_data *d, double *x, double *v) {
  int i, tv;

  if (d->fref == LUA_REFNIL) return 1;
  if (d->fref == LUA_NOREF) {
    for (i=0; i < d->nret; ++i) v[i] = d->const_val[i];
    return 0;
  }

  lua_rawgeti(L, LUA_REGISTRYINDEX, d->fref);
  tv = lua_type(L,-1);

  if (tv == LUA_TFUNCTION) {
    for (i=0; i < d->nprm; i++) lua_pushnumber(L, x[i]); // Push args.
    if (lua_pcall(L, d->nprm, d->nret, 0) != 0)
      return luaL_error(L, "error: %s", lua_tostring(L,-1));
    for (i=0; i < d->nret; i++) {
      if (lua_type(L, i-d->nret) != LUA_TNUMBER)
        return luaL_error(L, "error: expected number as return value: %d", i+1);
      v[i] = lua_tonumber(L, i - d->nret);
    }
    lua_pop(L, d->nret); // Pop return values.

  } else if (tv == LUA_TTABLE) {
    int len = lua_objlen(L,-1);
    for (i=1; i<=len; i++) {
      lua_rawgeti(L,-1,i);
      if (lua_type(L,-1) != LUA_TNUMBER) return 1;
      v[i-1] = lua_tonumber(L,-1);
      lua_pop(L,1);
    }
    lua_pop(L,1);

  } else {
    fprintf(stderr,"error, wrong type\n");
    return 1;
  }

  return 0;
}
#endif

int eval_dd(lua_State *L, lua_cb_dd_data *d, double *x, double *v) {
  int i;

  if (d->fref == LUA_REFNIL) return 1;
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
  int ios, i, n, j, ii;
  double x[3] = { 2.0, 3.0, 4.0 }, v=0.0, v2[2];

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

  char *ss = strdup(IR_STR(table1.s));
  printf("table1.s = \"%s\"\n", ss);

#if 0
  start = msec_clock();
  for (n=0;n<1000000;n++) {
    i = eval_dd(L, &table1.f1, x, &v);
    vv += v;
  }
  end = msec_clock();
  printf("Call f1 1000000 times: %g (%g)\n", end-start, vv);

  start = msec_clock();
  for (n=0;n<1000000;n++) {
    i = eval_d4(L, &table1.f4, x, vvv);
    vv += vvv[0] + vvv[1] + vvv[2];
  }
  end = msec_clock();
  printf("Call f4 1000000 times: %g (%g)\n", end-start, vv);

  vv = 0.0;
  start = msec_clock();
  for (n=0;n<1000000;n++) {
    i = eval_dd(L, &table1.table3.f3, x, &v);
    vv += v;
  }
  end = msec_clock();
  printf("Call f3 1000000 times: %g (%g)\n", end-start, vv);
#endif

  i = eval_dd(L, &table1.f1, x, &v);
  if (!i) printf("table1.f1(%g,%g,%g) = %g\n", x[0],x[1],x[2], v);
  else printf("f1 undefined\n");

  i = eval_dd(L, &table1.table2[1].f2, x, &v);
  if (!i) printf("table1.table2[1].f2(%g,%g,%g) = %g\n", x[0],x[1],x[2], v);
  else printf("f2 undefined\n");

  i = eval_dd(L, &table1.table3.f3, x, v2);
  if (!i) printf("table1.table3.f3(%g,%g,%g) = %g %g\n", x[0],x[1],x[2], v2[0],v2[1]);
  else printf("f3 undefined\n");

  n = sizeof(table1.e)/sizeof(*table1.e);
  printf("\ntable1.e: %d elements, %d given\n", n, ir_rtlen(L, "table1.e"));
  for (i=0;i<n;i++)
    printf("table1.e[%d] = %g\n", i, table1.e[i]);

  ios = ir_read(L, "table4");
  printf("\nREAD TABLE4: ios=%d\n",ios);
  for (i=1; i<=3; i++) {
    char *ss = strdup(IR_STR(table4[i].name));
    printf("table4[%d].name = \"%s\" (len=%d)\n", i, ss, (int)strlen(ss));
    lua_rawgeti(L, LUA_REGISTRYINDEX, table4[i].fooref);
    printf("Type of table4[%d].fooref: %s\n", i,lua_typename(L,lua_type(L,-1)));
    if (lua_type(L,-1) == LUA_TTABLE) {
      ii = lua_objlen(L,-1);
      for (j=1; j<=ii; j++) {
        lua_rawgeti(L,-1,j);
        printf("fooref[%d] = %g\n", j,lua_tonumber(L,-1));
        lua_pop(L,1);
      }
    }
    lua_pop(L,1);
  }

  return 0;
}

#if defined(__cplusplus)
}
#endif
