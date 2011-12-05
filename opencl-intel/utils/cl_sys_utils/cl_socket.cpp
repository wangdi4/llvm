#include "cl_socket.h"
#include <sstream>
#include <vector>
#include <iostream>
#include <cstring>

using namespace std;


// Some definitions for portability
//
//-----------------------------------------------------------------------------
#ifdef _WIN32

#include <WinSock2.h>
#include <WS2tcpip.h>

// OS socket handle type
//
typedef SOCKET SOCKET_T;

#define CLOSESOCKET_FUNC ::closesocket

// This flag doesn't exist on Windows, so fake it
//
#define MSG_NOSIGNAL 0

//-----------------------------------------------------------------------------
#else // not _WIN32. LINUX

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

typedef int SOCKET_T;

#define SOCKET_ERROR    -1
#define INVALID_SOCKET  -1

#define CLOSESOCKET_FUNC ::close

#endif // _WIN32
//-----------------------------------------------------------------------------


// Get the last socket error from the OS
//
static string get_last_socket_error()
{
#ifdef _WIN32
    LONG last_error_code = WSAGetLastError();
    char* s_buf = 0;
    int len = FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                last_error_code,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPSTR) &s_buf,
                0,
                NULL);

    if (len == 0 || s_buf == 0) {
        stringstream numstr;
        numstr << last_error_code;
        return numstr.str();
    }

    // Save s_buf in a string and release it as required
    string s = s_buf;
    LocalFree(s_buf);

    // Strip trailing chars FormatMessage leaves behind
    size_t i = s.find_last_not_of("\n\r.");
    return s.substr(0, i + 1);
#else // not _WIN32
    return strerror(errno);
#endif // _WIN32
}


enum OclSocketState 
{
    OCLSOCKET_INVALID,
    OCLSOCKET_CREATED,
    OCLSOCKET_CLOSED
};


struct OclSocket::OclSocketImpl
{
    OclSocketImpl()
        : sock(INVALID_SOCKET), state(OCLSOCKET_INVALID)
    {
        memset(&sock_addr, 0, sizeof(sock_addr));
        recv_buf.resize(1024);
    }

    void system_error(const string& msg)
    {
        throw OclSocketError(msg + ": " + get_last_socket_error());
    }

    SOCKET_T sock;
    OclSocketState state;
    sockaddr_in sock_addr;
    vector<char> recv_buf;
};


// Used to encapsulate the actual socket descriptor type from the header file,
// leaving it opaque to the user who shouldn't really be aware of it.
//
struct OclSocket::OSSocketDescriptor
{
    OSSocketDescriptor(SOCKET_T sock_fd_)
        : sock_fd(sock_fd_)
    {    
    }

    SOCKET_T sock_fd;
};


OclSocket::OclSocket()
    : d(new OclSocketImpl())
{
    d->sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (d->sock == INVALID_SOCKET)
        d->system_error("failed socket()");
    d->state = OCLSOCKET_CREATED;

    int optval = 1;
    if (::setsockopt(d->sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval)) != 0)
        d->system_error("failed setsockopt()");
}


OclSocket::OclSocket(const OSSocketDescriptor& s)
    : d(new OclSocketImpl())
{
    d->sock = s.sock_fd;
    d->state = OCLSOCKET_CREATED;
}


OclSocket::~OclSocket()
{
    close();
}


void OclSocket::close()
{
    if (d->state != OCLSOCKET_CLOSED) {
        CLOSESOCKET_FUNC(d->sock);
        d->state = OCLSOCKET_CLOSED;
    }
}


void OclSocket::bind(unsigned short port)
{
    d->sock_addr.sin_family = AF_INET;
    d->sock_addr.sin_addr.s_addr = INADDR_ANY;
    d->sock_addr.sin_port = htons(port);

    if (::bind(d->sock, (struct sockaddr*) &d->sock_addr, sizeof(d->sock_addr)) != 0) 
        d->system_error("failed bind()");
}


void OclSocket::listen(int backlog)
{
    if (::listen(d->sock, backlog) != 0)
        d->system_error("failed listen()");
}


OclSocket* OclSocket::accept()
{
    SOCKET_T sock = ::accept(d->sock, 0, 0);
    if (sock == INVALID_SOCKET)
        d->system_error("failed accept()");
    return new OclSocket(OSSocketDescriptor(sock));
}


void OclSocket::connect(const string& host, unsigned short port)
{
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    stringstream port_str;
    port_str << port;

    int gai_status = ::getaddrinfo(host.c_str(), port_str.str().c_str(), &hints, &res);
    if (gai_status != 0) {
        // Not using system_error here, since getaddrinfo has its own
        // error formatting function.
        //
#ifdef _WIN32
        string gai_error_str = gai_strerrorA(gai_status);
#else
        string gai_error_str = "<?>";
#endif // _WIN32
        throw OclSocketError("failed getaddrinfo() in connect(): " + gai_error_str);
    }

    if (::connect(d->sock, res->ai_addr, static_cast<int>(res->ai_addrlen)) != 0)
        d->system_error("failed connect()");
}


size_t OclSocket::send(const vector<char>& buf)
{
    int ret = ::send(d->sock, &buf[0], static_cast<int>(buf.size()), MSG_NOSIGNAL);
    if (ret == SOCKET_ERROR)
        d->system_error("failed send()");
    return static_cast<size_t>(ret);
}


vector<char> OclSocket::recv()
{
    int ret = ::recv(d->sock, &d->recv_buf[0], static_cast<int>(d->recv_buf.size()), 0);
    if (ret == 0) {
        // Connection closed
        return vector<char>();
    }
    else if (ret == SOCKET_ERROR) {
        d->system_error("failed recv()");
        return vector<char>(); // compiler pacifier, since d->system_error throws
    }
    else {
        return d->recv_buf;
    }
}


vector<char> OclSocket::recv_n_bytes(size_t n)
{
    d->recv_buf.resize(n);
    size_t buf_ptr = 0;

    while (buf_ptr < n) {
        int ret = ::recv(d->sock, &d->recv_buf[buf_ptr], static_cast<int>(n - buf_ptr), 0);
        if (ret == 0) {
            // Connection closed - return what we have so far.
            d->recv_buf.resize(buf_ptr);
            return d->recv_buf;
        }
        else if (ret == SOCKET_ERROR) {
            d->system_error("failed recv()");
            return vector<char>(); // compiler pacifier, since d->system_error throws
        }
        else {
            buf_ptr += static_cast<size_t>(ret);
        }
    }
    return d->recv_buf;
}


void OclSocket::set_recv_buf_size(size_t bufsize)
{
    d->recv_buf.resize(bufsize);
}


// Takes care of the initialization and cleanup of the OS network module on 
// Windows.
//
#ifdef _WIN32
struct YNetworkInitializer
{
    YNetworkInitializer()
    {
        WSADATA data;
        WSAStartup(MAKEWORD(2, 2), &data);
    }

    ~YNetworkInitializer()
    {
        WSACleanup();
    }
};

static YNetworkInitializer ynetwork_initializer;
#endif // _WIN32
