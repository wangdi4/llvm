// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#include "oclEventsMapper.h"

#include <cassert>

using namespace Intel::OpenCL::Utils;

void NotifierEventsMapper::addEventPair(cl_event userEvent,
                                        cl_event notifierEvent) {
  std::lock_guard<std::mutex> M(m_lock);
  std::map<cl_event, cl_event>::iterator it;
  it = m_eventsMap.find(userEvent);
  assert(m_eventsMap.end() == it);
  m_eventsMap[userEvent] = notifierEvent;
}
cl_event NotifierEventsMapper::getNotifierEvent(cl_event userEvent) {
  std::lock_guard<std::mutex> M(m_lock);
  cl_event notifierEvent = nullptr;
  std::map<cl_event, cl_event>::iterator it;
  it = m_eventsMap.find(userEvent);
  if (m_eventsMap.end() != it) {
    notifierEvent = it->second;
  }
  return notifierEvent;
}
cl_event NotifierEventsMapper::getUserEvent(cl_event notifierEvent) {
  std::lock_guard<std::mutex> M(m_lock);
  cl_event userEvent = nullptr;
  std::map<cl_event, cl_event>::iterator it;
  for (it = m_eventsMap.begin(); m_eventsMap.end() != it; ++it) {
    if (it->second == notifierEvent) {
      userEvent = it->first;
      break;
    }
  }
  return userEvent;
}
void NotifierEventsMapper::delEvent(cl_event userEvent) {
  std::lock_guard<std::mutex> M(m_lock);
  std::map<cl_event, cl_event>::iterator it;
  it = m_eventsMap.find(userEvent);
  assert(m_eventsMap.end() != it);
  m_eventsMap.erase(it);
}
