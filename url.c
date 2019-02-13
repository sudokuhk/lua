/*************************************************************************
    > File Name: url.c
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Tue Jan 29 12:02:06 2019
 ************************************************************************/

#include <defines.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define SAFE_FREE(p)        \
    do {                    \
        if ((p) != NULL) {  \
            free(p);        \
            (p) = NULL;     \
        }                   \
    } while (0)

typedef struct url_t
{
    char * protocol;
    char * host;
    int port;
    char * path;
    char * userinfo;
    char * authority;
    char * file;
    char * query;
    char * ref;
} url_t;

static void release_url(url_t * url)
{
    SAFE_FREE(url->protocol);
    SAFE_FREE(url->host);
    SAFE_FREE(url->path);
    SAFE_FREE(url->userinfo);
    SAFE_FREE(url->authority);
    SAFE_FREE(url->file);
    SAFE_FREE(url->query);
    SAFE_FREE(url->ref);
}

static int is_string_empty(const char * sz_str)
{
    if (sz_str == NULL || strlen(sz_str) == 0) {
        return 1;
    }
    return 0;
}

static void copy_string(char ** dst, const char * src, int nsrc)
{
    if (*dst == NULL) {
        *dst = (char *)malloc(nsrc + 1);
    }
    memcpy(*dst, src, nsrc);
    (*dst)[nsrc] = '\0';
}

static int strnchr(const char * s, int n, char c) 
{
    const char * se = s + n, *p = s;
    while (p < se && *p != c) {
        p++;
    }
    return p == se ? -1 : p - s;
}

static int is_valid_protocol(const char * protocol, int nprotocol)
{
    int len = nprotocol, i;
    char c;
    
    if (len < 1) {
        return 0;
    }
    
    if (!isalpha(protocol[0])) {
        return 0;
    }
    
    for (i = 1, c = protocol[i]; i < len; i++) {
        if (!(isalpha(c) || isdigit(c)) && c != '.' &&
            c != '+' && c != '-') {
            return 0;
        }
    }
    
    return 1;
}

static int is_unc_name(const char * sz_url, int start, int limit) 
{
    if (start + 4 < limit) {
        if (sz_url[start + 0] == '/' && sz_url[start + 1] == '/' &&
            sz_url[start + 2] == '/' && sz_url[start + 3] == '/') {
            return 1;
        }
    }
    return 0;
}

static int is_https(const char * protocol) 
{
    if (protocol != NULL && strlen(protocol) == 5 && 
        strncmp(protocol, "https", 5) == 0) {
        return 1;
    }
    return 0;
}

static int do_parse_1(const char * sz_url, int start, int limit, url_t * url)
{
    int i = 0, ind = 0;
    
    if (start < limit) {
        //printf("start:%d, limit:%d, length:%d\n", start, limit, limit - start);
        ind = strnchr(sz_url + start, limit - start, '?');        
        if (ind >= 0 && (ind + start) < limit) { 
            ind += start;
            copy_string(&url->query, sz_url + ind + 1, limit - ind - 1);
            //printf("query:%s\n", url->query);
            limit = ind;
        }
    }
    
    if (!is_unc_name(sz_url, start, limit) &&
        (start + 2 <= limit) && sz_url[start] == '/' && sz_url[start + 1] == '/') {
        start += 2;
        //printf("%s\n", sz_url + start);
        
        i = strnchr(sz_url + start, limit - start, '/');
        if (i < 0 || (i + start) > limit) {
            i = strnchr(sz_url + start, limit - start, '?');
            i += start;
            if (i < 0 || i > limit) {
                i = limit;
            }
        } else {
            i += start;
        }
        
        if (start < i) {
            ind = strnchr(sz_url + start, i - start, ':');            
            if (ind >= 0) {
                ind += start;
                copy_string(&url->host, sz_url + start, ind - start);
                url->port = atoi(sz_url + ind + 1);
            } else {
                copy_string(&url->host, sz_url + start, i - start);
                url->port = is_https(url->protocol) ? 443 : 80;
            }
            //printf("host:%s, port:%d\n", url->host, url->port);
        }
    }
    
    if (i < limit) {
        if (sz_url[i] == '/') {
            copy_string(&url->path, sz_url + i, limit - i);
            //printf("path:%s\n", url->path);
        }
    }
    
    return is_string_empty(url->host) ? 0 : 1;
}

static int do_parse(const char * sz_url, url_t * url)
{
    int limit = strlen(sz_url), start = 0, i;
    char c;
    
    while (limit > 0 && sz_url[limit - 1] <= ' ') {
        limit --;   //eliminate trailing whitespace
    }
    
    while (start > limit && sz_url[start] <= ' ') {
        start ++;   //eliminate leading whitespace
    }
    
    if (start + 4 <= limit && memcmp(&sz_url[start], "url:", 4) == 0) {
        start += 4;
    }
    
    for (i = start; i < limit && (c = sz_url[i]) != '/'; i++) {
        if (c == ':') {
            int nprotocol = i - start;
            copy_string(&url->protocol, sz_url + start, nprotocol);
            
            if (is_valid_protocol(url->protocol, nprotocol) > 0) {
                start = i + 1;
            } else {
                return 0;
            }
            break;
        }
    }
    
    if (is_string_empty(url->protocol)) {
        return 0;
    }
    //printf("protocol:%s\n", url->protocol);
    
    i = strnchr(sz_url + start, limit - start, '#');
    if (i >= 0) {
        i += start;
        copy_string(&url->ref, sz_url + i + 1, limit - i - 1);
        
        limit = i;
        //printf("ref:%s\n", url->ref);
    }
    
    return do_parse_1(sz_url, start, limit, url);
}

static void push_string(lua_State * L, const char * key, const char * value)
{
    if (value != NULL) {
        lua_pushstring(L, key);
        lua_pushstring(L, value);
        lua_settable(L, -3);
    }
}

static void push_number(lua_State * L, const char * key, int value)
{
    lua_pushstring(L, key);
    lua_pushnumber(L, value);
    lua_settable(L, -3);
}

static void build_table(lua_State * L, const url_t * url)
{
    lua_newtable(L);
    push_string(L, "protocol", url->protocol);
    push_string(L, "host", url->host);
    push_number(L, "port", url->port);
    push_string(L, "path", url->path);
    push_string(L, "query", url->query);
    push_string(L, "ref", url->ref);
}

static int l_parse(lua_State * L)
{
    url_t url;
    const char * sz_url = NULL;
    
    if (lua_isstring(L, 1) != 1) {
        goto error;
    }
    
    sz_url = lua_tostring(L, 1);
    if (sz_url == NULL) {
        goto error;
    }
    
    //printf("url:%s\n", sz_url);
    memset(&url, 0, sizeof(url));
    if (do_parse(sz_url, &url) <= 0) {
        goto error;
    }
    
    build_table(L, &url);    
    release_url(&url);
    return 1;    
error:
    release_url(&url);
    lua_pushnil(L);
    return 1;
}

static luaL_Reg l_url[] = {
    {"parse", l_parse},
    {NULL, NULL},
};

DEFINE_REGISTER(curl, l_url);

