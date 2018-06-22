// INTEL CONFIDENTIAL
//
// Copyright 2007-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "debug_communicator.h"
#include <cstdio>
#include "protobufpackedmessage.h"
#include <iostream>

using namespace std;

char const* localhost = "127.0.0.1";

#include <windows.h>
void sleep_ms(int ms)
{
    SleepEx(ms, TRUE);
}


DebugCommunicator::DebugCommunicator(unsigned short port)
    : m_state(NO_CLIENT), m_port(port)
{
    // Start the internal thread (executed in the Run method)
    //
    Start();
}


DebugCommunicator::~DebugCommunicator()
{
    // Politely ask the thread to exit
    m_cmd_queue.PushBack(EXIT);
    Join();
}


RETURN_TYPE_ENTRY_POINT DebugCommunicator::Run()
{
    DEBUG_SERVER_LOG("DebugCommunicator thread created");
    try
    {
        // Bind the server socket to localhost:port and wait for a client connection.
        // Once a client has connected, we can proceed to processing commands
        // in the communication queue.
        //
        m_server_socket.bind(localhost, m_port);
        m_server_socket.listen();
        m_listen_event.Signal();
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
                    return (RETURN_TYPE_ENTRY_POINT)0;
                case SEND_MESSAGE:
                {
                    DEBUG_SERVER_LOG("executing SEND_MESSAGE");
                    assert(!m_msg_send_queue.IsEmpty());
                    ServerToClientMessage msg_to_send = m_msg_send_queue.PopFront();
                    if (!do_send_message(msg_to_send)) {
                        log_and_terminate("Send error -- exiting");
                        return (RETURN_TYPE_ENTRY_POINT)-1;
                    }
                    break;
                }
                case RECEIVE_MESSAGE:
                {
                    ClientToServerMessage recv_msg;
                    if (do_receive_message(recv_msg) == false) {
                        log_and_terminate("Receive error -- exiting");
                        return (RETURN_TYPE_ENTRY_POINT)-1;
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
        return (RETURN_TYPE_ENTRY_POINT)-1;
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


void DebugCommunicator::waitForListen()
{
    m_listen_event.Wait();
}
