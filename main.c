/*************************************************************************
    > File Name: main.c
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Mon Jan 28 12:03:23 2019
 ************************************************************************/

//gcc main.c -llua -lm -ldl
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <defines.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

int main(int argc, char * argv[])
{
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    
    DO_REGISTER(L, cmath);
    DO_REGISTER(L, cjson);
    DO_REGISTER(L, cnetwork);
    DO_REGISTER(L, cmemory);
    DO_REGISTER(L, curl);
    DO_REGISTER(L, cfile);
    DO_REGISTER(L, cdir);
    DO_REGISTER(L, cos);
    
    if (argc < 2) {
        printf("input lua script file\n");
        return 0;
    }
    
    if (0 != luaL_loadfile(L, argv[1])) {
        printf("load lua error:%s\n", lua_tostring(L, -1));
        return 0;
    }
    
    if (0 != lua_pcall(L, 0, 0, 0)) {
        printf("call lua error:%s\n", lua_tostring(L, -1));
        return 0;
    }
    
    lua_close(L);
}
