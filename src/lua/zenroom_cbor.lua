-- This file is part of Zenroom (https://zenroom.dyne.org)
--
-- Copyright (C) 2019 Dyne.org foundation
-- designed, written and maintained by Denis Roio <jaromil@dyne.org>
--
-- This program is free software: you can redistribute it and/or modify
-- it under the terms of the GNU Affero General Public License as
-- published by the Free Software Foundation, either version 3 of the
-- License, or (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Affero General Public License for more details.
--
-- You should have received a copy of the GNU Affero General Public License
-- along with this program.  If not, see <https://www.gnu.org/licenses/>.

local _cbor = require('cbor')

_cbor.decode = _cbor.raw_decode

_cbor.encode = function(tab)
   -- encodes zencode types according to CODEC
   assert(luatype(tab) == 'table', "CBOR encode argument needs to be a table")
   local fun
   local res = { }
   for k,v in pairs(tab) do
	  fun = guess_outcast( check_codec(k) )
	  if luatype(v) == 'table' then
		 res[k] = deepmap(fun, v)
	  else
		 res[k] = fun(v)
	  end
   end
   return _cbor.raw_encode( res )
end

_cbor.auto = function(obj)
   local t = luatype(obj)
   if t == 'table' then
	  -- export table to JSON
	  return _cbor.encode(obj)
   elseif t == 'string' then
	  -- import JSON string to table
	  return _cbor.decode(obj)
   else
	  error("CBOR.auto unrecognised input type: "..t, 3)
	  return nil
   end
end

return _cbor
