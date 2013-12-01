/////////////////////////////////////////////////////////////////////////
// cl_synch_objects.hpp
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2013 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related
// to the source code ("Material") are owned by Intel Corporation or its
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and
// proprietary and confidential information of Intel Corporation and its
// suppliers and licensors, and is protected by worldwide copyright and trade
// secret laws and treaty provisions. No part of the Material may be used, copied,
// reproduced, modified, published, uploaded, posted, transmitted, distributed,
// or disclosed in any way without Intel?s prior express written permission.
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery
// of the Materials, either expressly, by implication, inducement, estoppel or
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice
// or any other notice embedded in Materials by Intel or Intel?s suppliers or licensors
// in any way.
/////////////////////////////////////////////////////////////////////////

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
