// Copyright 2016-2021 Lawrence Livermore National Security, LLC and other
// IREP Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "lua.hpp"
#include "ir_std.h" // lua_cb_data
#include "ir_extern.h"
#include "wkt_table1.h"
#include "wkt_table4.h"

#if 0
static void itemdump(lua_State *L, int i, char c) {
  int t = lua_type(L, i);
  switch (t) {
    case LUA_TSTRING: {
      printf("%d: (string): %s%c", i, lua_tostring(L,i), c);
      break;
    }
    case LUA_TBOOLEAN: {
      printf("%d: (bool): %s%c",
             i, lua_toboolean(L, i) ? "true":"false", c);
      break;
    }
    case LUA_TNUMBER: {
      printf("%d: (number): %g%c", i, lua_tonumber(L, i), c);
      break;
    }
    default: {
      printf("%d: (%s): %p%c",
             i, lua_typename(L, t), lua_topointer(L,i), c);
      break;
    }
  }
}
static void stackdump(lua_State *L) {
  int i, top = lua_gettop(L);
  for (i=1; i<= top; i++) itemdump(L, i, '|');
  printf("\n");
}
#endif

class LuaCallback {
public:
  LuaCallback(lua_State *L, lua_cb_data &d);
  ~LuaCallback(){}
  virtual int Evaluate(std::vector<double> &v, const std::vector<double> &x);
  virtual double Evaluate(const std::vector<double> &x);

private:
  lua_State *L;
  lua_cb_data &d;
  int nprm;
  int nret;
};

// Constructor.
LuaCallback::LuaCallback(lua_State *L, lua_cb_data &d) : L(L), d(d) {
  nprm = ir_nprm(d.npnr);
  nret = ir_nret(d.npnr);
}

// Evaluator for scalar returns.
double LuaCallback::Evaluate(const std::vector<double> &x) {
  std::vector<double> v(1);
  (void)LuaCallback::Evaluate(v, x);
  return v[0];
}

// Evaluator for vector returns.
int LuaCallback::Evaluate(std::vector<double> &v, const std::vector<double> &x)
{
  int actual_nret = nret;

  if (d.fref == LUA_NOREF) {
    // In the LUA_NOREF case, nret always has the actual length of d.data.
    // (Even if nret was originally specified as -1 in the wkt file.)
    if (nret > (int)v.size()) {
      fprintf(stderr,"ERROR: v is too small\n"); exit(1);
    }
    double *data = (double *)d.data;
    for (int i=0; i<nret; ++i) v[i] = data[i];

  } else {
    if (nprm > (int)x.size()) {
      fprintf(stderr,"ERROR: not enough parameters given\n"); exit(1);
    }
    lua_rawgeti(L, LUA_REGISTRYINDEX, d.fref);
    for (int i=0; i<nprm; i++) { lua_pushnumber(L, x[i]); }
    if (nret == -1) actual_nret = 1; // One return value, a table.
    if (lua_pcall(L, nprm, actual_nret, 0) != 0) {
      fprintf(stderr,"ERROR: calling function\n"); exit(1);
    }

    if (nret == -1) { // Arbitrary length table expected.
      if (lua_type(L,-1) != LUA_TTABLE) {
        fprintf(stderr,"ERROR: Expected function to return table\n"); exit(1);
      }
      actual_nret = lua_objlen(L,-1);
      if (actual_nret > (int)v.size()) {
        fprintf(stderr,"ERROR: v is too small for table returned\n"); exit(1);
      }
      for (int i=1; i<=actual_nret; i++) {
        lua_rawgeti(L,-1,i);
        if (lua_type(L,-1) != LUA_TNUMBER) {
          fprintf(stderr,"ERROR: table item is not number: %d\n", i); exit(1);
        }
        v[i-1] = lua_tonumber(L,-1);
        lua_pop(L,1);
      }
      lua_pop(L,1);

    } else { // Standard return on the stack.
      for (int i=0; i<nret; i++) {
        if (lua_type(L,i-nret) != LUA_TNUMBER) {
          fprintf(stderr,"ERROR: not number: %d\n", i); exit(1);
        }
        v[i] = lua_tonumber(L, i - nret);
      }
      lua_pop(L, nret);
    }
  }
  return actual_nret;
}

// ----------------------------------------------------------------------
int main(int argc, char *argv[]) {
  lua_State *L = luaL_newstate();
  int i, ios;
  double dd;
  static const double arr[] = { 1.,2.,3. };
  std::vector<double> x (arr, arr+sizeof(arr)/sizeof(arr[0]));
  std::vector<double> v(3);

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
  printf("table1.i = %d\n", irep::table1.i);
  printf("table1.d = %g\n", irep::table1.d);
  printf("table1.b = %d\n", irep::table1.b);

  LuaCallback *f1 = new LuaCallback(L, irep::table1.f1);
  dd = f1->Evaluate(x);
  printf("return from f1: %g\n", dd);

  LuaCallback *f4 = new LuaCallback(L, irep::table1.f4);
  i = f4->Evaluate(v, x);
  printf("return from f4: %d, %g\n", i,v[0]);

  LuaCallback *f5 = new LuaCallback(L, irep::table1.f5);
  i = f5->Evaluate(v, x);
  printf("return from f5: %d, %g %g %g\n", i,v[0],v[1],v[2]);
  printf("Full name for f5: %s\n", ir_get_function_name(L,&irep::table1.f5));

  return 0;
}
