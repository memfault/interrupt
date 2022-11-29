#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

extern int luaopen_bindings(lua_State *L);

int32_t multiply(int32_t x, int32_t y)
{
    int32_t result = x * y;
    printf("[C] Multiply %d * %d = %d\r\n", x, y, result);
    return result;
}

int main(void)
{
    lua_State *L = luaL_newstate();
    assert(L != NULL);

    luaL_openlibs(L);
    luaopen_bindings(L); // Load the wrapped module

    int r = luaL_loadfile(L, "example1.lua");
    assert(r == LUA_OK);

    printf("[C] Calling Lua\r\n");

    if (lua_pcall(L, 0, 0, 0) != LUA_OK)
    {
        const char *msg = lua_tostring(L, -1);
        printf("[LUA] Error: %s\n", msg);
    }

    printf("[C] We're back from Lua\r\n");
    lua_close(L);
    printf("[C] Finished\r\n");
}
