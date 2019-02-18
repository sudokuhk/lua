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

static int l_add(lua_State * L)
{
    double op1 = luaL_checknumber(L, 1);
    double op2 = luaL_checknumber(L, 2);

    lua_pushnumber(L, op1 + op2);
    return 1;
}

static int l_sub(lua_State * L)
{
    double op1 = luaL_checknumber(L, 1);
    double op2 = luaL_checknumber(L, 2);

    lua_pushnumber(L, op1 - op2);    
    return 1;
}

static int l_min(lua_State * L)
{
    double op1 = luaL_checknumber(L, 1);
    double op2 = luaL_checknumber(L, 2);
    
    lua_pushnumber(L, op1 > op2 ? op2 : op1);    
    return 1;
}

static int l_max(lua_State * L)
{
    double op1 = luaL_checknumber(L, 1);
    double op2 = luaL_checknumber(L, 2);
    
    lua_pushnumber(L, op1 < op2 ? op2 : op1);    
    return 1;
}

static int l_abs(lua_State * L)
{
    double op1 = luaL_checknumber(L, 1);
    
    lua_pushnumber(L, op1 > 0 ? op1 : -op1);
    return 1;
}

static luaL_Reg mylibs[] = {
    {"add", l_add},
    {"sub", l_sub},
    {"min", l_min},
    {"max", l_max},
    {"abs", l_abs},
    {NULL, NULL},
};

DEFINE_REGISTER(cmath, mylibs);
