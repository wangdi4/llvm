#include <cassert>
#include "oclEventsMapper.h"

using namespace Intel::OpenCL::Utils;

void NotifierEventsMapper::addEventPair(cl_event userEvent, cl_event notifierEvent)
{
    OclAutoMutex M(&m_lock);
    map<cl_event, cl_event>::iterator it;
    it = m_eventsMap.find(userEvent);
    assert(m_eventsMap.end() == it);
    m_eventsMap[userEvent] = notifierEvent;
}
cl_event NotifierEventsMapper::getNotifierEvent(cl_event userEvent)
{
    OclAutoMutex M(&m_lock);
    cl_event notifierEvent = nullptr;
    map<cl_event, cl_event>::iterator it;
    it = m_eventsMap.find(userEvent);
    if (m_eventsMap.end() != it)
    {
        notifierEvent = it->second;
    }
    return notifierEvent;
}
cl_event NotifierEventsMapper::getUserEvent(cl_event notifierEvent)
{
    OclAutoMutex M(&m_lock);
    cl_event userEvent = nullptr;
    map<cl_event, cl_event>::iterator it;
    for (it = m_eventsMap.begin(); m_eventsMap.end() != it; ++it)
    {
        if (it->second == notifierEvent)
        {
            userEvent = it->first;
            break;
        }
    }
    return userEvent;
}
void NotifierEventsMapper::delEvent(cl_event userEvent)
{
    OclAutoMutex M(&m_lock);
    map<cl_event, cl_event>::iterator it;
    it = m_eventsMap.find(userEvent);
    assert(m_eventsMap.end() != it);
    m_eventsMap.erase(it);
}
