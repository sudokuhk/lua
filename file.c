/*************************************************************************
    > File Name: file.c
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Wed Jan 30 09:42:08 2019
 ************************************************************************/

#include <defines.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

static int l_open(lua_State * L)
{
    if (lua_isstring(L, 1) != 1) {
        goto error;
    }
    
    if (lua_isstring(L, 2) != 1) {
        goto error;
    }
    
    const char * filename = lua_tostring(L, 1);
    const char * mode = lua_tostring(L, 2);
    
    FILE * fp = fopen(filename, mode);
    if (fp == NULL) {
        goto error;
    }
    
    lua_pushlightuserdata(L, fp);
    return 1;
error:
    lua_pushnil(L);
    return 1;
}

static int l_read_data(lua_State * L)
{
    int n = -1;
    if (lua_islightuserdata(L, 1) != 1 || (lua_islightuserdata(L, 2) != 1) ||
        (lua_isnumber(L, 3) != 1)) {
        goto error;
    }
    
    FILE * fp = (FILE *)lua_touserdata(L, 1);
    void * buf = (void *)lua_touserdata(L, 2);
    int size = lua_tonumber(L, 3);
    
    n = fread(buf, size, 1, fp);
    
error:
    lua_pushnumber(L, n);
    return 1;
}

static int l_read_string(lua_State * L)
{
    lua_pushnumber(L, 0);
    return 1;
}

static int l_write_data(lua_State * L)
{
    int n = -1;
    if (lua_islightuserdata(L, 1) != 1 || (lua_islightuserdata(L, 2) != 1) ||
        (lua_isnumber(L, 3) != 1)) {
        goto error;
    }
    
    FILE * fp = (FILE *)lua_touserdata(L, 1);
    void * buf = (void *)lua_touserdata(L, 2);
    int size = lua_tonumber(L, 3);
    //printf("fp:%p, buf:%p, size:%d\n", fp, buf, size);
    n = fwrite(buf, size, 1, fp);
    
error:
    lua_pushnumber(L, n);
    return 1;
}

static int l_write_string(lua_State * L)
{
    lua_pushnumber(L, 0);
    return 1;
}

static int l_seek(lua_State * L)
{
    return 0;
}

static int l_close(lua_State * L)
{
    if (lua_islightuserdata(L, 1) == 1) {
        FILE * fp = lua_touserdata(L, 1);
        if (fp != NULL) {
            fclose(fp);            
        }
    }
    return 0;
}

const static luaL_Reg l_file[] = {
    {"open", l_open},
    {"read_data", l_read_data},
    {"read_string", l_read_string},
    {"write_data", l_write_data},
    {"write_string", l_write_string},
    {"seek", l_seek},
    {"close", l_close},
    {NULL, NULL},
};

DEFINE_REGISTER(cfile, l_file);
