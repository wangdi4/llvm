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

#ifdef __USE_TBB_CONCURENT_QUEUE
/////////////////////////////////////////////////////////////////////////////
// OclConcurrentQueue
/////////////////////////////////////////////////////////////////////////////
template<class T> inline
bool OclConcurrentQueue<T>::IsEmpty() const
{
    return m_queue.empty();
}

template<class T>
T OclConcurrentQueue<T>::Top()
{
    TTypeConcurrentQueueConstIterator iter = m_queue.unsafe_begin();
    return *iter;
    //Todo: the above will throw an exception if the queue is empty. Maybe assert on it?
}

template<class T>
T OclConcurrentQueue<T>::PopFront()
{
    T val;
    if (m_queue.try_pop(val))
    {
        return val;
    }
    //else? Todo: figure this out
    assert(0);
    return val;
}

template<class T>
void OclConcurrentQueue<T>::PushBack(const T& newNode)
{
    m_queue.push(newNode);    
}

template<class T>
bool OclConcurrentQueue<T>::TryPop(T& val)
{
    return m_queue.try_pop(val);
}
#endif // __USE_TBB_CONCURENT_QUEUE

/////////////////////////////////////////////////////////////////////////////
// OclNaiveConcurrentQueue
/////////////////////////////////////////////////////////////////////////////
template<class T> inline
bool OclNaiveConcurrentQueue<T>::IsEmpty() const
{
    return m_queue.empty();
}

template<class T>
T OclNaiveConcurrentQueue<T>::Top()
{
    OclAutoMutex mu(&m_queueLock);

    assert(!IsEmpty());
    T& ret = m_queue.front();

    return ret;
}

template<class T>
T OclNaiveConcurrentQueue<T>::PopFront()
{
    OclAutoMutex mu(&m_queueLock);

    assert(!IsEmpty());
    T ret = m_queue.front();
    m_queue.pop();

    return ret;
}

template<class T>
void OclNaiveConcurrentQueue<T>::PushBack(const T& newNode)
{
    OclAutoMutex mu(&m_queueLock);

    m_queue.push(newNode);
}

template<class T>
bool OclNaiveConcurrentQueue<T>::TryPop(T& val)
{
    OclAutoMutex mu(&m_queueLock);

    if ( m_queue.empty() )
    {
        return false;
    }

    val = m_queue.front();
    m_queue.pop();

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// OclNaiveConcurrentMap
/////////////////////////////////////////////////////////////////////////////
template<class T, class S> inline
bool OclNaiveConcurrentMap<T,S>::IsEmpty()
{
    OclAutoMutex mu(&m_mapLock);

    return IsEmptyInternal();
}

template<class T, class S>
void OclNaiveConcurrentMap<T,S>::Insert(const T& key, const S& val)
{
    OclAutoMutex mu(&m_mapLock);

    m_map[key] = val;
}

template<class T, class S>
S OclNaiveConcurrentMap<T,S>::Find(const T& key)
{
    OclAutoMutex mu(&m_mapLock);

    assert(!IsEmptyInternal());
    typename std::map<T,S>::const_iterator it = m_map.find(key);
    assert(it != m_map.end());
    S val = it->second;
    return val;
}

template<class T, class S>
void OclNaiveConcurrentMap<T,S>::Erase(const T& key)
{
    OclAutoMutex mu(&m_mapLock);

    assert(!IsEmptyInternal());
    m_map.erase(key);
}

template<class T, class S>
bool OclNaiveConcurrentMap<T,S>::IsFound(const T& key, S& val)
{
    OclAutoMutex mu(&m_mapLock);

    if (IsEmptyInternal())
    {
        return false;
    }
	
    typename std::map<T,S>::iterator it = m_map.find(key);
    if (it == m_map.end())
    {
        return false;
    }
	
    val = it->second;
    return true;
}

template<class T, class S>
void OclNaiveConcurrentMap<T,S>::Clear()
{
    OclAutoMutex mu(&m_mapLock);

    m_map.clear();
}

template<class T, class S>
bool OclNaiveConcurrentMap<T,S>::IsEmptyInternal() const
{
    return m_map.empty();
}
