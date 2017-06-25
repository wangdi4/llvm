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
#include <memory>
//#include "svm_buffer.h"

namespace Intel { namespace OpenCL { namespace Framework {

    class Program;
    class DeviceProgram;
    class Kernel;
    class Context;
    class SVMBuffer;
    class SVMPointerArg;

    /*! \struct cl_kernel_arg_info
     *  \brief Defines extended information for a kernel arguments.
     */
    struct SKernelArgumentInfo
    {
        std::string                     name;               //!< String specifies the name of the argument
        std::string                     typeName;           //!< String specifies the argument type
        cl_kernel_arg_address_qualifier adressQualifier;    //!< Argument's address qualifier
        cl_kernel_arg_access_qualifier  accessQualifier;    //!< Argument's access qualifier
        cl_kernel_arg_type_qualifier    typeQualifier;      //!< Argument's type qualifier
    };

    /**********************************************************************************************
    * Class name:    SKernelPrototype
    *
    * Description:    contains information on kernel prototype
    * Members:        m_psKernelName    - the name of the kernel
    *                 m_uiArgsCount    - number of arguments in the kernel
    *                 m_pArgs            - list of all kernel's arguments
    *                 m_szKernelAttributes - space seperated list of kernel attributes
    *
    * Author:         Uri Levy
    * Date:           January 2008
    **********************************************************************************************/    
    struct SKernelPrototype
    {
        std::string                     m_szKernelName;
        cl_dev_dispatch_buffer_prop     m_dispatchBufferProperties;
        std::vector<cl_kernel_argument> m_vArguments;
        std::vector<cl_uint>            m_MemArgumentsIndx;
        std::string                     m_szKernelAttributes;
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
        const SKernelPrototype& GetPrototype() const { return m_sKernelPrototype; }

        // compare between kernel's prototypes
        bool CheckKernelDefinition(const DeviceKernel * pKernel) const;

        // get prefetched info
        size_t          GetKernelWorkGroupSize() const { return m_CL_KERNEL_WORK_GROUP_SIZE; }
        const size_t*   GetKernelCompileWorkGroupSize() const { return m_CL_KERNEL_COMPILE_WORK_GROUP_SIZE; }
        cl_ulong        GetKernelLocalMemSize() const { return m_CL_KERNEL_LOCAL_MEM_SIZE; }
        cl_bool         GetKernelNonUniformWGSizeSupport() const { return m_CL_KERNEL_NON_UNIFORM_WG_SIZE_SUPPORT; }
        size_t          GetKernelArgBufferSize() const { return m_sKernelPrototype.m_dispatchBufferProperties.size - m_sKernelPrototype.m_dispatchBufferProperties.argumentOffset;}
        size_t          GetKernelDispatchBufferSize() const { return m_sKernelPrototype.m_dispatchBufferProperties.size;}
        size_t          GetArgumentOffset() const { return m_sKernelPrototype.m_dispatchBufferProperties.argumentOffset;}
        size_t          GetKernelArgBufferAlignment() const { return m_sKernelPrototype.m_dispatchBufferProperties.alignment;}

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
        cl_bool                                 m_CL_KERNEL_NON_UNIFORM_WG_SIZE_SUPPORT;

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

        KernelArg() : m_pValueLocation(NULL), m_bValid(false) {}
        
        void Init( char* baseAddress, const cl_kernel_argument& clKernelArgType );

        cl_uint             GetOffset() const { return m_clKernelArgType.offset_in_bytes; }

        // return the size (in bytes) of the kernel arg's value
        // if Buffer / Image / ... returns sizeof(MemoryObject*)
        size_t              GetSize()          const { return  m_clKernelArgType.size_in_bytes; }
        cl_kernel_arg_type  GetType()          const { return m_clKernelArgType.type; }

        // returns the value of the kernel argument
        void                GetValue( size_t size, void* pValue ) const;
        void                SetValue( size_t size, const void* pValue );
        void                SetValid() { m_bValid = true; }
        void                SetSvmObject( const SharedPtr<ReferenceCountedObject>& svmMemObj ) { m_pSvmPtrArg = svmMemObj; }

        size_t              GetLocalBufferSize() const
            { assert( IsLocalPtr() && (NULL!=m_pValueLocation) && "Not a local PTR or value location is not set"); return *(size_t*)m_pValueLocation; }

        bool                IsMemObject()     const { return (CL_KRNL_ARG_PTR_GLOBAL  <= m_clKernelArgType.type); }
        bool                IsBuffer()        const { return ((CL_KRNL_ARG_PTR_GLOBAL == m_clKernelArgType.type) ||
                                                              (CL_KRNL_ARG_PTR_CONST  == m_clKernelArgType.type)); }
        bool                IsImage()         const { return ((CL_KRNL_ARG_PTR_IMG_2D     <= m_clKernelArgType.type) &&
                                                              (CL_KRNL_ARG_PTR_IMG_1D_BUF >= m_clKernelArgType.type)); }
        bool                IsSampler()       const { return (IsOpaqueSampler() || IsInt32Sampler()); }

        bool                IsOpaqueSampler() const { return (CL_KRNL_ARG_PTR_SAMPLER_T == m_clKernelArgType.type); }
        bool                IsInt32Sampler()  const { return (   CL_KRNL_ARG_SAMPLER    == m_clKernelArgType.type); }

        bool                IsLocalPtr()      const { return (CL_KRNL_ARG_PTR_LOCAL == m_clKernelArgType.type); }
        

        bool                IsValid()         const { return m_bValid; }

        bool                IsSvmPtr()        const { return (0 != m_pSvmPtrArg); }

        bool                IsQueueId()       const { return CL_KRNL_ARG_PTR_QUEUE_T == m_clKernelArgType.type; }

    private:
        void SetValuePlaceHolder( void * pValuePlaceHolder, size_t offset );


        // type of kernel argument
        cl_kernel_argument              m_clKernelArgType;

        void*                           m_pValueLocation;
        bool                            m_bValid;

        SharedPtr<ReferenceCountedObject> m_pSvmPtrArg;    // we hold a SharedPtr to ReferenceCountedObject because of header dependencies
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
        * Function:      GetSubGroupInfo
        *
        * Description:   returns information about the kernel object that may be specific to a
        *                device
        *
        * Arguments:     device                [in]    identifies a specific device in the list of
        *                                              devices associated with
        *                iParamName            [in]    parameter's name
        *                szParamValueSize      [in]    parameter's value size (in bytes)
        *                input_value_size      [in]    input value's size
        *                input_value           [in]    input value's value
        *                pParamValue           [out]   parameter's value
        *                pszParamValueSizeRet  [out]   parameter's value return size
        * Return value:  CL_INVALID_DEVICE - device is not valid device
        *                CL_INVALID_VALUE  - iParamName is not valid parameter
        *                CL_SUCCESS        - operation succeeded
        * Author:        Vlad Romanov
        * Date:          October 2015
        ******************************************************************************************/
        cl_err_code GetSubGroupInfo(const       SharedPtr<FissionableDevice>& device,
                                    cl_int      iParamName,
                                    size_t      szParamValueSize,
                                    size_t      input_value_size,
                                    const void* input_value,
                                    void *      pParamValue,
                                    size_t*     pszParamValueSizeRet);
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
        cl_err_code CreateDeviceKernels(std::vector<unique_ptr<DeviceProgram>>& pDevicePrograms);

        // set kernel argument without checks.
        cl_err_code SetKernelArgInternal(cl_uint uiIndex, const KernelArg* arg);
        // set kernel argument
        cl_err_code SetKernelArg(cl_uint uiIndex, size_t szSize, const void * pValue, bool bIsSvmPtr = false);

        // returns the number of arguments in the kernel
        size_t GetKernelArgsCount() const { return m_sKernelPrototype.m_vArguments.size(); }

        // returns memory object argument number in the kernel
        size_t GetKernelMemoryArgsCount() const { return m_sKernelPrototype.m_MemArgumentsIndx.size();}

        // Return true if all kernel arguments were specified
        bool IsValidKernelArgs() const { return m_numValidArgs == m_sKernelPrototype.m_vArguments.size(); }

        // Return size in bytes for device arguments area
        size_t GetTotalLocalSize() const { return m_totalLocalSize; }
        void*  GetArgsBlob()       const { return m_pArgsBlob; }

        const KernelArg* GetKernelArg(size_t uiIndex) const 
        { 
            assert (uiIndex < m_sKernelPrototype.m_vArguments.size() && "Invalide argument index"); 
            return &(m_vecArgs[uiIndex]); 
        }

        const KernelArg* GetKernelMemoryArg(size_t uiIndex) const
        {
            assert (uiIndex < m_sKernelPrototype.m_MemArgumentsIndx.size() && "Invalide argument index"); 
            return &(m_vecArgs[m_sKernelPrototype.m_MemArgumentsIndx[uiIndex]]); 
        }
        // Returns non zero handle.
        cl_dev_kernel GetDeviceKernelId(const FissionableDevice* pDevice) const;
        const DeviceKernel* GetDeviceKernel(const FissionableDevice* pDevice) const;

        // get kernel's name
        const char * GetName() const { return m_sKernelPrototype.m_szKernelName.c_str(); }

        // get kernel's associated program
        ConstSharedPtr<Program> GetProgram() const { return m_pProgram; }
        const SharedPtr<Program>& GetProgram()     { return m_pProgram; }

        ConstSharedPtr<Context> GetContext() const { return m_pContext; }
        const SharedPtr<Context>& GetContext()     { return m_pContext; }

        const char *GetAttributes() const { return m_sKernelPrototype.m_szKernelAttributes.c_str(); }

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

        size_t GetNonArgSvmBuffersCount() const;
        
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
        cl_err_code SetKernelPrototype(const SKernelPrototype& sKernelPrototype, size_t maxArgumentBufferSize, size_t maxArgumentBufferAlignment);
        cl_err_code SetKernelArgumentInfo(const DeviceKernel* pDeviceKernel); 

        SKernelPrototype                        m_sKernelPrototype;

        SharedPtr<Program>                      m_pProgram;
        SharedPtr<Context>                      m_pContext;
        size_t                                  m_szAssociatedDevices;

        // Kernel arguments
        vector<KernelArg>                       m_vecArgs;
        char*                                   m_pArgsBlob;

        // Per-device kernels
        vector<const DeviceKernel*>             m_vpDeviceKernels;

        // To ensure all args have been set
        size_t                                  m_numValidArgs;
        size_t                                  m_totalLocalSize;

        std::vector<SKernelArgumentInfo>        m_vArgumentsInfo;

        bool                                                m_bSvmFineGrainSystem;
        mutable Intel::OpenCL::Utils::OclReaderWriterLock   m_rwlock;
        std::vector<SharedPtr<SVMBuffer> >                  m_nonArgSvmBufs;

    private:
        
        /******************************************************************************************
        * Function:     Kernel
        * Description:    The Kernel class constructor
        * Arguments:    pProgram [in]        - associated program object
        *                psKernelName [in]    - kernel's name
        * Author:        Uri Levy
        * Date:            January 2008
        ******************************************************************************************/        
        Kernel(const SharedPtr<Program>& pProgram, const char * psKernelName, size_t szNumDevices);


        // disable possibility to create two instances of Kernel with the same m_pArgsBlob pointer.
        Kernel(const Kernel& s);
        Kernel& operator=(const Kernel& s);
    };


}}}
