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

#if !defined(_OCL_IMAGE_H_)
#define _OCL_IMAGE_H_

#include <cl_types.h>
#include <cl_memory_object.h>
#include <logger.h>
#include <map>

namespace Intel { namespace OpenCL { namespace Framework {

	class Device;

	/**********************************************************************************************
	* Class name:	Image2D
	*
	* Inherit:		MemoryObject
	* Description:	represents a 2 dimentional image object
	* Author:		Uri Levy
	* Date:			April 2008
	**********************************************************************************************/		
	class Image2D : public MemoryObject
	{
	public:

		/******************************************************************************************
		* Function: 	Image2D
		* Description:	The Image2D class constructor
		* Arguments:	
		* Author:		Uri Levy
		* Date:			April 2008
		******************************************************************************************/		
		Image2D(Context *         pContext, 
				cl_mem_flags      clMemFlags,
				cl_image_format * pclImageFormat,
				void *            pHostPtr,
				size_t            szImageWidth,
				size_t            szImageHeight,
				size_t            szImageRowPitch,
				cl_err_code *     pErrCode);

		/******************************************************************************************
		* Function: 	~Image2D
		* Description:	The Image2D class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			April 2008
		******************************************************************************************/			
		virtual ~Image2D();

		// get image info
		virtual cl_err_code GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);

		virtual cl_err_code Release();

		// MemoryObject methods
		cl_err_code CreateDeviceResource(cl_device_id clDeviceId);
		cl_err_code ReadData(void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch, size_t szSlicePitch = 0);
		size_t GetSize() const;
        virtual size_t GetRowPitchSize() const { return m_szImageRowPitch; }
		
		virtual size_t CalcRowPitchSize(const size_t * pszRegion) { return pszRegion[0] * GetPixelBytesCount(m_pclImageFormat); }

        bool   CheckBounds( const size_t* pszOrigin, const size_t* pszRegion) const;
		void*  GetData( const size_t * pszOrigin = NULL ) const;
		void*  CreateMappedRegion(	cl_device_id clDeviceId, 
									cl_map_flags clMapFlags, 
									const size_t * szOrigins, 
									const size_t * szRegions, 
									size_t * pszImageRowPitch, 
									size_t * pszImageSlicePitch);
        cl_err_code ReleaseMappedRegion(cl_device_id clDeviceId, void* mappedPtr);
		void* GetMappedRegionInfo( cl_device_id clDeviceId, void* mappedPtr);

	protected:

		// check if the image format is supported and valid
		virtual cl_err_code CheckImageFormat(cl_image_format * pclImageFormat, cl_mem_flags clMemFlags);

		// get number of bytes per pixel for the current image
		virtual size_t GetPixelBytesCount(cl_image_format * pclImageFormat);

		// calculate the total image size
		virtual size_t CalcImageSize();

		cl_image_format *	m_pclImageFormat;
		size_t				m_szImageWidth;
		size_t				m_szImageHeight;
		size_t				m_szImageRowPitch;

	};


	/**********************************************************************************************
	* Class name:	Image3D
	*
	* Inherit:		Image2D
	* Description:	represents a 3 dimentional image object
	* Author:		Uri Levy
	* Date:			April 2008
	**********************************************************************************************/
	class Image3D : public Image2D
	{
	public:
		/******************************************************************************************
		* Function: 	Image3D
		* Description:	The Image3D class constructor
		* Arguments:	
		* Author:		Uri Levy
		* Date:			April 2008
		******************************************************************************************/		
		Image3D(Context *         pContext, 
				cl_mem_flags      clMemFlags,
				cl_image_format * pclImageFormat,
				void *            pHostPtr,
				size_t            szImageWidth,
				size_t            szImageHeight,
				size_t            szImageDepth,
				size_t            szImageRowPitch,
				size_t            szImageSlicePitch,
				cl_err_code *     pErrCode);

		/******************************************************************************************
		* Function: 	~Image3D
		* Description:	The Image3D class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			April 2008
		******************************************************************************************/			
		virtual ~Image3D();

		// get image info
		virtual cl_err_code GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);

		virtual cl_err_code Release();

		// MemoryObject methods
		cl_err_code CreateDeviceResource(cl_device_id clDeviceId);
		cl_err_code ReadData(void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch, size_t szSlicePitch);
		size_t GetSize() const;
        virtual size_t GetSlicePitchSize() const { return m_szImageSlicePitch; }
		virtual size_t CalcSlicePitchSize(const size_t * pszRegion) { return pszRegion[0] * pszRegion[1] * GetPixelBytesCount(m_pclImageFormat); }

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

	protected:

		// calculate the total image size
		size_t CalcImageSize();

		size_t	m_szImageDepth;
		size_t	m_szImageSlicePitch;


	};




}}};


#endif //_OCL_IMAGE_H_