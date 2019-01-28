/*************************************************************************
    > File Name: defines.h
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Mon Jan 28 12:06:55 2019
 ************************************************************************/

#include <stdio.h>

#ifdef __cplusplus
#define EXT_C extern "C"
#else
#define EXT_C
#endif

#define DECLARE_IN(name, ext)   ext int name##_register(lua_State *)
#define DECLARE_REGISTER(name)  DECLARE_IN(name, EXT_C)

#define DEFINE_IN(name, tab, ext)           \
ext int name##_register(lua_State * L) {    \
    luaL_newlib(L, tab);                    \
    lua_setglobal(L, #name);                \
    return 0;                               \
}
#define DEFINE_REGISTER(name, tab)   DEFINE_IN(name, tab, EXT_C)

#define DO_REGISTER(L, name) name##_register(L)
    
