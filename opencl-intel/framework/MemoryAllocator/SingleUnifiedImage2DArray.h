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

#include "SingleUnifiedImage3D.h"

#include <cl_types.h>
#include <Logger.h>
#include <vector>

namespace Intel { namespace OpenCL { namespace Framework {

	class Device;

    /**********************************************************************************************
	* Class name:	SingleUnifiedImage2DArray
	*
	* Inherit:		MemoryObject
	* Description:	represents an array of 2 dimensional image objects
	* Author:		Evgeny Fiksman
	* Date:			April 2011
	**********************************************************************************************/
    class SingleUnifiedImage2DArray : public SingleUnifiedImage3D, public IMemoryObjectArray
    {
    public:

        /************************************************************************/
        /* Constructor                                                          */
        /************************************************************************/
        SingleUnifiedImage2DArray(Context * pContext, ocl_entry_points * pOclEntryPoints, cl_mem_object_type clObjType);

		// IMemoryObjectArray
        MemoryObject* GetMemObject(size_t index)
        {
            assert(index < m_szImageDepth);
            return m_img2DArray[index];
        }

		size_t GetNumObjects() const { return m_szImageDepth; }

        // MemoryObject
		cl_err_code Initialize(
			cl_mem_flags		clMemFlags,
			const cl_image_format*	pclImageFormat,
			unsigned int		dim_count,
			const size_t*		dimension,
			const size_t*       pitches,
			void*				pHostPtr
			);


		cl_err_code ReadData(void* pOutData, const size_t* pszOrigin,
                                   const size_t* pszRegion, size_t szRowPitch,
                                   size_t szSlicePitch);
		cl_err_code WriteData(const void* pOutData, const size_t* pszOrigin,
                                    const size_t* pszRegion, size_t szRowPitch,
                                    size_t szSlicePitch);

        void GetLayout(OUT size_t* dimensions, OUT size_t* rowPitch, OUT size_t* slicePitch) const;

		cl_err_code CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const;

		void* GetBackingStoreData(const size_t* pszOrigin);
#if 0   // disabled until changes in the spec regarding 2D image arrays are made
        cl_err_code GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);
#endif

    protected:
		std::vector<MemoryObject*>	m_img2DArray;
        ~SingleUnifiedImage2DArray();

        // do not implement
        SingleUnifiedImage2DArray(const SingleUnifiedImage2DArray&);
        SingleUnifiedImage2DArray& operator=(const SingleUnifiedImage2DArray&);

    };

}}}
