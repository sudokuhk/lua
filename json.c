/*************************************************************************
    > File Name: json.c
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Mon Jan 28 14:26:17 2019
 ************************************************************************/

#include <defines.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <cJSON.h>
#include <limits.h>
#include <float.h>

#define ABS(n)  ((n) > 0 ? (n) : -(n))

static int build_table(lua_State * L, cJSON * root);

static void push_value(lua_State * L, cJSON * item)
{
    switch ((item->type) & 255) {
        case cJSON_NULL:
            lua_pushstring(L, "null");
            break;
        case cJSON_False:
            lua_pushstring(L, "false");
            break;
        case cJSON_True:
            lua_pushstring(L, "true");
            break;
        case cJSON_Number: {
            double d = item->valuedouble;
            int i = item->valueint;            
            if (item->valuedouble == 0) {
                lua_pushnumber(L, 0);
            } else if (ABS(((double)i) - d) <= DBL_EPSILON &&
                       d <= INT_MAX && d >= INT_MIN) {
                lua_pushnumber(L, i);
            } else {
                lua_pushnumber(L, d);
            }            
            break;
        }
        case cJSON_String:
            lua_pushstring(L, item->valuestring);
            break;
        case cJSON_Object:
            build_table(L, item);
            break;
    }
}

static void push_item(lua_State * L, cJSON * item)
{
    lua_pushstring(L, item->string);
    if (((item->type) & 255) == cJSON_Array) {
        int i, nsize = cJSON_GetArraySize(item);
        lua_newtable(L);
        
        for (i = 0; i < nsize; i++) {
            cJSON * array = cJSON_GetArrayItem(item, i);
            lua_pushnumber(L, i);
            push_value(L, array);
            lua_settable(L, -3);
        }
        
    } else {
        push_value(L, item);
    }
    lua_settable(L, -3);
}

static int build_table(lua_State * L, cJSON * root)
{
    int i, nsize = cJSON_GetArraySize(root);
    lua_newtable(L);
    for (i = 0; i < nsize; i++) {
        cJSON * item = cJSON_GetArrayItem(root, i);
        push_item(L, item);
    }
    return 0;
}

static int parse(lua_State * L)
{
    const char * sz_error = "no error", *sz_json;
    cJSON * root = NULL;
    
    if (lua_isstring(L, 1) != 1) { 
        sz_error = "invalid parameter for index 1, need string\n";
        goto error;
    }
    
    sz_json = lua_tostring(L, 1);
    if (sz_json == NULL) {
        sz_error = "invalid parameter for index 1, NULL string\n";
        goto error;
    }
    
    root = cJSON_Parse(sz_json);
    if (root == NULL) {
        sz_error = "parse json failed\n";
        goto error;
    }
    
    cJSON_Delete(root);
    lua_pushnumber(L, 0);
    build_table(L, root);
    lua_pushstring(L, sz_error);
    return 3;
    
error:
    lua_pushnumber(L, -1);
    lua_pushnil(L);
    lua_pushstring(L, sz_error);
    return 3;
}

const static luaL_Reg ljson[] = {
    {"parse", parse},
    {NULL, NULL},
};

DEFINE_REGISTER(cjson, ljson);
