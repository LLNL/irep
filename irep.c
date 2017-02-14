// Copyright (c) 2016, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory.  Written by Lee Busby,
// busby1@llnl.gov. LLNL-CODE-702338. All rights reserved.
// See ./Copyright for additional notices.

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#if defined(__cplusplus)
extern "C" {
#endif

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#define BSZ 2048

// Some necessary forward declarations:
typedef struct sbuf_t { char frep[BSZ], lrep[BSZ]; } sbuf_t;
typedef int (*sh_handler)(lua_State *L, sbuf_t *sp, char *fp, char *lp);
static int iir_read(lua_State *L, sbuf_t *sb, char *fp, char *lp, int sh);
static int read_nml(const char *frep, const char *val, int addnull);

static int ir_error(const char *format, ...) {
  va_list argp;
  char buf[BSZ];
  va_start(argp, format);
  (void) vsnprintf(buf, sizeof(buf), format, argp);
  va_end(argp);
  (void)fprintf(stderr, "ERROR (Lua/IR): %s\n", buf);
  return 1;
}

#define irep_c
#include "ir_special.h"

// Simple regular expression pattern matcher.
// See Section 9.2, "The Practice of Programming", Kernighan & Pike.
// www.cs.princeton.edu/courses/archive/spr09/cos333/beautiful.html
static int matchhere(const char *regexp, const char *text);

// matchstar: search for c*regexp at beginning of text.
static int matchstar(int c, const char *regexp, const char *text) {
  do { // a * matches zero or more instances
    if (matchhere(regexp, text)) return 1;
  } while (*text != '\0' && (*text++ == c || c == '?'));
  return 0;
}
// matchhere: search for regexp at beginning of text.
static int matchhere(const char *regexp, const char *text) {
  if (regexp[0] == '\0') return 1;
  if (regexp[1] == '*')  return matchstar(regexp[0], regexp+2, text);
  if (regexp[0] == '$' && regexp[1] == '\0') return *text == '\0';
  if (*text!='\0' && (regexp[0]=='?' || regexp[0]==*text))
    return matchhere(regexp+1, text+1);
  return 0;
}
// match: search for regexp anywhere in text.
static int match(const char *regexp, const char *text) {
  if (regexp[0] == '^') return matchhere(regexp+1, text);
  do { // must look even if string is empty
    if (matchhere(regexp, text)) return 1;
  } while (*text++ != '\0');
  return 0;
}

// Construct an assignment statement and pass it to namelist read.
static int read_nml(const char *frep, const char *val, int addnull) {
  char buf[BSZ];
  extern int ir_rd_nml(char *, int, int);
  (void)snprintf(buf, BSZ, "&ir_input %s = %s", frep, val);
#ifdef IR_DEBUG
  (void)fprintf(stderr, "read_nml: %s\n", buf);
#endif
  return ir_rd_nml(buf, (int)strlen(buf), addnull);
}

// Read a Lua callback function into a struct lua_cb_dd_data, at index i.
static int read_cb(lua_State *L, sbuf_t *sb, char *fp, char *lp, int i) {
  char val[64];
  const char *fmt = "reading ``%s = %s''";
  int fref = LUA_NOREF, tv = lua_type(L,-1);

  if (tv == LUA_TNUMBER || tv == LUA_TTABLE || tv == LUA_TFUNCTION) {
    (void)snprintf(fp, BSZ+(sb->frep-fp), "%%nprm");
    (void)snprintf(val, sizeof(val), "%d", cbf_tbl[i].nprm);
    if (read_nml(sb->frep, val, 0) != 0) return ir_error(fmt, sb->lrep, val);

    (void)snprintf(fp, BSZ+(sb->frep-fp), "%%nret");
    (void)snprintf(val, sizeof(val), "%d", cbf_tbl[i].nret);
    if (read_nml(sb->frep, val, 0) != 0) return ir_error(fmt, sb->lrep, val);

    if (tv == LUA_TFUNCTION) {
      fref = luaL_ref(L, LUA_REGISTRYINDEX);
      lua_pushnil(L);

    } else {
      char *nfp = fp;
      if (tv == LUA_TNUMBER) {
        nfp += snprintf(fp, BSZ+(sb->frep-fp), "%%const_val(1)");

      } else { // Table.
        if (lua_objlen(L,-1) != cbf_tbl[i].nret)
          return ir_error("``%s'', expected array of %d value(s), got %d", 
            sb->lrep, cbf_tbl[i].nret, lua_objlen(L,-1));
        nfp += snprintf(fp, BSZ+(sb->frep-fp), "%%const_val");
      }
      if (iir_read(L, sb, nfp, lp, 0) != 0) return 1;
      *fp = '\0';
    }

  } else {
    return ir_error("Expected function, array, or number: %s", sb->lrep);
  }
  (void)snprintf(fp, BSZ+(sb->frep-fp), "%%fref");
  (void)snprintf(val, sizeof(val), "%d", fref);
  if (read_nml(sb->frep, val, 0) != 0) return ir_error(fmt, sb->lrep, val);
  return 0;
}

// The internal table reader.
static int iir_read(lua_State *L, sbuf_t *sb, char *fp, char *lp, int sh) {
  char val[BSZ];
  const char *ft[] = {".false.",".true."}, *fmt = "reading ``%s = %s''";
  int errcnt = 0, i, tv = lua_type(L,-1);

  // Self-referential table, maybe?
  if (!lua_checkstack(L,6)) return ir_error("stack overflow: %s",sb->lrep);

  // Callback function or special handling requested?
  if (sh) {
    for(i=0; i < (int)(sizeof(cbf_tbl)/sizeof(cbf_tbl[0])); i++)
      if (match(cbf_tbl[i].pat, sb->lrep) > 0)
        return read_cb(L, sb, fp, lp, i);

    for(i=0; i < (int)(sizeof(sh_tbl)/sizeof(sh_tbl[0])); i++)
      if (match(sh_tbl[i].pat, sb->lrep) > 0)
        return sh_tbl[i].h(L, sb, fp, lp);
  }

  if (tv != LUA_TTABLE) { // Top of stack is a scalar value: read it.

    if (tv == LUA_TSTRING) {
      char dim2[16], val4[4*BSZ], *p4=val4;
      size_t vlen;
      const char *vp=lua_tolstring(L,-1,&vlen);
      // val4 gets an array of single characters: 'a','b','c', ...
      for (i=0; i<(int)vlen; i++, p4+=4) (void)sprintf(p4, "'%c',", vp[i]);
      if (*(fp-1) == ')') {                // We have a vector of strings.
        while (*fp != '(') fp--;           // Find the preceding '(', save
        (void)sprintf(dim2, "%s", fp+1);   // the existing dimension, and
        (void)sprintf(fp+1, ":,%s", dim2); // right shift it (now 2nd dim.)
      }
      if (read_nml(sb->frep, val4, 1) != 0) return ir_error(fmt, sb->lrep, vp);

      // The rest of the string is overfilled with blanks.
      if (*fp == '(') (void)sprintf(fp, "(%d:,%s", (int)vlen+2, dim2);
      else            (void)sprintf(fp, "(%d:)",   (int)vlen+2);
      (void)snprintf(val4, BSZ, "%d*' '", BSZ);
      (void)read_nml(sb->frep, val4, 0); // Ignore error (too many blanks.)

    } else if (tv == LUA_TBOOLEAN) {
      const char *ss = ft[lua_toboolean(L,-1)];
      if (read_nml(sb->frep, ss, 0) != 0) return ir_error(fmt, sb->lrep, ss);

    } else if (tv == LUA_TNUMBER) {
      double d = lua_tonumber(L,-1);
      int isint = ((d - (double)(int)d) == 0.0);
      if (isint) (void)snprintf(val, BSZ, "%d", (int)d);
      else       (void)snprintf(val, BSZ, "%25.17e", d);
      if (read_nml(sb->frep, val, 0) != 0) return ir_error(fmt, sb->lrep, val);

    } else {
      return ir_error("Bad name, missing definition(?): %s", sb->lrep);
    }
    return 0;
  }

  for (lua_pushnil(L); lua_next(L,-2); lua_pop(L,1)) { // TOS is a subtable.
    char *nfp = fp, *nlp = lp;
    int nsh = sh;
    if (lua_type(L,-2) == LUA_TSTRING) {
      const char *s = lua_tostring(L,-2);
      nfp += snprintf(fp, BSZ+(sb->frep-fp), "%%%s", s);
      nlp += snprintf(lp, BSZ+(sb->lrep-fp),  ".%s", s);
    } else if (lua_type(L,-2) == LUA_TNUMBER) {
      i = (int)lua_tonumber(L,-2);
      nfp += snprintf(fp, BSZ+(sb->frep-fp), "(%d)", i);
      nlp += snprintf(lp, BSZ+(sb->lrep-fp), "[%d]", i);
    } else {
      return ir_error("Expected string or integer key: %s", sb->lrep);
    }
    errcnt += iir_read(L, sb, nfp, nlp, nsh);
    *fp = *lp = '\0'; // Restore trailing null.
  }
  return errcnt;
}

// Copy a Lua element name (lrep) into a Fortran element name (frep).
static char *cpfrep(char *dest, const char *src) {
  int i;
  for(i=0; src[i]; i++) {
    dest[i] = src[i];
    if (src[i] == '[') dest[i] = '(';
    if (src[i] == ']') dest[i] = ')';
    if (src[i] == '.') dest[i] = '%';
  }
  dest[i] = src[i];
  return dest;
}

// Empty the Lua stack, and load an arbitrary element name onto TOS.
static int ir_elem(lua_State *L, const char *s) {
  char buf[BSZ];
  (void)snprintf(buf, sizeof buf, "return %s", s);
  lua_settop(L,0);
  return luaL_loadstring(L, buf) || lua_pcall(L,0,1,0);
}

// External entry point: ir_read(L, "table[.subtable.subsubtable.value]").
int ir_read(lua_State *L, const char *s) {
  sbuf_t sbuf;
  int n = strlen(s);
  if (n > BSZ) return ir_error("Element name too long: %s", s);
  if (ir_elem(L,s)) return ir_error("Bad element: %s", lua_tostring(L,-1));
  char *lp = strcpy(sbuf.lrep, s) + n;
  char *fp = cpfrep(sbuf.frep, s) + n;
  return iir_read(L, &sbuf, fp, lp, 1);
}

// Check existence of an element.  If found, leave it on TOS.
int ir_exists(lua_State *L, const char *s) {
  if (ir_elem(L,s)) return 0;
  return !lua_isnil(L,-1);
}

// Non-blank length: Utility function for use by IR_LEN macro.
int ir_nblen(char *s, int n) {
  do { --n; } while ((s[n] == ' ' || s[n] == '\0') && n >= 0);
  return n+1;
}

// Return the run time length of a vector.
int ir_rtlen(lua_State *L, const char *s) {
  if (ir_elem(L,s)) return -1;
  int n = lua_type(L,-1);
  return (n==LUA_TNIL) ? -1 : ((n==LUA_TNUMBER) ? 0 : (int)lua_objlen(L,-1));
}

#if defined(__cplusplus)
}
#endif
