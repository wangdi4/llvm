/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2009 Intel Corporation All Rights Reserved.
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

///////////////////////////////////////////////////////////
//  cl_backend_api.h
//  Created on:      Aug-2009
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////

/************************************************************************
 * As long as no backend is available for LRB, this agent implements
 * a pseudo implementation of the backend API.
 * TODO: Replace this implementation with a real backend
 ************************************************************************/
///////////////////////////////////////////////////////////
//  lrb_device_backend.h
//  Created on:      Aug-2009
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#if !defined(__CL_BACKEND_API_H__)
#define __CL_BACKEND_API_H__

#include "lrb_agent_common.h"
#include <cl_device_api.h>

namespace Intel { namespace OpenCL { namespace LRBAgent {

    // Forward deceleration
    class CLBackendKernel; 

    /**********************************************************************************************
     * Class name:    CLBackendExecutable
     *
     * Description:    
     *       Represents a single execution of a kernel
     *
     * Author:        Arnon Peleg
     * Date:          Aug. 2009
    /**********************************************************************************************/    
    class CLBackendExecutable
    {
    public:

        CLBackendExecutable(
            CLBackendKernel*    pKernel,
            void*               pArgsBuffer,
            size_t              BufferSize,
            cl_uint             WorkDimension,
            const size_t*       pGlobalWorkSize,
            const size_t*       pLocalWorkSize
            );
        ~CLBackendExecutable() {};

        cl_uint Execute(
            void*         pLocalMemoryBuffers, 
            const size_t* pBufferCount, 
            const size_t* pGlobalId, 
            const size_t* pLocalId, 
            const size_t* pItemsToProcess 
            );

        cl_uint GetLocalMemoryBuffersSizes(size_t* pBuffersSizes, size_t*  pBufferCount ) { return 0; }
        cl_uint GetKernelPackSize() const           { return 0; };  
        const CLBackendKernel* GetKernel() const    { return m_pKernel; }

    private:
        CLBackendKernel*    m_pKernel;
        cl_uint             m_uiWorkDimension;
        size_t              m_pszGlobalWorkSize[3];
        size_t              m_pszLocalWorkSize[3];

        // This executable supports only a fix type of kernel with the prototype:
        // void (fn_lrbKernelFunction)( const float* , const float* , float* c, int tid);
        void* m_pSrcBuffer1;
        void* m_pSrcBuffer2;
        void* m_pDstBuffer;
    };

    /**********************************************************************************************
     * Class name:    CLBackendKernel
     *
     * Description:    
     *       Represents kernel in the system
     *
     * Author:        Arnon Peleg
     * Date:          Aug. 2009
    /**********************************************************************************************/    
    class CLBackendKernel
    {
    public:	
        CLBackendKernel( char* pKernelName, fn_lrbKernelFunction* pfnLrbKernelFunction);
        virtual ~CLBackendKernel();

        cl_int CreateExecutable(
                    void*                 pArgsBuffer,
                    size_t                BufferSize,
                    cl_uint               WorkDimension,
                    const size_t*         pGlobalWorkSize,
                    const size_t*         pLocalWorkSize,
                    CLBackendExecutable** ppExecutable
            );

        cl_int      GetKernelParams( void* pArgsBuffer, size_t* BufferSize );
        const char*	GetKernelName() const                       { return m_pKernelName; }
        size_t      GetImplicitLocalMemoryBufferSize() const    { return 0; }
        size_t      GetKernelPackSize() const                   { return 0; }
        size_t      GetRequiredWorkGroupSize() const            { return 0; }

    private:
        friend class CLBackendExecutable;

        char*                   m_pKernelName;
        fn_lrbKernelFunction*   m_pfnLrbKernelFunction;
    };

    /**********************************************************************************************
     * Class name:    CLBackendProgram
     *
     * Description:    
     *       Represents program in the system
     *
     * Author:        Arnon Peleg
     * Date:          Aug. 2009
    /**********************************************************************************************/    
    class CLBackendProgram
    {
    public:
        static cl_int CreateProgram(const cl_prog_container* pContainer, CLBackendProgram**  pProgram );
        const cl_prog_container* GetContainer( const cl_prog_binary_desc* pDescriptor  );
        cl_int	BuildProgram( const char *pOptions );
        cl_int  GetBuildLog(size_t  *pSize, char*  pLog) const;
        cl_int	GetKernel(const char* pKernelName, const CLBackendKernel** pKernel);
        cl_int	GetAllKernels(CLBackendKernel*** pKernels, uint32_t*  puiRetCount);

        // Current implementation specific functions (not part of the API
        cl_int GenerateOutputData(void* pOutputBuffer, uint32_t uiBufferLen);

    protected:
        // Construction of program objects must be performed using the BackendProgram object
        CLBackendProgram();
        ~CLBackendProgram();
        
        void*             m_hLib;            // The program library handle
        char*             m_pLibName;        // LibName without extensions and path, to use to get pointer to functions.
        int               m_iNumKernels;    // Number of kernels in this program
        CLBackendKernel** m_kernelsTable;  // List of pointers to kernels objects
    };

}}};    // Intel::OpenCL::LRBAgent

#endif  // !defined(__CL_BACKEND_API_H__)
