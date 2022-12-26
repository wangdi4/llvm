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

#ifndef DEBUG_COMMUNICATOR_H
#define DEBUG_COMMUNICATOR_H

#include "cl_socket.h"
#include "cl_synch_objects.h"
#include "cl_thread.h"
#include "debugservermessages_wrapper.h"
#include <iostream>

using namespace Intel::OpenCL::Utils;
using namespace debugservermessages;

// #define DEBUG_SERVER_LOG_ON

inline void DEBUG_SERVER_LOG(const std::string &s) {
#ifdef DEBUG_SERVER_LOG_ON
  std::cerr << "[DEBUG_SERVER] " << s << std::endl;
#endif // DEBUG_SERVER_LOG
}

class DebugCommunicator : private OclThread {
public:
  DebugCommunicator(unsigned short port);
  ~DebugCommunicator();

  DebugCommunicator(const DebugCommunicator &) = delete;
  DebugCommunicator &operator=(const DebugCommunicator &) = delete;

  enum State { NO_CLIENT, CLIENT_CONNECTED, TERMINATED };

  // Wait until the communicator starts listening on the port
  //
  void waitForListen();

  // Wait for client connection
  //
  void waitForConnection();

  // Send a message to the client
  //
  void sendMessage(const ServerToClientMessage &msg);

  // Receive a message from the client - blocking
  //
  ClientToServerMessage receiveMessage();

protected:
  // These methods are executed inside a separate thread
  //
  virtual RETURN_TYPE_ENTRY_POINT Run() override;
  bool do_receive_message(ClientToServerMessage &recv_msg);
  bool do_send_message(const ServerToClientMessage &send_msg);
  void log_and_terminate(std::string msg);
  void set_state(State s);

private:
  State m_state;

  enum ThreadCommand { EXIT, SEND_MESSAGE, RECEIVE_MESSAGE };

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
  std::unique_ptr<OclSocket> m_connected_socket;

  // Lock for synchronizing between internal thread and external callers.
  //
  mutable std::mutex m_lock;

  // Event used to signal that a message was received
  //
  OclBinarySemaphore m_recv_event;

  // Event used to signal that the communicator started listening on the port
  //
  OclBinarySemaphore m_listen_event;

  // Event used to signal that a client has connected
  //
  OclBinarySemaphore m_connect_event;
};

#endif // DEBUG_COMMUNICATOR_H
