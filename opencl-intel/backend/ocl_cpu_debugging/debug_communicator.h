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
// or disclosed in any way without Intel’s prior express written permission.
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery
// of the Materials, either expressly, by implication, inducement, estoppel or
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors
// in any way.
/////////////////////////////////////////////////////////////////////////

#ifndef DEBUG_COMMUNICATOR_H
#define DEBUG_COMMUNICATOR_H

#include "cl_thread.h"
#include "cl_socket.h"
#include "cl_synch_objects.h"
#include "debugservermessages.pb.h"
#include <iostream>


using namespace Intel::OpenCL::Utils;
using namespace debugservermessages;


//#define DEBUG_SERVER_LOG_ON

inline void DEBUG_SERVER_LOG(const std::string& s)
{
#ifdef DEBUG_SERVER_LOG_ON
    std::cerr << "[DEBUG_SERVER] " << s << std::endl;
#endif // DEBUG_SERVER_LOG
}


class DebugCommunicator : private OclThread
{
public:
    DebugCommunicator(unsigned short port);
    ~DebugCommunicator();

    enum State
    {
        NO_CLIENT,
        CLIENT_CONNECTED,
        TERMINATED
    };

    // Wait until the communicator starts listening on the port
    //
    void waitForListen();

    // Wait for client connection
    //
    void waitForConnection();

    // Send a message to the client
    //
    void sendMessage(const ServerToClientMessage& msg);

    // Receive a message from the client - blocking
    //
    ClientToServerMessage receiveMessage();
   
protected:
    // These methods are executed inside a separate thread
    //
    virtual RETURN_TYPE_ENTRY_POINT Run();
    bool do_receive_message(ClientToServerMessage& recv_msg);
    bool do_send_message(const ServerToClientMessage& send_msg);
    void log_and_terminate(std::string msg);
    void set_state(State s);
private:
    State m_state;

    enum ThreadCommand
    {
        EXIT,
        SEND_MESSAGE,
        RECEIVE_MESSAGE
    };

    // Queue for passing commands to the thread
    // 
    typedef OclNaiveConcurrentQueue<ThreadCommand> ThreadCommandQueue;
    ThreadCommandQueue m_cmd_queue;

    // Queue for passing messages to send to the thread
    //
    typedef OclNaiveConcurrentQueue<ServerToClientMessage> MessageSendQueue;
    MessageSendQueue m_msg_send_queue;

    // Queue for passing received messages from the thread
    //
    typedef OclNaiveConcurrentQueue<ClientToServerMessage> MessageRecvQueue;
    MessageRecvQueue m_msg_recv_queue;

    unsigned short m_port;
    OclSocket m_server_socket;
    std::auto_ptr<OclSocket> m_connected_socket;

    // Lock for synchronizing between internal thread and external callers.
    //
    mutable OclMutex m_lock;

    // Event used to signal that a message was received
    //
    OclBinarySemaphore m_recv_event;

    // Event used to signal that the communicator started listening on the port
    //
    OclBinarySemaphore m_listen_event;

    // Event used to signal that a client has connected
    //
    OclBinarySemaphore m_connect_event;

    // Disallow copying
    //
    DebugCommunicator(const DebugCommunicator&);
    DebugCommunicator& operator=(const DebugCommunicator&);
};


#endif // DEBUG_COMMUNICATOR_H
