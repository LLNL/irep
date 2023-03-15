// Copyright 2016-2021 Lawrence Livermore National Security, LLC and other
// IREP Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

#if defined(__cplusplus)
extern "C" {
#endif

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "ir_index.h"
#include "ir_std.h"

// BSZ is the internal buffer size for strings typically containing the
// name of Lua table elements such as "table1.table2[123].foo.bar".
#define BSZ 2048

// Set irep_debug=1 in the environment, to see elements visited during ir_read.
static int irep_debug = -1;
static void Dbg_print(const char *fmt, ...)
{
  va_list argp;
  if (irep_debug > 0) {
    char buf[BSZ];
    const int i = sprintf(buf, "IR_DBG: ");
    va_start(argp, fmt);
    (void)vsnprintf(buf+i,sizeof(buf)-10,fmt,argp);
    va_end(argp);
    (void)fprintf(stderr,"%s\n",buf);
  }
}

// Note comma operator below, specifying the return value.
#define Ir_error(fmt,...) \
  fprintf(stderr,"ERROR (Lua/IR): " fmt "\n",__VA_ARGS__),1

// TYP_ERR is common enough to get its own macro.
#define TYP_ERR(lrep, ltyp, ityp) \
Ir_error("Type mismatch: %s (%s): Expected: %s",lrep,s_typ[ltyp],s_typ[ityp])

// Type specifiers (Typ), and their string equivalents (s_typ).

static const char *s_typ[] = { "integer", "double", "logical", "string",
  "callback", "table", "reference", "pointer", "new_callback" };

// Find index of "name" in element table tp.
static int find_element(const char *name, ir_element *tp) {
  int i;
  for (i=0; tp[i].name; i++)
    if (strcmp(name, tp[i].name) == 0) return i;
  return -1;
}

// Find the entry for the well-known table "name".
static int find_wkt(const char *name) {
  int i;
  for (i=0; i < ir_wktt_size; i++) {
    if (strcmp(name, ir_wktt[i].e.name) == 0) return i;
  }
  return -1;
}

#if 0
// Print out an IREP structure.  Unused, except for debugging.
static int iir_print(char *lrep,char *lp,void *bp,ir_element *ep, int treat_as_scalar) {
  int i, j, errcnt = 0;

  if (ep->typ == T_tbl) { // Current IREP element is a struct.
    printf("T: %4ld %10s %2d %3ld %3ld %3d %d:%d %d %s\n",
    (long int)bp, ep->name,
    ep->ti, ep->sz, ep->off, ep->len, ep->flb, ep->fub, ep->typ, lrep);

    char *nlp;
    void *nbp;
    ir_element *nep = ep;

    // Scalar struct, or 1 element of an array.
    if (ep->fub == 0 || treat_as_scalar) {
      for (i=0; ir_ta[ep->ti][i].name; i++) {
        nep = &ir_ta[ep->ti][i];
        nlp = lp + snprintf(lp, BSZ+(lrep-lp), ".%s", nep->name);
        nbp = bp + nep->off;
        errcnt += iir_print(lrep, nlp, nbp, nep, 0);
      }

    } else { // Array of structs.
      for (j=ep->flb; j<=ep->fub; j++) {
        nlp = lp + snprintf(lp, BSZ+(lrep-lp), "[%d]", j);
        nbp = bp + (j - ep->flb)*ep->sz;
        errcnt += iir_print(lrep, nlp, nbp, nep, 1);
      }
    }

  // Current IREP element is NOT a struct.  (It is either scalar POD,
  // or an array thereof.)
  } else {
    // Loop below executes at least once, for a scalar.  More for array.
    i = ep->flb; // For a scalar, flb is always 1, and fub is always 0.
    do {
      printf("%d: %4ld %10s %2d %3ld %3ld %3d %d:%d %d %s\n", i,
      (long int)(bp + (i - ep->flb)*ep->sz), ep->name,
      ep->ti, ep->sz, ep->off, ep->len, ep->flb, ep->fub, ep->typ, lrep);
    } while (++i <= ep->fub);
  }
  *lp = '\0';
  return errcnt;
}
#endif

// Handle variables of "type" ir_reference.  These variables become
// Lua references, to be handled later by the compiled code as needed.
static int read_ref(lua_State *L,char *lrep,void *bp) {
  *((int *)bp) = luaL_ref(L, LUA_REGISTRYINDEX);
  lua_pushnil(L);
  Dbg_print("%s = %d", lrep, *((int *)bp));
  return 0;
}

// Store a name (lrep) using an address (its associated lua_cb_data) as
// the key.  Internal, used by read_cbk.
static void ir_set_function_name(lua_State *L,char *lrep,void *p) {
  lua_pushlightuserdata(L,p);
  lua_pushstring(L,lrep);
  lua_settable(L,LUA_REGISTRYINDEX);
}

// Retrieve a name by address.  Strdup is a memory leak.  This routine
// should normally only ever get used in error reporting, however, so
// the memory leak is probably not very important.
char *ir_get_function_name(lua_State *L,void *p) {
  lua_pushlightuserdata(L,p);
  lua_gettable(L,LUA_REGISTRYINDEX);
  char *s = strdup(lua_tostring(L,-1));
  lua_pop(L,1);
  return s;
}

// These two functions are externals, used by Lua callback evaluators.
int ir_nprm(int npnr) { return npnr%1024 - 9; }
int ir_nret(int npnr) { return npnr/1024 - 9; }

// Read a Lua callback function.
static int read_cbk(lua_State *L,char *lrep,void *bp,ir_element *ep) {
  int i, ii, fref = LUA_NOREF, tv = lua_type(L,-1), npnr = ep->len, base_npnr = ep->len;
  lua_cb_data *cb = (lua_cb_data *)bp;

  if (tv!=LUA_TNUMBER && tv!=LUA_TTABLE && tv!=LUA_TFUNCTION)
    return Ir_error("Expected function, array, or number: %s", lrep);

  if (tv == LUA_TFUNCTION) {
    fref = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pushnil(L);

  } else {
    // Unpack nprm, nret.  Ir_generate enforces nprm>=-1, nret>=-1.
    // The packing algorithm will allow a lower limit of -9.
    int nprm = ir_nprm(npnr);
    int nret = ir_nret(npnr);

    if (nret == 0)
      return Ir_error("``%s'': Function declares zero return values."
      "  Returning a Lua scalar or constant array is not allowed.", lrep);

    ii = (tv==LUA_TTABLE) ? lua_objlen(L,-1) : (nret>0) ? nret : 1;
    if (nret != -1 && ii != nret)
      return Ir_error("``%s'': need %d return val(s), got %d", lrep, nret, ii);

    // Recalculate npnr if nret was originally -1.  This is done so that
    // the callback evaluator will receive the actual length of the data
    // buffer in the nret value.
    if (nret == -1) npnr = (ii+9)*1024 + nprm+9;

    cb->data = realloc(cb->data, ii*sizeof(double));
    if (!cb->data) return Ir_error("``%s'': realloc failed", lrep);
    double *dp = (double *)cb->data;

    if (tv == LUA_TNUMBER) { // Input is a scalar Lua number.
      // Broadcast the scalar to all return values.  We treat nret==-1
      // as if it were nret==1.  (The scalar is treated as a table of
      // length 1.)
      i = 0;
      do {
        dp[i] = lua_tonumber(L,-1);
        Dbg_print("%s.data[%d] = %25.17e", lrep, i, dp[i]);
      } while (++i < nret);

    } else { // Input is a Lua table.
      for (i=1; i<=ii; i++) {
        lua_rawgeti(L,-1,i);
        if (lua_type(L,-1) != LUA_TNUMBER)
          return Ir_error("Bad entry: %s[%d]: %s",lrep,i,lua_tostring(L,-1));
        dp[i-1] = lua_tonumber(L,-1);
        Dbg_print("%s.data[%d] = %25.17e",lrep,i-1,dp[i-1]);
        lua_pop(L,1);
      }
    }
  }
  cb->npnr = npnr;
  cb->base_npnr = base_npnr;
  Dbg_print("%s.npnr = %d (%d,%d)", lrep, npnr, ir_nprm(npnr),ir_nret(npnr));
  cb->fref = fref;
  Dbg_print("%s.fref = %d", lrep, fref);
  ir_set_function_name(L,lrep,bp);
  return 0;
}

// The internal table reader.
// L:    Lua top-of-stack, contains the equivalent of lrep.
// lrep: Current full name, e.g., "table.subtable.element[3].key"
// lp:   Pointer to the right end of lrep.
// bp:   IREP base address for the current element.
// ep:   Descriptor for the current element.
static int iir_read(lua_State *L,char *lrep,char *lp,void *bp,ir_element *ep) {
  int i, errcnt = 0, tv = lua_type(L,-1);

  // A self-referential table will overflow.
  if (!lua_checkstack(L,6)) return Ir_error("stack overflow: %s",lrep);

  // Callback functions and references are handled separately.
  if (ep->typ == T_cbk) return read_cbk(L, lrep, bp, ep);
  if (ep->typ == T_ref) return read_ref(L, lrep, bp);

  if (tv != LUA_TTABLE) { // if top of stack is a scalar value, read it now.
    if (tv == LUA_TSTRING) {
      char *pchar = (char *)bp;
      size_t vlen;
      const char *vp=lua_tolstring(L,-1,&vlen);
      if (ep->typ != T_str) return TYP_ERR(lrep, T_str, ep->typ);
      if (vlen > ep->len - 1)
        return Ir_error("String too long (max %d): %s (%s)",ep->len,lrep,vp);
      (void)strcpy(pchar, vp);
      Dbg_print("%s = %s", lrep, pchar);

    } else if (tv == LUA_TBOOLEAN) {
      BOOLEAN *pbool = (BOOLEAN *)bp;
      if (ep->typ != T_log) return TYP_ERR(lrep, T_log, ep->typ);
      *pbool = (BOOLEAN)lua_toboolean(L,-1);
      Dbg_print("%s = %c",lrep, ((*pbool) ? 'T' : 'F'));

    } else if (tv == LUA_TNUMBER) {
      if (ep->typ!=T_dbl && ep->typ!=T_int) return TYP_ERR(lrep,T_dbl,ep->typ);
      double d = lua_tonumber(L,-1);
      int isint = ((d - (double)(int)d) == 0.0);
      if (ep->typ == T_dbl) {
        double *pdbl = (double *)bp;
        *pdbl = d;
        if (isint) Dbg_print("%s = %d", lrep, (int)(*pdbl));
        else       Dbg_print("%s = %25.17e", lrep, *pdbl);
      } else if (ep->typ == T_int) {
        int *pint = (int *)bp;
        if (isint) {
          *pint = (int)d;
          Dbg_print("%s = %d", lrep, *pint);
        } else return Ir_error("Integer value expected: %s: %25.17e", lrep,d);
      }

    } else {
      return Ir_error("Wrong type: %s (%s): Expected: %s",
        lrep, lua_typename(L,tv), s_typ[ep->typ]);
    }
    return 0;
  }

  // If we get here, Lua TOS must be a table.  Verify that the corresponding
  // IREP element is also a table, or an array.
  if (ep->typ != T_tbl && ep->fub == 0) return TYP_ERR(lrep, T_tbl, ep->typ);

  // Process the subtable recursively.
  for (lua_pushnil(L); lua_next(L,-2); lua_pop(L,1)) {
    char *nlp = lp;
    void *nbp = bp;
    ir_element *nep = ep;

    if (lua_type(L,-2) == LUA_TSTRING) { // Table has string keys.
      const char *s = lua_tostring(L,-2);
      nlp += snprintf(lp, BSZ+(lrep-lp),  ".%s", s);
      i = find_element(s, ir_ta[ep->ti]);
      if (i == -1) {
        lua_pop(L, 2);
        return Ir_error("No such IREP variable: %s (%s)", s, lrep);
      }
      nep = &ir_ta[ep->ti][i];
      nbp += nep->off;

    } else if (lua_type(L,-2) == LUA_TNUMBER) { // Table has numeric keys.
      i = (int)lua_tonumber(L,-2);
      nlp += snprintf(lp, BSZ+(lrep-lp), "[%d]", i);
      if (i<ep->flb || i>ep->fub) {
        lua_pop(L, 2);
        return Ir_error("Array bounds exceeded: %s[%d] (%d:%d)",
          lrep,i,ep->flb,ep->fub);
      }
      nbp += (i - ep->flb)*ep->sz;

    } else {
      lua_pop(L, 2);
      return Ir_error("Expected string or integer key: %s", lrep);
    }
    errcnt += iir_read(L, lrep, nlp, nbp, nep);
    *lp = '\0'; // Restore previous trailing null in lrep.
  }
  return errcnt;
}

static void newtable_byname(lua_State *L, const char *name) {
                            // TOS  (Lua stack initially has table T.)
                            //  T
  lua_newtable(L);          //  {}  T
  lua_pushvalue(L,-1);      //  {}  {}  T
  lua_setfield(L,-3,name);  //  {}  T         (Set T[name] = {}, pop 1 value.)
}

#define lua_swap(L) lua_insert(L,-2)
static void newtable_byindex(lua_State *L, int k) {
                         // TOS  (Lua stack initially has table T.)
                         //  T
  lua_newtable(L);       //  {}  T
  lua_pushvalue(L,-1);   //  {}  {}  T
  lua_pushinteger(L,k);  //  k   {}  {}  T
  lua_swap(L);           //  {}  k   {}  T  (x<>y, for HP RPN programmers.)
  lua_settable(L,-4);    //  {}  T          (Set T[k] = {}, pop 2 values.)
}

// Push an IREP table back to the lua_State.
static int iir_unread(lua_State *L,char *lrep,char *lp,void *bp,ir_element *ep, int treat_as_scalar) {
  int i, j, errcnt = 0;

  if (ep->typ == T_tbl) { // Current IREP element is a struct.
    char *nlp;
    void *nbp;
    ir_element *nep = ep;

    // Scalar struct, or 1 element of an array.
    if (ep->fub == 0 || treat_as_scalar) {
      for (i=0; ir_ta[ep->ti][i].name; i++) {
        nep = &ir_ta[ep->ti][i];
        nlp = lp + snprintf(lp, BSZ+(lrep-lp), ".%s", nep->name);
        nbp = bp + nep->off;
        if (nep->typ == T_tbl) newtable_byname(L,nep->name);
        errcnt += iir_unread(L, lrep, nlp, nbp, nep, 0);
        if (nep->typ == T_tbl) lua_pop(L,1);
      }

    } else { // Array of structs.
      for (j=ep->flb; j<=ep->fub; j++) {
        nlp = lp + snprintf(lp, BSZ+(lrep-lp), "[%d]", j);
        nbp = bp + (j - ep->flb)*ep->sz;
        newtable_byindex(L,j);
        errcnt += iir_unread(L, lrep, nlp, nbp, nep, 1);
        lua_pop(L,1);
      }
    }

  // Current IREP element is NOT a struct.  (It is either scalar POD,
  // or an array thereof.)
  } else {

    if (ep->fub > 0) newtable_byname(L,ep->name);

    // Loop below executes at least once, for a scalar.  More for array.
    i = ep->flb; // For a scalar, flb is always 1, and fub is always 0.
    do {
      if (ep->fub > 0) lua_pushinteger(L, i);       // An array index,
      else             lua_pushstring(L, ep->name); // Or the element name.

      // Push the element value onto the Lua stack.
      if (ep->typ == T_str) {
        lua_pushstring(L, (char *)bp);
      } else if (ep->typ == T_dbl) {
        lua_pushnumber(L, *((double *)bp));
      } else if (ep->typ == T_int) {
        lua_pushinteger(L, *((int *)bp));
      } else if (ep->typ == T_log) {
        lua_pushboolean(L, *((BOOLEAN *)bp));
      } else {
        return Ir_error("IR_UNREAD: bad type: %s (%s)", lrep, s_typ[ep->typ]);
      }
      lua_settable(L,-3); // Set the table key+value (and pop both.)
    } while (++i <= ep->fub);
    if (ep->fub > 0) lua_pop(L,1); // If it was an array, pop it: we're done.
  }
  *lp = '\0';
  return errcnt;
}

// Empty the Lua stack; load an arbitrary element name onto TOS.
static int ir_elem(lua_State *L, const char *s) {
  char buf[BSZ];
  (void)snprintf(buf, sizeof buf, "return %s", s);
  lua_settop(L,0);
  return luaL_loadstring(L, buf) || lua_pcall(L,0,1,0);
}

// External entry point: ir_read(L, "table[.subtable...]").
int ir_read(lua_State *L, const char *table_name) {
  int n = strlen(table_name);
  if (n > BSZ) return Ir_error("Table name too long: %s", table_name);

  if (ir_elem(L,table_name))
    return Ir_error("Bad Lua table: %s: %s", table_name, lua_tostring(L,-1));

  irep_debug = getenv("irep_debug") ? atoi(getenv("irep_debug")) : 0;

  // Find the well known table name first.
  char *s, tcopy[BSZ], lrep[BSZ];
  (void)strcpy(tcopy, table_name);
  s = strtok(tcopy, ".[]");
  int i = find_wkt(s);
  if (i == -1) return Ir_error("No such IREP table: %s (%s)", s,table_name);

  ir_wkt_desc *w = &ir_wktt[i];
  void *bp = w->p;
  ir_element *ep = &w->e;

  // Walk down any remaining elements after the wkt name.
  while ((s = strtok(0, ".[]"))) {
    if (isalpha((int)(*s)) || *s == '_') { // string key
      int j = find_element(s, ir_ta[ep->ti]);
      if (j == -1) return Ir_error("IREP key not found: %s (%s)", s,table_name);
      ep = &ir_ta[ep->ti][j];
      bp += ep->off;

    } else if (isdigit((int)(*s))) { // numeric key
      int j = atoi(s);
      if (j<ep->flb || j>ep->fub)
        return Ir_error("Array bounds exceeded: %s[%d] (%d:%d)",
        table_name, j, ep->flb, ep->fub);
      bp += (j - ep->flb)*ep->sz;

    } else {
      return Ir_error("Bad table element: %s (%s)", s,table_name);
    }
  }
  (void)strcpy(lrep, table_name);
  return iir_read(L, lrep, lrep+n, bp, ep);
}

// Push an IREP table to the lua_State (reverse of ir_read.)
// For now, can only handle the whole wkt.
int ir_unread(lua_State *L, const char *ir_tbl) {
  char lrep[BSZ];
  int n = strlen(ir_tbl);
  if (n > BSZ) return Ir_error("Table name too long: %s", ir_tbl);

  // Find the IREP table.
  int i = find_wkt(ir_tbl);
  if (i == -1) return Ir_error("No such IREP table: %s", ir_tbl);
  ir_wkt_desc *w = &ir_wktt[i];
  void *bp = w->p;
  ir_element *ep = &w->e;

  // (Re-)create the corresponding Lua table.
  lua_settop(L,0);
  lua_newtable(L);
  lua_pushvalue(L,-1);
  lua_setglobal(L,ir_tbl);
  (void)strcpy(lrep, ir_tbl);
  return iir_unread(L, lrep, lrep+n, bp, ep, 0);
}

// Check existence of an element.  If found, leave it on TOS.
int ir_exists(lua_State *L, const char *s) {
  if (ir_elem(L,s)) return 0;
  return !lua_isnil(L,-1);
}

// Return the run time length of a vector.
int ir_rtlen(lua_State *L, const char *s) {
  if (ir_elem(L,s)) return -1;
  int n = lua_type(L,-1);
  return (n==LUA_TNIL) ? -1 : ((n==LUA_TNUMBER) ? 0 : (int)lua_objlen(L,-1));
}

// Read an (arbitrarily large) string, stored earlier as an ir_reference.
// The third argument can be NULL if you're not interested in the length.
// The returned string must be copied into the caller's scope, and you
// should call lua_pop(L,-1) after that is done, to allow Lua to garbage
// collect the item.  Typical calling sequence:
//   if (ir_exists(L, "physics.foo")) {
//     int nn;
//     std::string foo = ir_get_stringref(L,irep::physics.foo,&nn);
//     lua_pop(L,-1);
//   }
const char *ir_get_stringref(lua_State *L, int n, int *len) {
  if (n != LUA_REFNIL) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, n);
    int ii = lua_type(L,-1);
    if (ii == LUA_TSTRING) return lua_tolstring(L,-1,(size_t *)len);
    (void)fprintf(stderr,"ERROR (Lua/IR): IR_GET_STRINGREF: Bad value(%s): "
      "ir_reference variable should be a string",lua_typename(L,ii));
  }
  return 0;
}

#if defined(__cplusplus)
}
#endif
