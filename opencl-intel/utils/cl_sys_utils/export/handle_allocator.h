///////////////////////////////////////////////////////////
//  HandleAllocator.h
///////////////////////////////////////////////////////////
// Copyright (c) 2006-2009 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#pragma once

#include "cl_synch_objects.h"

#include<assert.h>
#include<list>

// Defines  class that manages allocation and reuse of handles
namespace Intel { namespace OpenCL { namespace Utils {

template<class _HandleType> class HandleAllocator
{
public:
	HandleAllocator(_HandleType minValue, _HandleType maxValue)
	{
		m_initValues.maxValue = maxValue;
		m_initValues.minValue = minValue;

		m_freeRanges.push_back(m_initValues);
	}

	virtual ~HandleAllocator()
	{
		// assert(m_freeRanges.size() == 1);
		m_freeRanges.clear();
	}

	// Allocates new handle from the empty pool
	// Returns:
	//		true	- new handle is available
	//		false	- nothing to allocate
	bool	AllocateHandle(_HandleType* newHandle)
	{
		OclAutoMutex mutex(&m_muRange);

		if ( m_freeRanges.empty() )
		{
			return false;
		}

		RangeEntry_t &top = m_freeRanges.front();

		*newHandle = top.minValue;
		++top.minValue;
		// Range is fully occupied
		if (top.minValue > top.maxValue)
		{
			m_freeRanges.pop_front();
		}

		return true;
	}

	// Returns unused handle to empty pool
	void	FreeHandle(_HandleType handle)
	{
		OclAutoMutex mutex(&m_muRange);

		// Create new range for empty list
		if ( m_freeRanges.empty() )
		{
			RangeEntry_t	newEntry;
			newEntry.minValue = newEntry.maxValue = handle;
			m_freeRanges.push_back(newEntry);
			return;
		}

		// Search the list and try to find appropriate place to store the free handle
		RangesList_t::iterator	it;
		for(it = m_freeRanges.begin(); it != m_freeRanges.end(); ++it)
		{
			_HandleType lowLimit = it->minValue - 1;
			_HandleType highLimit = it->maxValue + 1;

			// The handle is out of lower bound of the current range
			if ( handle < lowLimit )
			{
				RangeEntry_t	newEntry;
				newEntry.minValue = newEntry.maxValue = handle;
				m_freeRanges.insert(it, newEntry);
				break;
			}

			// The handle matches the low limit of the current range
			if ( handle == lowLimit )
			{
				--(it->minValue);
				break;
			}

			// The handle matches the high limit of the current range
			if ( handle == highLimit )
			{
				++(it->maxValue);
				break;
			}

			// The handle is out of high bound of the current range, check next range
		}

		// We checked all ranges but nothing is found
		if ( it == m_freeRanges.end() )
		{
			RangeEntry_t	newEntry;
			newEntry.minValue = newEntry.maxValue = handle;
			m_freeRanges.push_back(newEntry);
		}
		
		return;
	}

	// Clear all allocated handles and return allocator to its initial state
	void Clear()
	{
		OclAutoMutex mutex(&m_muRange);

		// Bug: TODO: check why this is not working.
        // assert(m_freeRanges.size() == 1);
		m_freeRanges.clear();
		m_freeRanges.push_back(m_initValues);
	}

protected:
	typedef struct _RangeEntry_t
	{
		_HandleType minValue;
		_HandleType maxValue;
	} RangeEntry_t;

	typedef std::list<_RangeEntry_t>	RangesList_t;
	RangesList_t				m_freeRanges;
	RangeEntry_t				m_initValues;

	OclMutex		m_muRange;
};

}}}