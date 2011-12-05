#ifndef OCLSOCKET_H
#define OCLSOCKET_H


#include <stdexcept>
#include <vector>
#include <memory>


// Various methods of OclSocket throw OclSocketError in case of errors with the 
// socket APIs (where the low-level OS APIs return error codes).
//
class OclSocketError : public std::runtime_error
{
public:
    OclSocketError(const std::string msg)
        : std::runtime_error(msg)
    {}
};


// Low level TCP/IP socket class.
// 
// For servers:
// * create OclSocket -> bind -> listen -> accept()
// * send/recv on new sockets returned by accept()
//
// For clients:
// * create OclSocket -> connect() to server
// * send/recv 
//
// Notes: 
// * All network operations are blocking.
// * Only IPv4 addresses are supported
//
class OclSocket
{
public:
    OclSocket();
    ~OclSocket();

    // Server socket initialization
    //
    void bind(unsigned short port);
    void listen(int backlog = 5);

    // Close the socket manually.
    // Note that if you don't do it, the destructor eventually will.
    //
    void close();

    // Accept a new connection on a listening socket. Return a pointer to a new
    // OclSocket, initialized and ready to communicate.
    // The caller should delete the new object when it's no longer needed.
    // 
    OclSocket* accept();

    // Client socket initialization
    //
    void connect(const std::string& host, unsigned short port);

    // Send the given buffer. Return the amount of bytes actually sent (due to
    // the way sockets work, this may be less than the size of the buffer).
    //
    std::size_t send(const std::vector<char>& buf);

    // Receive data and return it. To set the amount of data you ask
    // to receive, call set_recv_buf_size(). The default amount is 1024 bytes.
    // The actual amount received may be less than requested. 
    // An empty vector is returned if the peer has shut down the connection.
    //
    std::vector<char> recv();

    // Receive exactly n bytes from the socket. Will not return until n bytes
    // have been received, unless the connection has been closed (in which case
    // less than n bytes may be returned).
    //
    std::vector<char> recv_n_bytes(std::size_t n);

    // Set the size of the receive buffer used in recv() calls
    //
    void set_recv_buf_size(std::size_t bufsize);

private:
    struct OSSocketDescriptor;
    OclSocket(const OSSocketDescriptor& sock_fd);

    struct OclSocketImpl;
    std::auto_ptr<OclSocketImpl> d;
private:
    // Disallow copying
    OclSocket(const OclSocket&);
    OclSocket& operator=(const OclSocket&);
};


#endif // OCLSOCKET_H

