#pragma once
#include <sys/queue.h>
#include <stddef.h>

namespace Intel { namespace OpenCL { namespace MicDevice {

template <class T>
class OclListIterator;

/* Class that represent generic link list.
   The client can enter to the list any parameter type. 
   The implementation is wrapper to sys/queue implementation of Linux. 
   It is NOT thread safe implementation. */
template <class T>
class OclList
{
	
public:
	
	/* Link definition, contain parameter of type T and nodes which represent link in sys/queue implementation. (nodes is private and does not expose to the client)*/
	struct Node
	{
		// Those class can enter this struct privatemember
	    friend class OclList;
		friend class OclListIterator<T>;
		
		public:
			T data;
		
		private:

			Node() {};
			Node(const T& d) : data(d) {};

			LIST_ENTRY(OclList::Node) nodes;
	};
	
	/* Constructor */
	OclList();

	/* Destructor */
	virtual ~OclList();

	/* Create new Node element (by malloc) that its data values is according to the client input.
	   Push the new node to the front of the list and return the created node pointer to the client.
	   T - The content of the node. */
	OclList::Node* pushFront(T& data);

	/* Create new Node element (by malloc) that its data values is according to the client input.
	   Push the new node to the back of the list and return the created node pointer to the client.
	   T - The content of the node. */
	OclList::Node* pushBack(T& data);

	/* Remove specific node from the list and free the node.
	   node - node to remove.
	   The "OclListIterator" iterators that are not pointing this specific node are not demaged. */
	void removeNode(OclList::Node* node);

	/* Remove the node currently pointed by iter and free the node.
	   After this operation the current iterator point to the successor node, It is the client responsibility to check that iter is not pointing list.end() 
	   before performing any operation with iter.
	   iter - list iterator. */
	void removeNode(OclListIterator<T>& iter);

	/* Clear the list and free all nodes memory. */
	void clear();

	/* Return true if the list is empty */
	bool isEmpty();

	/* Return OclListIterator that point to the head of the list */
	OclListIterator<T> begin();

	/* Return OclListIterator that point to the end of the list */
	OclListIterator<T> end();

private:

	LIST_HEAD(listhead, OclList::Node) m_head;

	// Need it in order to implement push_back
	Node m_lastDummyNode;
};

/* Class that represent iterator to OclList.
   It is NOT thread safe implementation. */
template <class T>
class OclListIterator
{

	// OclList can access private members of this class
    friend class OclList<T>;

public:

	typedef typename OclList<T>::Node NodeType;

	/* Default constructor */
	OclListIterator();

	/* advance the iterator to the next node */
	void operator++ (void);

	/* Return the current node pointer that pointing by the iterator */
	const T& operator* (void) const;
	T& operator* (void);

	/* Compare between positions of two iterators */
	int operator== (const OclListIterator& other) const;

	/* Compare between positions of two iterators */
	int operator!= (const OclListIterator& other) const;

private:

	/* Constructor that set the node that is pointed by the iterator */
	OclListIterator(NodeType* headNode);
#ifdef _DEBUG
	/* Constructor that set the node that is pointed by the iterator */
	OclListIterator(NodeType* headNode, NodeType* endNode);
#endif

	/* The current node that is pointing by the iterator */
	NodeType* m_position;
#ifdef _DEBUG
	/* The node that represent the end of the list (for debug only)*/
	NodeType* m_end;
#endif

};

}}}

#include "ocl_list.hpp"
