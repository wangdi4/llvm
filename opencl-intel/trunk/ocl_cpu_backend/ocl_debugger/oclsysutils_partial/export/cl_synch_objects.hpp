template<class T> inline
bool OclNaiveConcurrentQueue<T>::IsEmpty() const
{
	return m_queue.empty();
}

template<class T>
T OclNaiveConcurrentQueue<T>::Top()
{
	m_queueLock.Lock();
	assert(!IsEmpty());
	T ret = m_queue.front();
	m_queueLock.Unlock();
	return ret;
}

template<class T>
T OclNaiveConcurrentQueue<T>::PopFront()
{
	m_queueLock.Lock();
	assert(!IsEmpty());
	T ret = m_queue.front();
	m_queue.pop();
	m_queueLock.Unlock();
	return ret;
}

template<class T>
void OclNaiveConcurrentQueue<T>::PushBack(const T& newNode)
{
	m_queueLock.Lock();
	m_queue.push(newNode);
	m_queueLock.Unlock();
}
