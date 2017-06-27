// Copyright (c) 2006-2012 Intel Corporation
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
//  fe_compiler.h
//  Implementation of the front-end compiler class
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <Logger.h>
#include <cl_dynamic_lib.h>
#include "cl_framework.h"
#include <cl_object.h>
#include <frontend_api.h>

namespace Intel { namespace OpenCL { namespace Framework {

    /**********************************************************************************************
    * Class name:   FECompiler
    *
    * Description:  front-end compiler class
    * Author:       Uri Levy
    * Date:         March 2008
    **********************************************************************************************/
    class FrontEndCompiler : public OCLObject<_cl_object>
    {

    private:

        /******************************************************************************************
        * Function:     FrontEndCompiler
        * Description:  Private copy constructor to avoid wrong assignment (Klocwork)
        * Arguments:
        * Author:       Guy Benyei
        * Date:         June 2012
        ******************************************************************************************/
        FrontEndCompiler(const FrontEndCompiler&): OCLObject<_cl_object>(NULL, "") {};  

        /******************************************************************************************
        * Function:     FECompiler
        * Description:  The Frontend compiler class constructor
        * Arguments:        
        * Author:       Uri Levy
        * Date:         March 2008
        ******************************************************************************************/
        FrontEndCompiler();

    public:

        PREPARE_SHARED_PTR(FrontEndCompiler)

        static SharedPtr<FrontEndCompiler> Allocate()
        {
            return SharedPtr<FrontEndCompiler>(new FrontEndCompiler());
        }

        /******************************************************************************************
        * Function:     Initialize    
        * Description:  Initialize the front-end compiler
        * Arguments:        
        * Return value: CL_SUCCESS - The initialization operation succeeded
        * Author:       Uri Levy
        * Date:         March 2008
        ******************************************************************************************/     
        cl_err_code     Initialize(const char * psModuleName, const void *pDeviceInfo, size_t stDevInfoSize);

        /******************************************************************************************
        * Function:     FreeResources    
        * Description:  Frees the front-end compiler resources
        * Arguments:        
        * Return value: CL_SUCCESS - The operation succeeded
        * Author:       Doron Singer
        * Date:         March 2008
        ******************************************************************************************/
        void        FreeResources();

        /******************************************************************************************
        * Function:     CompileProgram    
        * Description:  Compile source code and headers and return binary data
        * Arguments:    szProgramSource - the main program source string
        *               uiNumInputHeaders - the number of input headers in pszInputHeaders
        *               pszInputHeaders - an array of input headers strings
        *               pszInputHeadersNames - array of headers names corresponding to pszInputHeaders
        *               szOptions - compile options string
        * Output:       ppBinary - the compiled binary container
        *               puiBinarySize - pBinary size in bytes
        *               pszCompileLog - compile log string
        * Return value: CL_SUCCESS - The operation succeeded
        * Author:       Sagi Shahar
        * Date:         January 2012
        ******************************************************************************************/
        cl_err_code CompileProgram( const char*   szProgramSource,
                                    unsigned int  uiNumInputHeaders,
                                    const char**  pszInputHeaders,
                                    const char**  pszInputHeadersNames, 
                                    const char *  szOptions,
                                    bool          bFpgaEmulator,
                                    OUT char**    ppBinary,
                                    OUT size_t*   puiBinarySize,
                                    OUT char**    pszCompileLog) const;

        /******************************************************************************************
        * Function:     ParseSpirv
        * Description:  Convert SPIRV binary to LLVM.
        * Arguments:    szProgramBinary - the main program binary.
        *               szOptions - compile options string.
        * Output:       ppBinary - the compiled binary container.
        *               puiBinarySize - pBinary size in bytes.
        *               pszCompileLog - compile log string.
        * Return value: CL_SUCCESS - The operation succeeded.
        * Author:       Vlad Romanov
        * Date:         January 2016
        ******************************************************************************************/
        cl_err_code ParseSpirv(const char*    szProgramBinary,
                               unsigned int   uiProgramBinarySize,
                               const char*    szOptions,
                               OUT char**     ppBinary,
                               OUT size_t*    puiBinarySize,
                               OUT char**     pszCompileLog) const;

        /******************************************************************************************
        * Function:     MaterializeSPIR
        * Description:  Converts SPIR 1.2 binary to LLVM IR form compatible with OpenCL compiler.
        * Arguments:    szProgramBinary - SPIR 1.2 program binary.
                        uiProgramBinarySize - SPIR 1.2 probram binary size
        * Output:       ppBinary - materialized binary container.
        *               puiBinarySize - pBinary size in bytes.
        *               pszCompileLog - compile log string.
        * Return value: CL_SUCCESS - The operation succeeded.
        ******************************************************************************************/
        cl_err_code MaterializeSPIR(const char*    szProgramBinary,
                                    unsigned int   uiProgramBinarySize,
                                    OUT char**     ppBinary,
                                    OUT size_t*    puiBinarySize,
                                    OUT char**     pszCompileLog) const;

        /******************************************************************************************
        * Function:     LinkProgram    
        * Description:  Compile source code and return binary data
        * Arguments:    ppBinaries - array of binary containers to be link together
        *               uiNumInputBinaries - num containers in ppBinaries
        *               puiBinariesSizes - sizes of the containers in ppBinaries
        *               szOptions - link options string
        * Output:       pBinary - the linked binary container
        *               uiBinarySize - pBinary size in bytes
        *               szCompileLog - link log string
        * Return value: CL_SUCCESS - The operation succeeded
        * Author:       Sagi Shahar
        * Date:         January 2012
        ******************************************************************************************/
        cl_err_code LinkProgram(  const void**    ppBinaries,
                                  unsigned int    uiNumInputBinaries,
                                  const size_t*   puiBinariesSizes,
                                  const char *    szOptions,
                                  OUT char**      ppBinary,
                                  OUT size_t*     puiBinarySize,
                                  OUT std::vector<char>& linkLog,
                                  OUT bool*       pbIsLibrary) const;

        /******************************************************************************************
        * Function:     GetKernelArgInfo    
        * Description:  Get the kernel arguments info
        * Arguments:    pBin - the program's binary including the header
        *               uiBinarySize - size of the binary
        *               szKernelName - the name of the kernel for which we query the arg info
        * Output:       ppArgInfo - a struct containing all the arguments info
        *               puiNumArgs - the number of arguments
        * Return value: CL_SUCCESS - The operation succeeded
        *               CL_KERNEL_ARG_INFO_NOT_AVAILABLE if binary was built without -cl-kernel-arg-info
        *               CL_OUT_OF_HOST_MEMORY for out of host memory
        * Author:       Sagi Shahar
        * Date:         March 2012
        ******************************************************************************************/
        cl_err_code GetKernelArgInfo(   const void*                       pBin,
                                        size_t                            uiBinarySize,
                                        const char*                       szKernelName,
                                        ClangFE::IOCLFEKernelArgInfo*   *ppArgInfo ) const;

        /******************************************************************************************
        * Function:     CheckCompileOptions    
        * Description:  Check if the compile options are legal
        * Arguments:    szOptions - a string representing the compile options
        * Output:       szUnrecognizedOptions - a new string containing the unrecognized options separated by spaces
        * Return value: true - the compile options are legal
        *               false otherwise
        * Author:       Sagi Shahar
        * Date:         May 2012
        ******************************************************************************************/
        bool CheckCompileOptions(const char*  szOptions,
                                 char*        szUnrecognizedOptions,
                                 size_t       uiUnrecognizedOptionsSize) const;

        /******************************************************************************************
        * Function:     CheckLinkOptions    
        * Description:  Check if the link options are legal
        * Arguments:    szOptions - a string representing the link options
        * Output:       szUnrecognizedOptions - a new string containing the unrecognized options separated by spaces
        * Return value: true - the link options are legal
        *               false otherwise
        * Author:       Sagi Shahar
        * Date:         May 2012
        ******************************************************************************************/
        bool CheckLinkOptions(const char*  szOptions,
                              char*       szUnrecognizedOptions, 
                              size_t uiUnrecongnizedOptionsSize) const;

        /******************************************************************************************
        * Function:     GetModuleName    
        * Description:  returns the module name of the front-end compiler
        * Arguments:    N/A
        * Return value: [char *] - pointer to the module name's string
        * Author:       Uri Levy
        * Date:         March 2008
        ******************************************************************************************/
        const char * GetModuleName() const { return m_pszModuleName; }

        //OclObject implementation
        cl_err_code GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const {return CL_INVALID_OPERATION; }

    protected:
        /******************************************************************************************
        * Function:     ~FECompiler
        * Description:  The Frontend compiler class destructor
        * Arguments:        
        * Author:       Uri Levy
        * Date:         March 2008
        ******************************************************************************************/
        virtual ~FrontEndCompiler();

        Utils::OclDynamicLib        m_dlModule;
        Intel::OpenCL::FECompilerAPI::fnCreateFECompilerInstance*   m_pfnCreateInstance;

        // module name
        const char *                m_pszModuleName;

        Intel::OpenCL::FECompilerAPI::IOCLFECompiler* m_pFECompiler;

        DECLARE_LOGGER_CLIENT;
    
    private:
        FrontEndCompiler& operator=(const FrontEndCompiler&);
        cl_err_code ProcessResults(cl_err_code Error,
                                   IOCLFEBinaryResult *Result, char **Binary,
                                   size_t *BinarySize, char **CompileLog) const;
    };
}}}
