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

#include <cl_types.h>
#include <cl_memory_object.h>
#include <Logger.h>
#include <map>

namespace Intel { namespace OpenCL { namespace Framework {

	class Device;

	/**********************************************************************************************
	* Class name:	Image2D
	*
	* Inherit:		MemoryObject
	* Description:	represents a 2 dimensional image object
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
				ocl_entry_points * pOclEntryPoints,
				cl_err_code *     pErrCode);

		// get image info
		virtual cl_err_code GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);

		// MemoryObject methods
		cl_err_code CreateDeviceResource(cl_device_id clDeviceId);
		cl_err_code ReadData(void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch, size_t szSlicePitch = 0);
		cl_err_code WriteData(const void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch, size_t szSlicePitch = 0);
		size_t GetSize() const;
		size_t GetNumDimensions() const { return 2; }
        virtual size_t GetRowPitchSize() const { return m_szImageRowPitch; }
		
		virtual size_t CalcRowPitchSize(const size_t * pszRegion) { return pszRegion[0] * GetPixelBytesCount(m_pclImageFormat); }
		void GetLayout( OUT size_t* dimensions, OUT size_t* rowPitch, OUT size_t* slicePitch ) const;
        cl_err_code CheckBounds( const size_t* pszOrigin, const size_t* pszRegion) const;
		void*  GetData( const size_t * pszOrigin = NULL ) const;
        // get number of bytes per pixel for the current image
        static size_t GetPixelBytesCount(const cl_image_format * pclImageFormat);

	protected:
		/******************************************************************************************
		* Function: 	~Image2D
		* Description:	The Image2D class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			April 2008
		******************************************************************************************/			
		virtual ~Image2D();

		// check if the image format is supported and valid
		virtual cl_err_code CheckImageFormat(cl_image_format * pclImageFormat, cl_mem_flags clMemFlags);

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
				ocl_entry_points * pOclEntryPoints,
				cl_err_code *     pErrCode);


		// get image info
		virtual cl_err_code GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);

		// MemoryObject methods
		cl_err_code CreateDeviceResource(cl_device_id clDeviceId);
		cl_err_code ReadData(void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch, size_t szSlicePitch);
		size_t GetSize() const;
		size_t GetNumDimensions() const { return 3; }
        virtual size_t GetSlicePitchSize() const { return m_szImageSlicePitch; }
		virtual size_t CalcSlicePitchSize(const size_t * pszRegion) { return pszRegion[0] * pszRegion[1] * GetPixelBytesCount(m_pclImageFormat); }
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
		virtual ~Image3D();

		// calculate the total image size
		size_t CalcImageSize();

		size_t	m_szImageDepth;
		size_t	m_szImageSlicePitch;


	};

    /**********************************************************************************************
	* Class name:	Image2DArray
	*
	* Inherit:		MemoryObject
	* Description:	represents an array of 2 dimensional image objects
	* Author:		Aharon Abramson
	* Date:			April 2011
	**********************************************************************************************/		
    class Image2DArray : public MemoryObject
    {
    public:

        /************************************************************************/
        /* Constructor                                                          */
        /************************************************************************/
        Image2DArray(Context*          pContext, 
                     cl_mem_flags      clMemFlags,
                     cl_image_format*  pclImageFormat,
                     void*             pHostPtr,
                     size_t            szImageWidth,
                     size_t            szImageHeight,
                     size_t            szNumImages,
                     size_t            szImageRowPitch,
                     size_t            szImageSlicePitch,
                     ocl_entry_points* pOclEntryPoints,
                     cl_err_code*      pErrCode);

        /************************************************************************/
        /* Destructor                                                           */
        /************************************************************************/
        ~Image2DArray();

        /************************************************************************
         * @param index index of 2D image inside the array
         * @return the Image2D object representing the 2D image whose index in the
         *  array is index.
         ************************************************************************/
        const Image2D& GetImage(size_t index) const
        {
            assert(index < m_szNumImages);
            return *m_pImageArr[index];
        }

        /************************************************************************
         * @param index index of 2D image inside the array
         * @return the Image2D object representing the 2D image whose index in the
         *  array is index.
         ************************************************************************/
        Image2D& GetImage(size_t index)
        {
            assert(index < m_szNumImages);
            return *m_pImageArr[index];
        }

        /************************************************************************
         * @return number of Image2D objects in this Image2DArray
         ************************************************************************/
        size_t GetNumImages() const { return m_szNumImages; }

        // overridden methods:

        cl_err_code Initialize(void* pHostPtr);

        virtual cl_err_code ReadData(void* pOutData, const size_t* pszOrigin,
            const size_t* pszRegion, size_t szRowPitch = 0, size_t szSlicePitch = 0);

        virtual cl_err_code WriteData(const void* pOutData, const size_t* pszOrigin,
            const size_t* pszRegion, size_t szRowPitch = 0, size_t szSlicePitch = 0);

        virtual size_t GetSize() const;

        virtual size_t GetNumDimensions() const;

        virtual void GetLayout(OUT size_t* dimensions, OUT size_t* rowPitch, OUT size_t* slicePitch) const;
        
        virtual cl_err_code CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const;

        virtual void* GetData(const size_t* pszOrigin = NULL) const;

        virtual cl_err_code CreateDeviceResource(cl_device_id clDeviceId);

        virtual size_t CalcRowPitchSize(const size_t *  pszRegion);

        virtual size_t CalcSlicePitchSize(const size_t *  pszRegion);

        virtual cl_err_code GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);

    private:

        Image2D** m_pImageArr; 
        size_t m_szNumImages;
        cl_image_format m_pclImageFormat;
        size_t m_szImageWidth;
        size_t m_szImageHeight;
        size_t m_szImageRowPitch;
        size_t m_szImageSlicePitch;
        void** m_pImageData;

        // do not implement
        Image2DArray(const Image2DArray&);
        Image2DArray& operator=(const Image2DArray&);

    };

}}}
