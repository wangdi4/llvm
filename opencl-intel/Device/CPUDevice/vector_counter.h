#pragma once

// Copyright (c) 2006-2007 Intel Corporation
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

///////////////////////////////////////////////////////////
//  vector_counter.h
///////////////////////////////////////////////////////////


namespace Intel { namespace OpenCL {

template<class _CountType> class VectorCounter
{
public:
	VectorCounter(int iSize, _CountType* _Inits, _CountType* _Limits)
	{
		m_iSize = iSize;
		m_Value = new _CountType[iSize];
		m_Limits = new _CountType[iSize];
		m_Init = new _CountType[iSize];
		
		memcpy(m_Value, _Inits, sizeof(_CountType)*iSize);
		memcpy(m_Init, _Inits, sizeof(_CountType)*iSize);
		memcpy(m_Limits, _Limits, sizeof(_CountType)*iSize);
	}

	~VectorCounter()
	{
		delete []m_Value;
		delete []m_Limits;
		delete []m_Init;
	}

	// Increment vector counter
	// Return
	//		false	counter increased
	//		true	counter overflow
	bool Inc()
	{
		for(int i=0; i<m_iSize; ++i)
		{
			++m_Value[i];
			if ( m_Value[i] < m_Limits[i] )
			{
				return false;
			}
			m_Value[i] = m_Init[i];
		}

		return true;
	}

	// Decrement vector counter
	// Return
	//		false	counter decreased by one
	//		true	counter underflow
	bool Dec()
	{
		for(int i=0; i<m_iSize; ++i)
		{
			if ( m_Value[i] > m_Init[i] )
			{
				--m_Value[i];
				return false;
			}

			m_Value[i] = m_Limits[i] - 1;
		}

		return true;
	}

	_CountType*	GetValue()
	{
		return m_Value;
	}

protected:
	int				m_iSize;
	_CountType*		m_Value;
	_CountType*		m_Limits;
	_CountType*		m_Init;
};

}}