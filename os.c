/*************************************************************************
    > File Name: os.c
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Mon Feb 18 14:29:25 2019
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <defines.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

static int l_sleep(lua_State * L)
{
    int seconds = 0;
    if (lua_isnumber(L, 1) != 1) {
        return;
    }
    seconds = lua_tonumber(L, 1);
    sleep(seconds);
    return 0;
}

static int l_usleep(lua_State * L)
{
    int ms = 0;
    if (lua_isnumber(L, 1) != 1) {
        return;
    }
    ms = lua_tonumber(L, 1);
    usleep(ms);
    return 0;
}

const static luaL_Reg l_os[] = {
    {"sleep", l_sleep}, 
    {"usleep", l_usleep}, 
    {NULL, NULL},
};

DEFINE_REGISTER(cos, l_os);
