/*****************************************************************************\

Copyright (c) Intel Corporation (2010,2011).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  ocl_recorder.h

\*****************************************************************************/

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the OCL_RECORDER_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// OCL_RECORDER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#if defined(_WIN32)
    #ifdef OCL_RECORDER_EXPORTS
        #define OCL_RECORDER_API __declspec(dllexport)
    #else
        #define OCL_RECORDER_API __declspec(dllimport)
    #endif
#else
    #define OCL_RECORDER_API
#endif


#include "llvm/System/Path.h"
#include "llvm/System/Atomic.h"
#include "llvm/System/Mutex.h"
#include "llvm/Target/TargetData.h"
#include <list>
#include "plugin_interface.h"
#include "cl_types.h"
#include "cl_device_api.h"
#include "cl_dev_backend_api.h"
#define TIXML_USE_STL
#include "tinyxml.h"

using namespace Intel::OpenCL::DeviceBackend;

#ifdef __cplusplus
extern "C"
{
#endif
///\brief exported creator method - used to create the OCLRecorder plug-in
OCL_RECORDER_API ICLDevBackendPlugin* CreatePlugin(void);
///\brief exported destructor method - used to delete the OCLRecorder plug-in
OCL_RECORDER_API void ReleasePlugin(ICLDevBackendPlugin* pPlugin);
#ifdef __cplusplus
}
#endif

namespace llvm
{
    class Function;
}


namespace Validation
{
    ///\brief Context maintained for each captured kernel's binary.
    ///       The main purpose to maintain this information is to
    ///       ensure that kernel work size parameters will be
    ///       captured only once (regardless of multiple CreateBinary calls)
    class BinaryContext
    {
    public:
        BinaryContext():
            m_index(0)
        {
            m_workDesc.workDimension = 0;
            for( size_t i = 0; i < MAX_WORK_DIM; ++i )
            {
                m_workDesc.globalWorkSize[i] = 0;
                m_workDesc.localWorkSize[i] = 0;
                m_workDesc.globalWorkOffset[i] = 0;
            }
        }

        ///\brief Ctor
        BinaryContext( const std::string& name,
                       const _cl_work_description_type* workDesc,
                       int index):
            m_name(name),
            m_workDesc(*workDesc),
            m_index(index)
        {
        }

        bool operator == ( const BinaryContext& rhs ) const
        {
            if( m_workDesc.workDimension != rhs.m_workDesc.workDimension)
            {
                return false;
            }

            for( size_t i = 0; i < m_workDesc.workDimension; ++i )
            {
                if( m_workDesc.globalWorkSize[i] != rhs.m_workDesc.globalWorkSize[i] ||
                    m_workDesc.localWorkSize[i]  != rhs.m_workDesc.localWorkSize[i])
                {
                    return false;
                }
            }

            return true;
        }


        const std::string getBaseName( ) const;

        unsigned int  getWorkDimention() const { return m_workDesc.workDimension; }

        const size_t* getGlobalWorkSize() const { return m_workDesc.globalWorkSize; }

        const size_t* getLocalWorkSize() const { return m_workDesc.localWorkSize; }

        const size_t* getGlobalWorkOffset() const { return m_workDesc.globalWorkOffset; }


    public:
        std::string m_name;
        cl_work_description_type m_workDesc;
        int         m_index;
    };


    class KernelContext
    {
    public:
        KernelContext():
            m_pFunc(NULL)
        {}

        KernelContext( const std::string& name, const llvm::Function* pFunc):
            m_name(name),
            m_pFunc(pFunc)
        {}

        bool operator == ( const KernelContext& rhs ) const
        {
            return m_name == rhs.m_name;
        }

        ///\brief Creates or update the increase the counts of kernels in the kernel context
        BinaryContext* getOrCreateBinaryContext(const char* name,
                                                const _cl_work_description_type* workDesc,
                                                bool& created);

        const std::string& getName() const { return m_name; }

        const llvm::Function* getFuncPtr() const { return m_pFunc; }

    private:
        typedef std::list<BinaryContext> BinaryContextList;

        std::string m_name;
        const llvm::Function* m_pFunc;
        BinaryContextList m_binaries;
    };


    ///\brief Context maintained for each program during recording
    class RecorderContext
    {
    public:
        ///\brief Ctor
        ///\param logsPath [in] path to store all the recorded output
        ///\param prefix   [in] prefix used for the recorder files
        RecorderContext( const llvm::sys::Path& logsPath, const std::string& prefix);

        ~RecorderContext();

        ///\brief returns the file path for byte code file
        const std::string getByteCodeFilePath() const;

        ///\brief returns the file name for byte code file
        const std::string getByteCodeFileName() const;

        ///\brief returns the file path for configuration file
        const std::string getConfigFilePath() const;

        ///\brief returns the file path for input data for given kernel
        const std::string getInputFilePath( const std::string& kernelName ) const;

        ///\brief returns the file name for input data for given kernel
        const std::string getInputFileName( const std::string& kernelName ) const;

        ///\brief returns the file path for reference data for given kernel
        const std::string getReferenceFilePath( const std::string& kernelName ) const;

        ///\brief returns the file name for reference data for given kernel
        const std::string getReferenceFileName( const std::string& kernelName ) const;

        ///\brief returns the file path for neat data for given kernel
        const std::string getNeatFilePath( const std::string& kernelName ) const;

        ///\brief returns the file name for neat data for given kernel
        const std::string getNeatFileName( const std::string& kernelName ) const;

        ///\brief Flushes the open files to the file system
        void Flush();

        ///\brief Creates or update the increase the counts of kernels in the kernel context
        void createKernelContext(const ICLDevBackendKernel_* pKernel,
                                 const llvm::Function* pFunction);

        bool containsKernel( const ICLDevBackendKernel_* pKernel );

        KernelContext* getKernelContext(const ICLDevBackendKernel_* pKernel );

        TiXmlDocument   m_config;
        TiXmlElement   *m_pRunConfig;
        llvm::sys::Mutex m_configLock;
        llvm::TargetData *m_TD;

    private:

        const std::string getPath( const std::string& suffix ) const;

        const std::string getPath( const std::string& kernelName,  const std::string& suffix ) const;

        const std::string getFileName( const std::string& suffix ) const;

        const std::string getFileName( const std::string& kernelName, const std::string& suffix  ) const;

        typedef std::map<const ICLDevBackendKernel_*, KernelContext> KernelContextMap;

        std::string         m_baseName;
        std::string         m_logsPath;
        KernelContextMap    m_kernels;
        llvm::sys::Mutex    m_kernelsLock; // synchronization for the kernels container

        static llvm::sys::cas_flag s_counter;
    };

    ///
    ///\brief OCL Recorder plug-in
    ///
    ///OCL BackEnd Recorder is a component that will be responsible for capturing the bytecode,
    ///input data and execution parameters passed through the OCL back-end. This data could be
    ///then used to replay/rerun the captured kernel for testing and debugging purposes.
    ///
    ///The component will be implemented as a plug-in to the current OCL BackEnd, so that it could be activated only when needed.
    ///
    ///The captured data will be stored in predefined persisted storage location in format that is currently supported by SATest.
    class OCLRecorder: public Intel::OpenCL::DeviceBackend::ICLDevBackendPlugin
    {
    public:
        ///\brief Ctor
        ///\param logsPath [in] path to store all the recorded output
        ///\param prefix   [in] prefix used for the recorder files
        OCLRecorder( const llvm::sys::Path& logsDir, const std::string& prefix );
        ~OCLRecorder();

    public:
        virtual void OnCreateBinary(const ICLDevBackendKernel_* pKernel,
                                    const _cl_work_description_type* pWorkDesc,
                                    size_t bufSize,
                                    void* pArgsBuffer);

        virtual void OnCreateKernel(const ICLDevBackendProgram_* pProgram,
                                    const ICLDevBackendKernel_* pKernel,
                                    const llvm::Function* pFunction);

        virtual void OnCreateProgram(const _cl_prog_container_header* pContainer,
                                     const ICLDevBackendProgram_* pProgram);

        virtual void OnReleaseProgram(const ICLDevBackendProgram_* pProgram);

    private: // Internal method
        void RecordByteCode(const _cl_prog_container_header* pContainer, const RecorderContext& context);

        void RecordProgramConfig(RecorderContext& context);

        void RecordKernelConfig(RecorderContext& programContext,
                                const KernelContext& kernelContext,
                                const BinaryContext& binaryContext);

        void RecordKernelInputs(const RecorderContext& programContext,
                                const KernelContext& kernelContext,
                                const BinaryContext& binaryContext,
                                const ICLDevBackendKernel_* pKernel,
                                size_t bufSize,
                                void* pArgsBuffer);
    private: // Utility methods

        // XML configuration handling helper
        void AddChildTextNode( TiXmlElement* pParentNode, const char* childName, const std::string& value);

        // context map synchronized manipulation methods
        RecorderContext* GetProgramContext(const ICLDevBackendProgram_* pProgram);

        RecorderContext* GetProgramContextForKernel(const ICLDevBackendKernel_* pKernel);

        void AddNewProgramContext(const ICLDevBackendProgram_* pProgram, RecorderContext* pContext);

        void RemoveProgramContext(const ICLDevBackendProgram_* pProgram);

        // kernel context methods

        BinaryContext* GetKernelContext(ICLDevBackendKernel_* pKernel);

#if 0
        void AddNewKernelContext(RecorderContext& context,
                                 const char *name,
                                 size_t dim,
                                 const size_t *globalSize,
                                 const size_t *localSize,
                                 const size_t *globalOffset);
#endif

    private: // Data members
        typedef std::map<const ICLDevBackendProgram_*,  RecorderContext*> RecorderContextMap;

        RecorderContextMap m_contexts;     // container for program specific contexts
        llvm::sys::Mutex   m_contextsLock; // synchronization for the contexts container
        llvm::sys::Path    m_logsDir;      // optional path to log directory
        std::string        m_prefix;       // optional prefix to add to generated files
        llvm::LLVMContext* m_pLLVMContext; // context used to re-parse the recorder modules
    };
}


