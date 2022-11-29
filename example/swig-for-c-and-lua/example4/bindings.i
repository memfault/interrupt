%module bindings

%include "stdint.i"
%include "typemaps.i"

%typemap(out) uint8_t my_struct_t::priorities[3]
{
    lua_newtable(L);
    for (uint32_t i = 0; i < 3; i++)
    {
        lua_pushnumber(L, (lua_Number)$1[i]);
        lua_rawseti(L, -2, i + 1); // Lua indexes start from 1
    }

    SWIG_arg++;
}

%{
#include "types.h"
%}

%include "types.h"
