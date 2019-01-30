/*************************************************************************
    > File Name: dir.c
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Wed Jan 30 12:23:14 2019
 ************************************************************************/

#include <defines.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static void push_item(lua_State * L, const char * name, struct stat * sb)
{
    lua_pushstring(L, name);
    lua_newtable(L);
    {
        lua_pushstring(L, "type");        
        const char * type = "unknown";
        switch (sb->st_mode & S_IFMT) {
            case S_IFIFO:   type = "fifo";      break;
            case S_IFCHR:   type = "character"; break;
            case S_IFDIR:   type = "directory"; break;
            case S_IFBLK:   type = "block";     break;
            case S_IFREG:   type = "regular";   break;
            case S_IFSOCK:  type = "socket";    break;
            case S_IFLNK:   type = "link";      break;
        }
        lua_pushstring(L, type);        
        lua_settable(L, -3);
        
        lua_pushstring(L, "size");
        lua_pushnumber(L, sb->st_size);
        lua_settable(L, -3);
    }
    lua_settable(L, -3);
}

static int l_list(lua_State * L)
{
    const char * dir = NULL;
    DIR * dp = NULL;
    struct dirent * entry = NULL;
    struct stat sb;
    
    if (lua_isstring(L, 1) != 1) {
        goto error;
    }
    
    dir = lua_tostring(L, 1);
    if ((dp = opendir(dir)) == NULL) {
        goto error;
    }
    
    lua_newtable(L);    
    while ((entry = readdir(dp)) != NULL) {
        lstat(entry->d_name, &sb);
        if (strcmp(entry->d_name, ".") == 0 || 
            strcmp(entry->d_name, "..") == 0 ) { 
            continue;
        }
        
        push_item(L, entry->d_name, &sb);
    }
    closedir(dp);
    return 1;
error:
    lua_pushnil(L);
    return 1;
}

const static luaL_Reg l_dir[] = {
    {"list", l_list},
    //{"create", l_new},
    //{"remove", l_remove},
    {NULL, NULL},
};

DEFINE_REGISTER(cdir, l_dir);
