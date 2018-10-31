-- Copyright (c) 2016, Lawrence Livermore National Security, LLC. Produced
-- at the Lawrence Livermore National Laboratory.  Written by Lee Busby,
-- busby1@llnl.gov. LLNL-CODE-702338. All rights reserved.
-- See ../../Copyright for additional notices.

table1 = {
  b = true,
  d = 2.71,
  e = { 1.1, 2.2, 3.3, 4.4, 5.5 },
  -- f1 = function(x,y,z) return x*y*z end,
  f1 = 3,
  -- f4 = function(x,y,z) return x+y+z end,
  f4 = { 2, 2+3, 2+3+4 },
  -- f5 = function(x,y,z) return { 50, 30, 40 } end,
  -- f5 = function(x,y,z) return "abcdef" end,
  f5 = { 1.23, 3.45, 4.56 },
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
