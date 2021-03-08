-- Copyright 2016-2021 Lawrence Livermore National Security, LLC and other
-- IREP Project Developers. See the top-level LICENSE file for details.
--
-- SPDX-License-Identifier: MIT

table1 = {
  b = true,
  d = 2.71,
  e = { 1.1, 2.2, 3.3, 4.4, 5.5 },
  f1 = function(x,y,z) return x*y*z end,
  -- f4 = function(x,y,z) return x+y+z end,
  f4 = { 2, 2+3, 2+3+4 },
  i = 42,
  s = 'abcdefg',
  -- s = function() return true end,
  table2 = {
    [1] = {
      i = 1,
      f2 = function(x,y,z) return x+y+z end,
    },
  },
  table3 = {
    i = 2,
    f3 = { 1, 43 },
    -- xx = { 1,2,3, "ignore me" },
  },
}

table4 = {
  [1] = {
    name = "this is table4[1].name",
  },
  [2] = {
    name = "this is table4[2].name",
    fooref = { 3,2,1 },
    -- fooref = "abcdef",
    -- fooref = function() return 3 end,
  },
  -- [3]: Use the IREP default.
}
