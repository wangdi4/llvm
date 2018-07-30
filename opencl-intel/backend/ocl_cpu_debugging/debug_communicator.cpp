/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2014 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related
// to the source code ("Material") are owned by Intel Corporation or its
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and
// proprietary and confidential information of Intel Corporation and its
// suppliers and licensors, and is protected by worldwide copyright and trade
// secret laws and treaty provisions. No part of the Material may be used, copied,
// reproduced, modified, published, uploaded, posted, transmitted, distributed,
// or disclosed in any way without Intel�s prior express written permission.
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery
// of the Materials, either expressly, by implication, inducement, estoppel or
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice
// or any other notice embedded in Materials by Intel or Intel�s suppliers or licensors
// in any way.
/////////////////////////////////////////////////////////////////////////

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
