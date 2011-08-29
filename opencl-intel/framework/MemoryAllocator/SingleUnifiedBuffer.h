// Copyright (c) 2006-2010 Intel Corporation
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

#include "SingleUnifiedMemObj.h"

#include <cl_types.h>
#include <Logger.h>

namespace Intel { namespace OpenCL { namespace Framework {

	class Device;

	/**********************************************************************************************
	* Class name:	Buffer
	*
	* Inherit:		MemoryObject
	* Description:	represents a memory buffer supported on a single device with unified memory
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/
	class SingleUnifiedBuffer : public SingleUnifiedMemObject
	{
	public:
		friend class SingleUnifiedSubBuffer;
		/******************************************************************************************
		* Function: 	Buffer
		* Description:	The Buffer class constructor
		* Arguments:
		* Author:		Uri Levy
		* Date:			January 2008
		******************************************************************************************/
		SingleUnifiedBuffer(Context * pContext, ocl_entry_points * pOclEntryPoints, cl_mem_object_type clObjType);

		cl_err_code Initialize(
			cl_mem_flags		clMemFlags,
			const cl_image_format*	pclImageFormat,
			unsigned int		dim_count,
			const size_t*		dimension,
			const size_t*       pitches,
			void*				pHostPtr
			);

		cl_err_code CreateSubBuffer(cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type,
			const void * buffer_create_info, MemoryObject** ppBuffer);

		// Memory object interface
		cl_err_code ReadData(void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch = 0, size_t szSlicePitch = 0);
		cl_err_code WriteData(const void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch = 0, size_t szSlicePitch = 0);

		void GetLayout( OUT size_t* dimensions, OUT size_t* rowPitch, OUT size_t* slicePitch ) const;
		cl_err_code CheckBounds( const size_t* pszOrigin, const size_t* pszRegion) const;
		cl_err_code CheckBoundsRect( const size_t* pszOrigin, const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch) const;
		void * GetBackingStoreData( const size_t * pszOrigin = NULL ) const;

		size_t GetPixelSize() const {return 1;}

		// Get object pitches. If pitch is irrelevant to the memory object, zero pitch is returned
		size_t GetRowPitchSize() const { return 0;};
		size_t GetSlicePitchSize() const  { return 0;};

	protected:
		/******************************************************************************************
		* Function: 	~Buffer
		* Description:	The Buffer class destructor
		* Arguments:
		* Author:		Uri Levy
		* Date:			January 2008
		******************************************************************************************/
		virtual ~SingleUnifiedBuffer();

		// do not implement
        SingleUnifiedBuffer(const SingleUnifiedBuffer&);
        SingleUnifiedBuffer& operator=(const SingleUnifiedBuffer&);
	};

	class SingleUnifiedSubBuffer : public SingleUnifiedBuffer
	{
	public:
		SingleUnifiedSubBuffer(Context * pContext, ocl_entry_points * pOclEntryPoints);

		cl_err_code Initialize(
			cl_mem_flags		clMemFlags,
			const cl_image_format*	pclImageFormat,
			unsigned int		dim_count,
			const size_t*		dimension,
			const size_t*       pitches,
			void*				pHostPtr
			);

		cl_err_code CreateSubBuffer(cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type,
			const void * buffer_create_info, MemoryObject** ppBuffer) {return CL_INVALID_MEM_OBJECT;}

		bool IsSupportedByDevice(FissionableDevice* pDevice);

	protected:
		virtual ~SingleUnifiedSubBuffer();

		// do not implement
        SingleUnifiedSubBuffer(const SingleUnifiedSubBuffer&);
        SingleUnifiedSubBuffer& operator=(const SingleUnifiedSubBuffer&);
	};
}}}

