/*************************************************************************
    > File Name: cmath.c
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Mon Jan 28 12:04:04 2019
 ************************************************************************/

#include <defines.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

static int add(lua_State * L)
{
    double op1 = luaL_checknumber(L, 1);
    double op2 = luaL_checknumber(L, 2);

    lua_pushnumber(L, op1 + op2);
    return 1;
}

static int sub(lua_State * L)
{
    double op1 = luaL_checknumber(L, 1);
    double op2 = luaL_checknumber(L, 2);

    lua_pushnumber(L, op1 - op2);    
    return 1;
}

static luaL_Reg mylibs[] = {
   {"add", add},
   {"sub", sub},
   {NULL, NULL},
};

DEFINE_REGISTER(cmath, mylibs);
