-- example usage:
-- 
-- require 'print_r'
-- a = {x=1, y=2, label={text='hans', color='blue'}, list={'a','b','c'}}
-- pr(a, 'My table')
--
--My table = {
--|  y = 2
--|  x = 1
--|  list = {
--|  |  [1] = "a"
--|  |  [2] = "b"
--|  |  [3] = "c"
--|  }
--|  label = {
--|  |  color = "blue"
--|  |  text = "hans"
--|  }
--}
--
-- Copyright 2009: hans@hpelbers.org
-- This is freeware


function print_r (t, name, indent)
  local tableList = {}
  function table_r (t, name, indent, full)
    local id = not full and name
        or type(name)~="number" and tostring(name) or '['..name..']'
    local tag = indent .. id .. ' = '
    local out = {}	-- result
    if type(t) == "table" then
      if tableList[t] ~= nil then table.insert(out, tag .. '{} -- ' .. tableList[t] .. ' (self reference)')
      else
        tableList[t]= full and (full .. '.' .. id) or id
        if next(t) then -- Table not empty
          table.insert(out, tag .. '{')
          for key,value in pairs(t) do 
            --table.insert(out,table_r(value,key,indent .. '|  ',tableList[t]))
            table.insert(out,table_r(value,key,indent .. '  ',tableList[t]))
          end 
          table.insert(out,indent .. '}')
        else table.insert(out,tag .. '{}') end
      end
    else 
      local val = type(t)~="number" and type(t)~="boolean" and '"'..tostring(t)..'"' or tostring(t)
      table.insert(out, tag .. val)
    end
    return table.concat(out, '\n')
  end
  return table_r(t,name or 'Value',indent or '')
end

function pr (t, name)
  print(print_r(t,name))
end

function print_table(name)
  print(print_r(_G[name], name))
end
