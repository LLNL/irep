#!/usr/bin/env lua

-- Copyright (c) 2016, Lawrence Livermore National Security, LLC. Produced
-- at the Lawrence Livermore National Laboratory.  Written by Lee Busby,
-- busby1@llnl.gov. LLNL-CODE-702338. All rights reserved.
-- See ./Copyright for additional notices.

-- This script processes all the wkt_*.h files to create the ir_generate.h
-- file.  We depend on the marbl build for a particular Lua interpreter.
-- When marbl builds lua or luajit, it places the path of the lua binary
-- into the environment of subsequent package builds.  Thus, /usr/bin/env
-- should always use marbl's Lua.

-- Stbl is an associative string table: stbl.num_comp = "s0123", etc.
-- It's not really necessary, but makes the output look neater.  String
-- numbers count up from 0.
  
-- Tbl_list is a 0-based array.  Each element corresponds to one of the
-- structs in a wkt_*.h file (See the IREP "Beg_struct" macro.)  Each element
-- is itself a table, containing most or all the fields (depending on the
-- element type) of the "ir_element" struct in irep.c.
  
-- Wkt_list is similar to tbl_list.  It records similar information for
-- each well-known-table.  Each entry in wkt_list effectively contains an
-- ir_element, plus the base address of the well-known-table instance.
  
-- Rev_ta is an associative table of type names, such as "irt_method",
-- with that name's index in tbl_list.
  
-- Typename is an array parallel to tbl_list, that lists the structure
-- name for each index.

-- Process arguments and print the preamble.
local i,flg = 0, {}
while i < #arg and arg[i+1]:match("^-[DI]") do
  i=i+1
  flg[i] = arg[i]
end

print("// This file was generated by etc/ir_generate.lua. Do not modify.")
while i < #arg do
  i=i+1
  io.write(string.format("#include \"%s\"\n", arg[i]))
end
print("")

local ct
local scnt, tcnt, wcnt = -1,-1,-1
local stbl, tbl_list, wkt_list, rev_ta, typename = {}, {}, {}, {}, {}

local function pairsbyvalues(t,f)
  local rev, a = {}, {}
  for k,v in pairs(t) do rev[v] = k; a[#a + 1] = v end
  table.sort(a,f)
  local i = 0
  return function()
    i=i+1
    return a[i],rev[a[i]]
  end
end

local function add2stbl(s)
  if not stbl[s] then
    scnt = scnt + 1
    stbl[s] = string.format("s%04d",scnt)
  end
end

local tmap = {
  T_int="int",
  T_dbl="double",
  T_log="_Bool",
  T_str="char",
  T_ref="int",
  T_ptr="void*",
  T_cbk="lua_cb_data",
}
for k,v in pairs(tmap) do add2stbl(v) end

-- Main loop over input lines.  Gmatch skips blank lines.
local p = assert(io.popen("gcc -P -E " .. table.concat(flg," ") .. " -","r"))
for line in string.gmatch(p:read("*all"),'[^\n]+') do
  local f1,f2,f3,f4 = line:match("%s*(%S+)%s+(%S+)%s+(%S+)%s+(%S+)")
  if not f1 then error("Bad input: " .. line) end

  add2stbl(f2)
  if f1=="bst" then -- Begin structure declaration.
    if ct then error("ct should be nil at " .. f2) end
    assert(not rev_ta[f2], "Repeated table: " .. f2)
    ct = f2 -- type name of the struct being declared, e.g., "irt_sources"
    tcnt = tcnt + 1
    tbl_list[tcnt] = {}
    typename[tcnt] = f2
    rev_ta[f2] = tcnt

  elseif f1=="est" then -- End structure declaration.
    assert(ct == f2, "Expected equality: " .. ct .."==" .. f2)
    ct = nil

  elseif f1:match("T_[dilprs]") then -- Leaf declaration (POD or pointer). 
    tbl_list[tcnt][f2] = { typ=f1,len=tonumber(f3),flb=1,fub=tonumber(f4) }

  elseif f1=="T_cbk" then -- Leaf declaration (callback function.) 
    local nprm,nret = f3:match("([+-]?%d+):([+-]?%d+)")
    nprm,nret = tonumber(nprm),tonumber(nret)
    -- Strictly speaking, the lower limit for nprm,nret is -9.  However,
    -- nprm<0 or nret<-1 is semantically incorrect at this time.
    if nprm <  0 or nprm >    1014 then error("Bad nprm: " .. nprm) end
    if nret < -1 or nret > 2097142 then error("Bad nret: " .. nret) end
    local len = nprm+9 + (nret+9)*1024
    tbl_list[tcnt][f2] = { typ=f1,len=len,flb=1,fub=tonumber(f4) }

  elseif f1=="T_tbl" then -- Node declaration.
    local flb,fub = f4:match("([+-]?%d+):([+-]?%d+)")
    tbl_list[tcnt][f2] = { typ=f1,tname=f3,len=0,flb=tonumber(flb),fub=tonumber(fub) }
    add2stbl(f3)

  elseif f1=="wkt" then -- WKT definition.
    assert(rev_ta[f3], "No type declaration for: " .. f3)
    local flb,fub = f4:match("([+-]?%d+):([+-]?%d+)")
    wcnt = wcnt + 1
    wkt_list[wcnt] = { name=f2,tname=f3,flb=tonumber(flb),fub=tonumber(fub),ti=rev_ta[f3] }
    add2stbl(f3)

  else
    error("Bad input: " .. line)
  end
end -- Main loop.
if ct then error("(At end) ct should be nil: " .. ct) end
p:close()

-- Output section.
print("// Part 1: The string table.")
for k,v in pairsbyvalues(stbl) do print("#define "..k.." "..v) end
print("")

print("// Part 2: The element tables.")
local f1 = function(ti, tname, t)
  print("static ir_element " .. tname .. "[] = { // " .. typename[ti])
  for k,v in pairs(t) do
    local idesc = rev_ta[v.tname] or -1
    local szo = stbl[v.tname] or stbl[tmap[v.typ]]
    print(string.format("  { Q(%s),%3d, S(%s), O(%s,%s), %8d,%3d,%3d, %s },",
    stbl[k],idesc,szo,stbl[typename[ti]],stbl[k],v.len,v.flb,v.fub,v.typ))
  end
  print("  { 0 }\n};\n")
end
for i=0,tcnt do f1(i, string.format("ir_tbl%03d", i), tbl_list[i]) end

print("// Part 3: A list of pointers to the ir_element tables.")
print("static ir_element *ir_ta[] = {")
for i=0,tcnt do print(string.format("  ir_tbl%03d, // %s",i,typename[i])) end
print("};\n")

print("// Part 4: List the well-known tables.")
print("static ir_wkt_desc ir_wktt[] = {")
for i=0,wcnt do
  local t = wkt_list[i]
  local nm  = stbl[t.name]
  local tnm = stbl[t.tname]
  print(string.format("  { &%s, {Q(%s),%3d,S(%s),0,0,%3d,%3d,T_tbl }}, // %s",
    nm,nm,t.ti,tnm,t.flb,t.fub,t.name))
end
print("};")