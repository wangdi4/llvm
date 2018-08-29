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

#pragma once

namespace Intel { namespace OpenCL { namespace Utils {
	//Local array is an implementation of an array of POD types that exists only within the current scope
	template<class T>
	class clLocalArray
	{
		clLocalArray(const clLocalArray&);
		clLocalArray& operator=(const clLocalArray&);
	public:
		clLocalArray(size_t length) : m_data(nullptr), m_heapAllocated(false)
		{
			if (length * sizeof(T) > m_maxSizeToAllocateOnStack)
			{
				m_heapAllocated = true;
			}
			if (m_heapAllocated)
			{
				m_data = new T[length];
			}
			else
			{
				m_data = reinterpret_cast<T*>(m_storage);
			}
		}
		~clLocalArray()
		{
			if (m_heapAllocated)
			{
				delete[] m_data;
			}
			m_data = nullptr;
		}
		operator    T*(void)      { return m_data;      }
        operator void*(void)      { return m_data;      }
		T**   operator&(void)     { return &m_data;     }
        T&    operator[](int inx) { return m_data[inx]; }

	protected:
		//It makes no sense to allocate this class on the heap
		void* operator new(size_t sz) {return nullptr;}
		//Probably a good idea not to allocate more than 1kb on the stack 
		static const size_t m_maxSizeToAllocateOnStack = 0x400; 

		T*   m_data;
		T    m_storage[m_maxSizeToAllocateOnStack / sizeof(T)];
		bool m_heapAllocated;

	};
}}}
