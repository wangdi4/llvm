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

#include "SingleUnifiedImage2D.h"

#include <cl_types.h>
#include <Logger.h>
#include <map>

namespace Intel { namespace OpenCL { namespace Framework {

	class Device;


	/**********************************************************************************************
	* Class name:	Image3D
	*
	* Inherit:		Image2D
	* Description:	represents a 3 dimensional image object
	* Author:		Uri Levy
	* Date:			April 2008
	**********************************************************************************************/
	class SingleUnifiedImage3D : public SingleUnifiedImage2D
	{
	public:
		SingleUnifiedImage3D(Context *pContext, ocl_entry_points * pOclEntryPoints, cl_mem_object_type clObjType);

		cl_err_code Initialize(
			cl_mem_flags			clMemFlags,
			const cl_image_format*	pclImageFormat,
			unsigned int			dim_count,
			const size_t*			dimension,
			const size_t*			pitches,
			void*					pHostPtr,
			cl_rt_memobj_creation_flags	creation_flags
			);

		// get image info
		cl_err_code GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);

		// MemoryObject methods
		cl_err_code CreateDeviceResource(cl_device_id clDeviceId);
		cl_err_code ReadData(void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch, size_t szSlicePitch);
		virtual size_t GetSlicePitchSize() const { return m_szImageSlicePitch; }
		virtual size_t CalcSlicePitchSize(const size_t * pszRegion) { return pszRegion[0] * pszRegion[1] * m_szElementSize; }
		void GetLayout( OUT size_t* dimensions, OUT size_t* rowPitch, OUT size_t* slicePitch ) const;
        cl_err_code CheckBounds( const size_t* pszOrigin, const size_t* pszRegion) const;
		void * GetData( const size_t * pszOrigin = NULL ) const;

	protected:

		/******************************************************************************************
		* Function: 	~Image3D
		* Description:	The Image3D class destructor
		* Arguments:
		* Author:		Uri Levy
		* Date:			April 2008
		******************************************************************************************/
		virtual ~SingleUnifiedImage3D();

		// calculate the total image size
		size_t CalcImageSize();

		size_t	m_szImageDepth;
		size_t	m_szImageSlicePitch;

		// do not implement
        SingleUnifiedImage3D(const SingleUnifiedImage3D&);
        SingleUnifiedImage3D& operator=(const SingleUnifiedImage3D&);
	};




}}}
