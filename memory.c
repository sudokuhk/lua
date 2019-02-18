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
        void * p = malloc(size + 1);
        *((char *)p + size) = '\0';
        //printf("malloc:%p\n", p);
        lua_pushlightuserdata(L, p);
    }
    return 1;
}

static int l_free(lua_State * L)
{
    if (lua_islightuserdata(L, 1) == 1) {
        void * p = lua_touserdata(L, 1);
        if (p != NULL) {
            //printf("free:%p\n", p);
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

static int l_tostring(lua_State * L)
{
    if (lua_islightuserdata(L, 1) == 1) {
        if (lua_isnumber(L, 2) != 1) {            
            goto out;
        }
        void * p = lua_touserdata(L, 1);
        int size = lua_tonumber(L, 2);
        if (p == NULL) {
            goto out;
        }
        char * pchar = (char *)p;
        *(pchar + size) = '\0';
        lua_pushstring(L, pchar);
        return 1;
    }
out:
    lua_pushnil(L);
    return 1;
}

const static luaL_Reg l_memory[] = {
    {"alloc", l_malloc},
    {"free", l_free},
    {"get", l_get},
    {"tostring", l_tostring},
    {NULL, NULL},
};

DEFINE_REGISTER(cmemory, l_memory);
