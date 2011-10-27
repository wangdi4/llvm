#include "debug_communicator.h"
#include <cstdio>
#include "protobufpackedmessage.h"
#include <iostream>

using namespace std;


#ifdef WIN32
#include <windows.h>
void sleep_ms(int ms)
{
    SleepEx(ms, TRUE);
}
#else
#include <unistd.h>
void sleep_ms(int ms)
{
    usleep(1000 * ms);
}
#endif // WIN32


DebugCommunicator::DebugCommunicator(unsigned short port)
    : m_state(NO_CLIENT), m_port(port)
{
    m_recv_event.Init(true);  // autoreset event
    m_connect_event.Init(true);

    // Start the internal thread (executed in the Run method)
    //
    Start();
}


DebugCommunicator::~DebugCommunicator()
{
    // Nothing to do, all members have useful destructors
}


int DebugCommunicator::Run()
{
    DEBUG_SERVER_LOG("DebugCommunicator thread created");
    try 
    {
        // Bind the server socket to its port and wait for a client connection.
        // Once a client has connected, we can proceed to processing commands
        // in the communication queue.
        // 
        m_server_socket.bind(m_port);
        m_server_socket.listen();
        m_connected_socket = auto_ptr<OclSocket>(m_server_socket.accept());
        set_state(CLIENT_CONNECTED);
        m_connect_event.Signal();
        DEBUG_SERVER_LOG("Client connected");

        // Take commands off the command queue and execute them
        //
        while (true) {
            while (m_cmd_queue.IsEmpty()) {
                sleep_ms(50);
            }
            ThreadCommand cmd = m_cmd_queue.PopFront();

            switch (cmd) {
                case EXIT:
                    log_and_terminate("executing EXIT");
                    return 0;
                case SEND_MESSAGE: 
                {
                    DEBUG_SERVER_LOG("executing SEND_MESSAGE");
                    assert(!m_msg_send_queue.IsEmpty());
                    ServerToClientMessage msg_to_send = m_msg_send_queue.PopFront();
                    if (!do_send_message(msg_to_send)) {
                        log_and_terminate("Send error -- exiting");
                        return -1;
                    }
                    break;
                }
                case RECEIVE_MESSAGE:
                {
                    ClientToServerMessage recv_msg;
                    if (do_receive_message(recv_msg) == false) {
                        log_and_terminate("Receive error -- exiting");
                        return -1;
                    }
                    m_msg_recv_queue.PushBack(recv_msg);
                    m_recv_event.Signal();
                    break;
                }
                default:
                    assert(0);
            }
        }
    }
    catch (const OclSocketError& e)
    {
        log_and_terminate(string("Socket error: ") + e.what());
        return -1;
    }
}


bool DebugCommunicator::do_send_message(const ServerToClientMessage& send_msg)
{
    vector<char> buf;
    if (!ProtobufPackedMessage::pack(buf, &send_msg)) {
        log_and_terminate("invalid message sent");
        return false;
    }
    m_connected_socket->send(buf);
    return true;
}


bool DebugCommunicator::do_receive_message(ClientToServerMessage& recv_msg)
{
    DEBUG_SERVER_LOG("executing RECEIVE_MESSAGE");
    unsigned header_size = ProtobufPackedMessage::HEADER_SIZE;
    vector<char> header = m_connected_socket->recv_n_bytes(header_size);
    
    if (header.size() < header_size) {
        log_and_terminate("connection closed undexpectedly in receive header");
        return false;
    }

    unsigned message_size = ProtobufPackedMessage::decode_header(header);
    vector<char> msgbuf = m_connected_socket->recv_n_bytes(message_size);

    if (msgbuf.size() < message_size) {
        log_and_terminate("connection closed undexpectedly in receive message");
        return false;
    }

    if (!ProtobufPackedMessage::unpack(&recv_msg, msgbuf)) {
        log_and_terminate("invalid message received");
        return false;
    }

    return true;
}


void DebugCommunicator::log_and_terminate(string msg)
{
    DEBUG_SERVER_LOG(msg);
    set_state(TERMINATED);
}


void DebugCommunicator::set_state(State s)
{
    OclAutoMutex M(&m_lock);
    m_state = s;
}


void DebugCommunicator::sendMessage(const ServerToClientMessage& msg)
{
    m_msg_send_queue.PushBack(msg);
    m_cmd_queue.PushBack(SEND_MESSAGE);
}


ClientToServerMessage DebugCommunicator::receiveMessage()
{
    m_cmd_queue.PushBack(RECEIVE_MESSAGE);
    
    // wait until the message arrives - this event will be signaled by the
    // thread
    //
    m_recv_event.Wait();

    return m_msg_recv_queue.PopFront();
}


void DebugCommunicator::waitForConnection()
{
    m_connect_event.Wait();
}
