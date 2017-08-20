#include <malloc.h>
#include <assert.h>

using namespace Intel::OpenCL::MicDevice;

template <class T>
OclList<T>::OclList()
{
	// initialize the list
	LIST_INIT(&m_head);
	// insert the new dummy node to the head of the list
    LIST_INSERT_HEAD(&m_head, &m_lastDummyNode, nodes);
}

template <class T>
OclList<T>::~OclList()
{
	// remove all elements from the list and free the nodes memory.
	clear();
	// remove the dummy node
	LIST_REMOVE(LIST_FIRST(&m_head), nodes);
}

template <class T>
typename OclList<T>::Node* OclList<T>::pushFront(T& data)
{
	Node* node = new Node(data);
	assert(node && "malloc operation failed");
	// insert the new node to the head of the list
    LIST_INSERT_HEAD(&m_head, node, nodes);
	return node;
}

template <class T>
typename OclList<T>::Node* OclList<T>::pushBack(T& data)
{
	Node* node = new Node(data);
	assert(node && "malloc operation failed");
	// insert the new node to the head of the list
     LIST_INSERT_BEFORE(&m_lastDummyNode, node, nodes);
	return node;
}

template <class T>
void OclList<T>::removeNode(OclList<T>::Node* node)
{
	assert(node && "Null pointer (node)");
	assert(node != &m_lastDummyNode && "Trying to remove the dummy node");
	// remove node from the list
	LIST_REMOVE(node, nodes);
	// free the allocated memory of this node.
	delete(node);
}

template <class T>
void OclList<T>::removeNode(OclListIterator<T>& iter)
{
	// get the pointer to the node that iter is currently point.
	OclList<T>::Node* node = iter.m_position;
	// advance iter to the next node
	++iter;
	removeNode(node);
}

template <class T>
void OclList<T>::clear()
{
	// get the list head node.
	OclList<T>::Node* node = LIST_FIRST(&m_head);
	while (node != &m_lastDummyNode)
	{
		// remove the node from the list
		LIST_REMOVE(node, nodes);
		// free the node memory
		delete(node);
		node = LIST_FIRST(&m_head);
	}
}

template <class T>
bool OclList<T>::isEmpty()
{
	// check if the list is empty
	return LIST_FIRST(&m_head) == &m_lastDummyNode;
}

template <class T>
OclListIterator<T> OclList<T>::begin()
{
#ifdef _DEBUG
	return OclListIterator<T>(LIST_FIRST(&m_head), &m_lastDummyNode);
#endif
	return OclListIterator<T>(LIST_FIRST(&m_head));
}

template <class T>
OclListIterator<T> OclList<T>::end()
{
#ifdef _DEBUG
	return OclListIterator<T>(&m_lastDummyNode, &m_lastDummyNode);
#endif
	return OclListIterator<T>(&m_lastDummyNode);
}



template <class T>
OclListIterator<T>::OclListIterator() 
{
	m_position = nullptr;
#ifdef _DEBUG
	m_end = nullptr;
#endif
}

template <class T>
OclListIterator<T>::OclListIterator(NodeType* headNode)
{
	m_position = headNode;
}

#ifdef  _DEBUG
template <class T>
OclListIterator<T>::OclListIterator(NodeType* headNode, NodeType* endNode)
{
	m_position = headNode;
	m_end = endNode;
}
#endif

template <class T>
void OclListIterator<T>::operator++()
{
#ifdef  _DEBUG
	assert(m_position != m_end && "m_position == NULL");
#endif
	// advance to the next node in the list
	m_position = LIST_NEXT(m_position, nodes);
}

template <class T>
const T& OclListIterator<T>::operator*() const
{
#ifdef  _DEBUG
	assert(m_position != m_end && "m_position == NULL");
#endif
	return m_position->data;
}

template <class T>
T& OclListIterator<T>::operator*()
{
#ifdef  _DEBUG
	assert(m_position != m_end && "m_position == NULL");
#endif
	return m_position->data;
}

template <class T>
int OclListIterator<T>::operator==(const OclListIterator<T>& other) const
{
	return m_position == other.m_position;
}

template <class T>
int OclListIterator<T>::operator!= (const OclListIterator& other) const
{
	return m_position != other.m_position;
}
