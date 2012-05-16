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
	T ret = m_queue.front();

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
