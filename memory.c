/*************************************************************************
    > File Name: memory.c
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Tue Jan 29 10:28:36 2019
 ************************************************************************/

#include <defines.h>
#include <stdio.h>
#include <stdlib.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

static int l_malloc(lua_State * L)
{
    if (lua_isnumber(L, 1) != 1) {
        lua_pushnil(L);
    } else {
        int size = lua_tonumber(L, 1);
        void * p = malloc(size);
        printf("malloc:%p\n", p);
        lua_pushlightuserdata(L, p);
    }
    return 1;
}

static int l_free(lua_State * L)
{
    if (lua_islightuserdata(L, 1) == 1) {
        void * p = lua_touserdata(L, 1);
        if (p != NULL) {
            printf("free:%p\n", p);
            free(p);
        }
    }
    return 0;
}

static int l_get(lua_State * L)
{
    if (lua_isnumber(L, 1) != 1) {
        lua_pushnil(L);
    } else {
        int size = lua_tonumber(L, 1);
        lua_newuserdata(L, size);
    }
    return 1;
}

const static luaL_Reg l_memory[] = {
    {"alloc", l_malloc},
    {"free", l_free},
    {"get", l_get},
    {NULL, NULL},
};

DEFINE_REGISTER(cmemory, l_memory);
