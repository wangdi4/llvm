/////////////////////////////////////////////////////////////////////////
// cl_local_array.h
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2008 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related 
// to the source code ("Material") are owned by Intel Corporation or its 
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and 
// proprietary and confidential information of Intel Corporation and its 
// suppliers and licensors, and is protected by worldwide copyright and trade 
// secret laws and treaty provisions. No part of the Material may be used, copied, 
// reproduced, modified, published, uploaded, posted, transmitted, distributed, 
// or disclosed in any way without Intel’s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////
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
