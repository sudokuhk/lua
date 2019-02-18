/*************************************************************************
    > File Name: network.c
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Mon Jan 28 18:20:18 2019
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <defines.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define CHECK(index, type)  \
    if (lua_is##type(L, index) != 1) {      \
        sz_error = "invalid parameter for index #index, need #type\n"; \
        goto error; \
    }  

typedef struct socket_t
{
    int i_sock;
    
    char * p_host;
    int i_port;
    
    int i_capacity;
    int i_head, i_tail;
    int i_read, i_write;
    char * p_buffer;
    
    int i_linesize;
    char * p_line;
    
    int i_timeout;
} socket_t;

static int is_nonblock(int fd)
{
    if (fd < 0) {
        return 0;
    }
    
    int flag = fcntl(fd, F_GETFL, 0);
    return (flag | O_NONBLOCK) > 0 ? 1 : 0;
}

static int set_nonblock(int fd)
{
    if (fd < 0) {
        return 0;
    }
    
    int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);
    return 0;
}

static int connect_timeo(int sock, struct sockaddr * addr, int timeo/*s*/)
{
    fd_set rdset, wset;
    struct timeval to;
    int active = 0, error, len = sizeof(error);
    
    FD_ZERO(&rdset);
    FD_SET(sock, &rdset);
    wset = rdset;
    
    to.tv_sec = timeo;
    to.tv_usec = 0;
    
    if (connect(sock, addr, sizeof(*addr)) == 0) {
        printf("connect immediately\n");
        return 0;
    } else if (errno != EINPROGRESS) {
        printf("connect error!\n");
        return -1;
    }
    
    active = select(sock + 1, &rdset, &wset, NULL, &to);
    if (active < 0) {
        return -1;
    }
    
    if (FD_ISSET(sock, &rdset) || FD_ISSET(sock, &wset)) {
        error = 0;
        if(getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len) < 0
           || error != 0) {
            printf("connect error select\n");
            return -1;
        }
    }
    printf("connect succeed!\n");
    return 0;
}

static int sendn(int fd, void * data, int size)
{
    int nsend = 0;
    char * p = (char *)data;
    fd_set wset;
    struct timeval to;
    to.tv_sec = 10;
    to.tv_usec = 0;
    
    while (nsend < size) {
        FD_ZERO(&wset);
        FD_SET(fd, &wset);
        
        int active = select(fd + 1, NULL, &wset, NULL, &to);
        if (active < 0) {
            printf("send timeout\n");
            nsend = -1;
            break;
        }
        
        int once = send(fd, p, size - nsend, 0);
        if (once < 0 && errno != EAGAIN) {
            nsend = -1;
            break;
        }
        
        nsend += once;
        p += once;
    }
    return nsend;
}

static int recvn(int fd, void * data, int size, int completed)
{
    int nrecv = 0;
    char * p = (char *)data;
    fd_set rset;
    struct timeval to;
    to.tv_sec = 10;
    to.tv_usec = 0;
    
    while (nrecv < size) {
        FD_ZERO(&rset);
        FD_SET(fd, &rset);
        
        int active = select(fd + 1, &rset, NULL, NULL, &to);
        if (active < 0) {
            printf("recv timeout\n");
            nrecv = -1;
            break;
        }
        
        int once = recv(fd, p, size - nrecv, 0);
        if (once < 0 && errno != EAGAIN) {
            nrecv = -1;
            break;
        }
        
        nrecv += once;
        p += once;
        if (!completed) {
            break;
        }
    }
    return nrecv;
}

static int l_connect(lua_State * L)
{
    int fd = -1, port = 0, timeout = 60, res = -1;
    const char * sz_error = "no error", *sz_ip = NULL;
    char sz_port[10];
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    struct sockaddr addr;
    socket_t * psock = NULL;
    
    CHECK(1, string);   //ip
    CHECK(2, number);   //port
    CHECK(3, number);   //timeout seconds
    
    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (fd < 0) {
        sz_error = "create socket failed";
        goto error;
    }
    set_nonblock(fd);
    
    sz_ip = lua_tostring(L, 1);
    port = lua_tonumber(L, 2);
    timeout = lua_tonumber(L, 3);
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */
    
    sprintf(sz_port, "%d", port);
    res = getaddrinfo(sz_ip, sz_port, &hints, &result);
    if (res != 0) {
        sz_error = "getaddrinfo failed";
        goto error;
    }
    
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        if (rp->ai_family == AF_INET) { //always true.
            addr = *rp->ai_addr;
            break;
        }
    }
    freeaddrinfo(result);
    
    if (connect_timeo(fd, &addr, 10) < 0) {
        sz_error = "connect error\n";
        goto error;
    }
    
    printf("connect ok\n");
    psock = (socket_t *)malloc(sizeof(socket_t));
    memset(psock, 0, sizeof(socket_t));
    psock->i_sock = fd;
    psock->i_capacity = 4096;
    psock->p_buffer = (char *)malloc(psock->i_capacity + 1);
    psock->p_buffer[psock->i_capacity] = '\0';
    psock->i_linesize = 4096;
    psock->p_line = (char *)malloc(psock->i_linesize + 1);
    lua_pushlightuserdata(L, psock);
    return 1;
    
error:
    if (fd > 0) {
        close(fd);
    }
    lua_pushnil(L);
    return 1;
}

static int l_recv_data(lua_State * L)
{
    const char * sz_error = "no error";
    int nread = 0;
    int want = 0, n;
    socket_t * p_sock = NULL;
    
    CHECK(1, lightuserdata);
    if (lua_isuserdata(L, 2) != 1 && lua_islightuserdata(L, 2) != 1) {
        goto error;
    }
    CHECK(3, number);   //want
    p_sock = (socket_t *)lua_touserdata(L, 1);
    char * buf = lua_touserdata(L, 2);
    want = lua_tonumber(L, 3);

    n = p_sock->i_tail - p_sock->i_head;
    if (n > 0) {
        int npick = n > want ? want : n;
        memcpy(buf, p_sock->p_buffer + p_sock->i_head, npick);
        p_sock->i_head += npick;
        nread = npick;
    }
    if (want - nread > 0) {
        int nget = recvn(p_sock->i_sock, buf + n, want - n, 1);
        if (nget > 0) {
            nread += nget;
        } else {
            nread = -1;
        }
    }
error:
    //printf("recv_data:%d\n", nread);
    lua_pushnumber(L, nread);
    return 1;
}

static int l_recv_string(lua_State * L)
{
    const char * sz_error = "no error";
    int nread = 0, i = 0, n = 0, nstr = 0, tail = 0;
    socket_t * p_sock = NULL;
    
    CHECK(1, lightuserdata);
    p_sock = (socket_t *)lua_touserdata(L, 1);
    
    n = p_sock->i_tail - p_sock->i_head;
    if (n > 0) {
        for (i = 0; i < n; i++) {
            char c = p_sock->p_buffer[p_sock->i_head + i];
            if (c == '\n' || c == '\0') {
                tail = p_sock->i_head;
                goto got_line;
            }
        }
    } else {
        p_sock->i_tail = p_sock->i_head = 0;
    }
    
    if (p_sock->i_tail == p_sock->i_capacity) {
        memmove(p_sock->p_buffer, p_sock->p_buffer + p_sock->i_head, n);
    }
    
    while (1) {
        char * ptr = p_sock->p_buffer + p_sock->i_tail;
        int i_remain = p_sock->i_capacity - p_sock->i_tail;
        
        n = recvn(p_sock->i_sock, ptr, i_remain, 0);
        printf("recvn:%d\n", n);
        if (n < 0) {
            goto error;
        }
        
        for (i = 0; i < n; i++) {
            char c = p_sock->p_buffer[p_sock->i_tail + i];
            if (c == '\n' || c == '\0') {
                tail = p_sock->i_tail;
                goto got_line;
            }
        }
    }
    
got_line:    
    nstr = tail + i - p_sock->i_head + 1;
    memcpy(p_sock->p_line, 
           p_sock->p_buffer + p_sock->i_head,
           nstr);
    if (tail == p_sock->i_tail) {
        p_sock->i_tail += n;
    }
    p_sock->i_head += nstr;
    p_sock->p_line[nstr] = '\0';
    //printf("line:[%s]\n", p_sock->p_line);
    
    lua_pushstring(L, p_sock->p_line);
    return 1;
error:
    lua_pushnil(L);
    return 1;
}

static int l_send_data(lua_State * L)
{
    const char * sz_error = "no error";
    int nsend = 0;
    
    CHECK(1, lightuserdata);
    if (lua_isuserdata(L, 2) != 1 && lua_islightuserdata(L, 2) != 1) {
        goto error;
    }
    CHECK(3, number); 
    
    socket_t * p_sock = (socket_t * )lua_touserdata(L, 1);
    void * data = (void *)lua_touserdata(L, 2);
    int size = lua_tonumber(L, 3);
    
    if (p_sock->i_sock > 0) {
        nsend = sendn(p_sock->i_sock, data, size);
    }
    
error:
    //printf("send:%d\n", nsend);
    lua_pushnumber(L, nsend);
    return 1;
}

static int l_send_string(lua_State * L)
{
    const char * sz_error = "no error";
    int nsend = -1;
    
    CHECK(1, lightuserdata);
    CHECK(2, string); 
    
    socket_t * p_sock = (socket_t * )lua_touserdata(L, 1);
    const char * str = lua_tostring(L, 2);
    
    if (p_sock->i_sock > 0) {
        int len = strlen(str);
        if (len > 0) {
            nsend = sendn(p_sock->i_sock, (void *)str, strlen(str));
        } else {
            nsend = 0;
        }
    }
    
error:
    //printf("fd:%d, send:%d\n", fd, nsend);
    lua_pushnumber(L, nsend);
    return 1;
}

static int l_disconnect(lua_State * L)
{
    const char * sz_error = "no error";
    CHECK(1, lightuserdata);
    
    socket_t * p_sock = (socket_t * )lua_touserdata(L, 1);
    printf("disconnect:%d\n", p_sock->i_sock);
    if (p_sock->i_sock > 0) {
        close(p_sock->i_sock);
    }
    if (p_sock->p_buffer != NULL) {
        free(p_sock->p_buffer);
    }
    if (p_sock->p_line != NULL) {
        free(p_sock->p_line);
    }
    free(p_sock);
error:
    return 0;
}

const static luaL_Reg l_network[] = {
    {"connect", l_connect},    
    {"recv_data", l_recv_data},
    {"recv_string", l_recv_string},
    {"send_data", l_send_data},
    {"send_string", l_send_string},
    //{"checkconnect", l_checkconnect},
    {"disconnect", l_disconnect},
    {NULL, NULL},
};

DEFINE_REGISTER(cnetwork, l_network);
