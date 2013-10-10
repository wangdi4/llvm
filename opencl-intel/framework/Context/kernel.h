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
//  kernel.h
//  Implementation of the Kernel class
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <cl_types.h>
#include <cl_object.h>
#include <Logger.h>
#include <cl_device_api.h>
#include <cl_objects_map.h>
#include "Device.h"
#include "cl_sys_defines.h"

namespace Intel { namespace OpenCL { namespace Framework {

    class Program;
    class DeviceProgram;
    class Kernel;
    class Context;
    class SVMBuffer;
    class SVMPointerArg;

    /**********************************************************************************************
    * Class name:    SKernelPrototype
    *
    * Description:    contains information on kernel prototype
    * Members:        m_psKernelName    - the name of the kernel
    *                m_uiArgsCount    - number of arguments in the kernel
    *                m_pArgs            - list of all kernel's arguments
    * Author:        Uri Levy
    * Date:            January 2008
    **********************************************************************************************/    
    struct SKernelPrototype
    {
        char *                        m_psKernelName;
        cl_uint                       m_uiArgsCount;
        cl_kernel_argument*           m_pArgs;
        cl_kernel_argument_info*      m_pArgsInfo;
    };

    /**********************************************************************************************
    * Class name:    DeviceKernel
    *
    * Description:    represents a device kernel object
    * Author:        Uri Levy
    * Date:            January 2008
    **********************************************************************************************/    
    class DeviceKernel : public OCLObjectBase
    {
    public:
        
        // Constructor
        DeviceKernel(   Kernel*                                pKernel,
                        const SharedPtr<FissionableDevice>&    pDevice,
                        cl_dev_program                         devProgramId,
                        const char *                           psKernelName,
                        Intel::OpenCL::Utils::LoggerClient *   pLoggerClient,
                        cl_err_code *                          pErr );

        // Destructor
        ~DeviceKernel();

        // get kernel id
        cl_dev_kernel GetId()       const { return m_clDevKernel; }

        // Get device ID
        cl_int        GetDeviceId() const { return m_pDevice->GetId(); }

        const SharedPtr<FissionableDevice>& GetDevice() const {return m_pDevice;}

        // get kernel prototype
        const SKernelPrototype GetPrototype() const { return m_sKernelPrototype; }

        // compare between kernel's prototypes
        bool CheckKernelDefinition(DeviceKernel * pKernel) const;

        // get prefetched info
        size_t          GetKernelWorkGroupSize() const { return m_CL_KERNEL_WORK_GROUP_SIZE; }
        const size_t*   GetKernelCompileWorkGroupSize() const { return m_CL_KERNEL_COMPILE_WORK_GROUP_SIZE; }
        cl_ulong        GetKernelLocalMemSize() const { return m_CL_KERNEL_LOCAL_MEM_SIZE; }

    private:
        bool            CacheRequiredInfo();

        // device kernel id
        cl_dev_kernel                           m_clDevKernel;

        // kernel prototype
        SKernelPrototype                        m_sKernelPrototype;

        // parent kernel
        // We hold a regular pointer to Kernel rather than a SharedPtr, because Kernel points to DeviceKerel and otherwise we would have a circular reference.
        Kernel*                                 m_pKernel;
        
        // device to which the device kernel is associated
        SharedPtr<FissionableDevice>            m_pDevice;

        // prefetched info
        size_t                                  m_CL_KERNEL_WORK_GROUP_SIZE;
        size_t                                  m_CL_KERNEL_COMPILE_WORK_GROUP_SIZE[MAX_WORK_DIM];
        cl_ulong                                m_CL_KERNEL_LOCAL_MEM_SIZE;

        // logger client
        DECLARE_LOGGER_CLIENT;
    };

    /**********************************************************************************************
    * Class name:    KernelArg
    *
    * Inherit:        
    * Description:    represents a kernel argument object
    * Author:        Uri Levy
    * Date:            January 2008
    **********************************************************************************************/    
    class KernelArg
    {
    public:        

        KernelArg() : m_pValue(NULL), m_bValid(false) {}
        
        cl_uint             GetIndex() const  { return m_uiIndex; }
        cl_uint             GetOffset() const { return m_szOffset; }

        // return the size (in bytes) of the kernel arg's value
        // if Buffer / Image / ... returns sizeof(MemoryObject*)
        size_t              GetSize()          const { return m_clKernelArgType.size_in_bytes; }
        cl_kernel_arg_type  GetType()          const { return m_clKernelArgType.type; }

        // returns the value of the kernel argument
        void *              GetValue()    const { return m_pValue; }

        bool                IsMemObject() const { return (CL_KRNL_ARG_PTR_GLOBAL  <= m_clKernelArgType.type); }
        bool                IsBuffer()    const { return ((CL_KRNL_ARG_PTR_GLOBAL == m_clKernelArgType.type) || 
                                                          (CL_KRNL_ARG_PTR_CONST  == m_clKernelArgType.type)); }
        bool                IsImage()     const { return ((CL_KRNL_ARG_PTR_IMG_2D <= m_clKernelArgType.type) && 
                                                          (CL_KRNL_ARG_PTR_IMG_1D_BUF >= m_clKernelArgType.type)); }
        bool                IsSampler()   const { return (CL_KRNL_ARG_SAMPLER == m_clKernelArgType.type); }
        bool                IsLocalPtr()  const { return (CL_KRNL_ARG_PTR_LOCAL == m_clKernelArgType.type); }
        
        size_t              GetLocalBufferSize() const { assert( IsLocalPtr() ); assert( m_pValue); return *(size_t*)m_pValue; }

        bool                IsValid()     const { return m_bValid; }

        const SharedPtr<ReferenceCountedObject>& GetSvmObject() const   { return m_pSvmPtrArg; }
        
    private:

        void Init( cl_uint uiIndex, const cl_kernel_argument& clKernelArgType );
        void SetValuePlaceHolder( void * pValuePlaceHolder, size_t offset );
        void SetValue( size_t szSize, void * pValue );
        void SetSvmObject( const SharedPtr<ReferenceCountedObject>& svmMemObj ) { m_pSvmPtrArg = svmMemObj; }

        void SetValid() { m_bValid = true; }

        // index of kernel argument
        cl_uint                         m_uiIndex;

        // value of kernel argument
        void *                          m_pValue;
        size_t                          m_szOffset;

        // type of kernel argument
        cl_kernel_argument              m_clKernelArgType;

        bool                            m_bValid;

        SharedPtr<ReferenceCountedObject> m_pSvmPtrArg;    // we hold a SharedPtr to ReferenceCountedObject because of header dependencies

        friend class Kernel;
    };

    /**********************************************************************************************
    * Class name:    Kernel
    *
    * Inherit:        OCLObject
    * Description:    represents a kernel object
    * Author:        Uri Levy
    * Date:            January 2008
    **********************************************************************************************/        
    class Kernel : public OCLObject<_cl_kernel_int>
    {
    public:

        PREPARE_SHARED_PTR(Kernel)

        /******************************************************************************************
        * Function:     Allocate
        * Description:    Allocate a new Kernel
        * Arguments:    pProgram [in]        - associated program object
        *                psKernelName [in]    - kernel's name
        * Return:       a SharedPtr<Kernel> holding the new Kernel
        * Author:        Aharon Abramson
        * Date:            March 2012
        ******************************************************************************************/        
        static SharedPtr<Kernel> Allocate(const SharedPtr<Program>& pProgram, const char * psKernelName, size_t szNumDevices)
        {
            return SharedPtr<Kernel>(new Kernel(pProgram, psKernelName, szNumDevices));
        }

        /******************************************************************************************
        * Function:     Kernel
        * Description:    The Kernel class constructor
        * Arguments:    pProgram [in]        - associated program object
        *                psKernelName [in]    - kernel's name
        * Author:        Uri Levy
        * Date:            January 2008
        ******************************************************************************************/        
        Kernel(const SharedPtr<Program>& pProgram, const char * psKernelName, size_t szNumDevices);

        /******************************************************************************************
        * Function:     GetInfo    
        * Description:    get object specific information (inherited from OCLObject) the function 
        *                query the desirable parameter value from the device
        * Arguments:    iParamName [in]                parameter's name
        *                szParamValueSize [in out]    parameter's value size (in bytes)
        *                pParamValue [out]            parameter's value
        *                pszParamValueSizeRet [out]    parameter's value return size
        * Return value:    CL_SUCCESS - operation succeeded
        *                CL_INVALID_VALUE - if iParamName is not valid, or if size in bytes 
        *                                   specified by szParamValueSize is < size of return type 
        *                                    and pParamValue is not NULL
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/
        cl_err_code    GetInfo(cl_int iParamName, 
                            size_t szParamValueSize, 
                            void * pParamValue, 
                            size_t * pszParamValueSizeRet) const;


        /******************************************************************************************
        * Function:     GetWorkGroupInfo    
        * Description:    returns information about the kernel object that may be specific to a 
        *                device
        * Arguments:    clDevice [in]                identifies a specific device in the list of 
        *                                            devices associated with
        *                iParamName [in]                parameter's name
        *                szParamValueSize [inout]    parameter's value size (in bytes)
        *                pParamValue [out]            parameter's value
        *                pszParamValueSizeRet [out]    parameter's value return size
        * Return value:    CL_INVALID_DEVICE - ckDevice is not valid device
        *                CL_INVALID_VALUE - iParamName is not valid parameter
        *                CL_SUCCESS - operation succeeded
        * Author:        Uri Levy
        * Date:            April 2008
        ******************************************************************************************/
        cl_err_code    GetWorkGroupInfo(    const SharedPtr<FissionableDevice>&        pDevice,
                                        cl_int      iParamName, 
                                        size_t      szParamValueSize,
                                        void *      pParamValue,
                                        size_t *    pszParamValueSizeRet);

        // create device kernels
        cl_err_code CreateDeviceKernels(DeviceProgram ** pDevicePrograms);

        // set kernel argument
        cl_err_code SetKernelArg(cl_uint uiIndex, size_t szSize, const void * pValue, bool bIsSvmPtr = false);

        // returns the number of arguments in the kernel
        size_t GetKernelArgsCount() const { return m_sKernelPrototype.m_uiArgsCount; }

        // Return true if all kernel arguments were specified
        bool IsValidKernelArgs() const { return m_numValidArgs == m_sKernelPrototype.m_uiArgsCount; }

        // Return size in bytes for device arguments area
        size_t GetDeviceArgsSize() const { return m_deviceArgsSize; }
        size_t GetTotalLocalSize() const { return m_totalLocalSize; }
        void*  GetArgsBlob()       const { return m_pArgsBlob; }

        // Return true if there is a successfully built program executable available for device
        bool IsValidExecutable(const ConstSharedPtr<FissionableDevice>& pDevice) const;

        // get pointer to the kernel argument object of the uiIndex. if no available returns NULL;
        const KernelArg* GetKernelArg(size_t uiIndex) const 
        { 
            assert (uiIndex < m_sKernelPrototype.m_uiArgsCount); 
            return &(m_vecArgs[uiIndex]); 
        }

        // Returns non zero handle.
        cl_dev_kernel GetDeviceKernelId(const SharedPtr<FissionableDevice>& pDevice) const;
        const DeviceKernel* GetDeviceKernel(const ConstSharedPtr<FissionableDevice>& pDevice) const;

        // get kernel's name
        const char * GetName() const { return m_sKernelPrototype.m_psKernelName; }

        // get kernel's associated program
        ConstSharedPtr<Program> GetProgram() const { return m_pProgram; }
        const SharedPtr<Program>& GetProgram()     { return m_pProgram; }

        ConstSharedPtr<Context> GetContext() const { return m_pContext; }
        const SharedPtr<Context>& GetContext()     { return m_pContext; }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // OpenCL 1.2 functions
        ///////////////////////////////////////////////////////////////////////////////////////////

        /******************************************************************************************
        * Function:     clGetKernelArgInfo    
        * Description:    returns information about the arguments of a kernel.
        * Arguments:    argIndx [in]                is the argument index
        *                paramName [in]                specifies the argument information to query
        *                szParamValueSize [inout]    parameter's value size (in bytes)
        *                pParamValue [out]            parameter's value
        *                pszParamValueSizeRet [out]    parameter's value return size
        * Return value:    CL_INVALID_ARG_INDEX    - if arg_indx is not a valid argument index and param_name is
        *                                            not CL_KERNEL_ATTRIBUTES.
        *                CL_INVALID_VALUE        - if param_name is not valid, or if size in bytes specified by
        *                                            param_value size is < size of return type as described in
        *                                            table 5.17 and param_value is not NULL.
        *                CL_KERNEL_ARG_INFO_NOT_AVAILABLE - if the argument information is not available
        *                                            for kernel.
        * Author:        Evgeny Fiksman
        * Date:            August 2011
        ******************************************************************************************/                
        cl_err_code GetKernelArgInfo (  cl_uint     argIndx,
                                        cl_kernel_arg_info paramName,
                                        size_t      szParamValueSize,
                                        void *      pParamValue,
                                        size_t *    pszParamValueSizeRet);

        /**
         * Set whether this Kernel uses pointers that are fine grain system SVM allocations
         * @param bSvmFineGrainSystem whether this Kernel uses pointers that are fine grain system SVM allocations
         */
        void SetSvmFineGrainSystem(bool bSvmFineGrainSystem) { m_bSvmFineGrainSystem = bSvmFineGrainSystem; };

        /**
         * @return whether this Kernel uses pointers that are fine grain system SVM allocations
         */
        bool IsSvmFineGrainSystem() const { return m_bSvmFineGrainSystem; }

        /**
         * Set SVMBuffers that are used by Kernel, but not passed as arguments to it
         * @param svmBufs a vector of SharedPtrs to the SVMBuffers
         */
        void SetNonArgSvmBuffers(const std::vector<SharedPtr<SVMBuffer> >& svmBufs);

        /**
         * @param svmBufs a vector of SVMBuffers that are used by Kernel, but not passed as arguments to it
         */
        void GetNonArgSvmBuffers(std::vector<SharedPtr<SVMBuffer> >& svmBufs) const;
        
        // needed so that DeviceKernel can access the raw program's binary (no const)
        friend class DeviceKernel;

    protected:
        /******************************************************************************************
        * Function:     ~Kernel
        * Description:    The Kernel class destructor
        * Arguments:        
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/            
        virtual ~Kernel();

        //Kernel prototype
        cl_err_code SetKernelPrototype(SKernelPrototype sKernelPrototype);
        SKernelPrototype                        m_sKernelPrototype;

        SharedPtr<Program>                      m_pProgram;
        SharedPtr<Context>                      m_pContext;
        size_t                                  m_szAssociatedDevices;

        // Kernel arguments
        vector<KernelArg>                       m_vecArgs;
        char*                                   m_pArgsBlob;

        // Per-device kernels
        DeviceKernel**                          m_ppDeviceKernels;

        // To ensure all args have been set
        size_t                                  m_numValidArgs;
        size_t                                  m_deviceArgsSize;
        size_t                                  m_totalLocalSize;

        bool                                    m_bSvmFineGrainSystem;
        mutable Intel::OpenCL::Utils::OclReaderWriterLock m_rwlock;
        std::vector<SharedPtr<SVMBuffer> >      m_nonArgSvmBufs;

    };


}}}
