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
//  image.h
//  Implementation of the Classes Image2D and Image3D
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SingleUnifiedMemObj.h"

#include <cl_types.h>
#include <Logger.h>
#include <map>

namespace Intel { namespace OpenCL { namespace Framework {

	class Device;

	/**********************************************************************************************
	* Class name:	Image2D
	*
	* Inherit:		SingleUnifiedImage2D
	* Description:	represents a 2 dimensional image object with single device support
	*					and shared host memory
	* Author:		Uri Levy
	* Date:			April 2008
	**********************************************************************************************/
	class SingleUnifiedImage2D : public SingleUnifiedMemObject
	{
	public:

		SingleUnifiedImage2D(Context * pContext, ocl_entry_points * pOclEntryPoints, cl_mem_object_type clObjType);

		cl_err_code Initialize(
			cl_mem_flags		clMemFlags,
			const cl_image_format*	pclImageFormat,
			unsigned int		dim_count,
			const size_t*		dimension,
			const size_t*       pitches,
			void*				pHostPtr
			);
		// get image info
		cl_err_code GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);

		// MemoryObject methods
		cl_err_code CreateDeviceResource(cl_device_id clDeviceId);
		cl_err_code ReadData(void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch, size_t szSlicePitch = 0);
		cl_err_code WriteData(const void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch, size_t szSlicePitch = 0);
        size_t GetRowPitchSize() const { return m_szImageRowPitch; }

		virtual size_t	CalcRowPitchSize(const size_t * pszRegion) { return pszRegion[0] * m_szElementSize; }
		void			GetLayout( OUT size_t* dimensions, OUT size_t* rowPitch, OUT size_t* slicePitch ) const;
        cl_err_code			CheckBounds( const size_t* pszOrigin, const size_t* pszRegion) const;
		cl_err_code			CheckBoundsRect( const size_t* pszOrigin, const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch) const;
		void*			GetBackingStoreData( const size_t * pszOrigin = NULL ) const;
		size_t			GetPixelSize() const;
		size_t			GetSlicePitchSize(void) const;

		cl_err_code CreateSubBuffer(cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type,
			const void * buffer_create_info, MemoryObject** ppBuffer) {return CL_INVALID_OPERATION;}

	protected:
		/******************************************************************************************
		* Function: 	~SingleUnifiedImage2D
		* Description:	The SingleUnifiedImage2D class destructor
		* Arguments:
		* Author:		Uri Levy
		* Date:			April 2008
		******************************************************************************************/
		virtual ~SingleUnifiedImage2D();

		// calculate the total image size
		virtual size_t CalcImageSize();

		size_t				m_szImageWidth;
		size_t				m_szImageHeight;
		size_t				m_szImageRowPitch;
		size_t				m_szElementSize;

		// do not implement
        SingleUnifiedImage2D(const SingleUnifiedImage2D&);
        SingleUnifiedImage2D& operator=(const SingleUnifiedImage2D&);
	};
}}}

