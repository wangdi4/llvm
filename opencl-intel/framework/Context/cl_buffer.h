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

#pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////
//  cl_buffer.h
//  Implementation of the Class Buffer
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(_OCL_BUFFER_H_)
#define _OCL_BUFFER_H_

#include <cl_types.h>
#include <cl_memory_object.h>
#include <logger.h>
#include <map>

namespace Intel { namespace OpenCL { namespace Framework {

	class Device;

	/**********************************************************************************************
	* Class name:	Buffer
	*
	* Inherit:		MemoryObject
	* Description:	represents a memory object
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/		
	class Buffer : public MemoryObject
	{
	public:

		/******************************************************************************************
		* Function: 	Buffer
		* Description:	The Buffer class constructor
		* Arguments:	
		* Author:		Uri Levy
		* Date:			January 2008
		******************************************************************************************/		
		Buffer(Context * pContext, cl_mem_flags clMemFlags, void * pHostPtr, size_t szBufferSize, cl_err_code * pErrCode);

		/******************************************************************************************
		* Function: 	~Buffer
		* Description:	The Buffer class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			January 2008
		******************************************************************************************/			
		virtual ~Buffer();

		cl_err_code Release();

		// MemoryObject methods
		cl_err_code CreateDeviceResource(cl_device_id clDeviceId);
		cl_err_code ReadData(void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch = 0, size_t szSlicePitch = 0);
		size_t GetSize() const;
        bool CheckBounds( const size_t* pszOrigin, const size_t* pszRegion) const;
		void * GetData( const size_t * pszOrigin = NULL ) const;
		void* CreateMappedRegion(	cl_device_id clDeviceId, 
									cl_map_flags clMapFlags, 
									const size_t * szOrigins, 
									const size_t * szRegions, 
									size_t * pszImageRowPitch, 
									size_t * pszImageSlicePitch);
        cl_err_code ReleaseMappedRegion(cl_device_id clDeviceId, void* mappedPtr);
		void* GetMappedRegionInfo( cl_device_id clDeviceId, void* mappedPtr);


	private:

	};


}}};


#endif //_OCL_CONTEXT_H_