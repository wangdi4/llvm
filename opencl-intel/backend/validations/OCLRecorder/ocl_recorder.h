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

#include "llvm/Support/Path.h"
#include "llvm/Support/Mutex.h"
#include "llvm/IR/DataLayout.h"
#include "plugin_interface.h"
#include "cl_types.h"
#include "cl_device_api.h"
#include "cl_dev_backend_api.h"
#include "ocl_source_recorder.h"
#define TIXML_USE_STL
#include "tinyxml.h"

#include <atomic>
#include <list>
#include <map>

using namespace Intel::OpenCL;

#ifdef __cplusplus
extern "C"
{
#endif
///\brief exported creator method - used to create the OCLRecorder plug-in
OCL_RECORDER_API IPlugin* CreatePlugin(void);
///\brief exported destructor method - used to delete the OCLRecorder plug-in
OCL_RECORDER_API void ReleasePlugin(IPlugin* pPlugin);
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
        RecorderContext(const std::string& logsPath, const std::string& prefix);

        ~RecorderContext();

        ///\brief returns the base file name
        const std::string getBaseName() const;

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
        llvm::DataLayout *m_DL;

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

        static std::atomic<unsigned> s_counter;
    };

    class HashComparator
    {
    public:
        bool operator() (const MD5Code& lhs, const MD5Code& rhs) const
        {
            int result = memcmp(lhs.code(), rhs.code(), 16);
            return (result < 0)? true: false;
        }
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
        OCLRecorder( const std::string& logsDir, const std::string& prefix );
        ~OCLRecorder();

    public:
        virtual void OnCreateBinary(const ICLDevBackendKernel_* pKernel,
                                    const _cl_work_description_type* pWorkDesc,
                                    size_t bufSize,
                                    void* pArgsBuffer);

        virtual void OnCreateKernel(const ICLDevBackendProgram_* pProgram,
                                    const ICLDevBackendKernel_* pKernel,
                                    const void* pFunction);

        virtual void OnCreateProgram(const void * pBinary,
                                     size_t uiBinarySize,
                                     const ICLDevBackendProgram_* pProgram);

        virtual void OnReleaseProgram(const ICLDevBackendProgram_* pProgram);

        void SetSourceRecorder(const OclSourceRecorder* recorder);

    private: // Internal method
        void RecordByteCode(const void* pBinary, size_t uiBinarySize, const RecorderContext& context);

        void RecordProgramConfig(RecorderContext& context);

        void RecordSourceCode(RecorderContext& context,
                              const Frontend::SourceFile& sourceFile);

        void RecordKernelConfig(RecorderContext& programContext,
                                const KernelContext& kernelContext,
                                const BinaryContext& binaryContext,
                                const std::string& pathToDataInputFile);

        void RecordKernelInputs(const RecorderContext& programContext,
                                const KernelContext& kernelContext,
                                const BinaryContext& binaryContext,
                                const ICLDevBackendKernel_* pKernel,
                                size_t bufSize,
                                void* pArgsBuffer,
                                std::string& pathToDataInputFile);
        //adds a file name to be recorded in this context
        //
        void AddRecordedFile(const std::string&);
        //indicates whether the given file name has allredy been recorded
        //within this context
        //
        bool IsRecordedFile(const std::string&)const;

        const std::string* GetPathToInputData(const MD5Code&) const;

    private: // Utility methods

        // XML configuration handling helper
        TiXmlElement* AddChildTextNode( TiXmlElement* pParentNode, const char* childName, const std::string& value);

        // context map synchronized manipulation methods
        RecorderContext* GetProgramContext(const ICLDevBackendProgram_* pProgram);

        RecorderContext* GetProgramContextForKernel(const ICLDevBackendKernel_* pKernel);

        void AddNewProgramContext(const ICLDevBackendProgram_* pProgram, RecorderContext* pContext);

        void RemoveProgramContext(const ICLDevBackendProgram_* pProgram);

        // kernel context methods

        BinaryContext* GetKernelContext(ICLDevBackendKernel_* pKernel);

        //Parameter(s):
        // (in): code- the md5 hash code of the bytecode that serves as an
        // entry point for the program.
        // (out): pSourceFile- source file ebing compiled, to be purged to the
        // configuration file. (valid pointer only if this method returns true).
        //Returns true, if we have a all the pre-requisits for source-level recordings.
        bool NeedSourceRecording(const MD5Code& code,
                                 OUT Frontend::SourceFile* pSourceFile) const;

    private: // Data members
        typedef std::map<const ICLDevBackendProgram_*,  RecorderContext*> RecorderContextMap;

        //files names that has been recorded in this session (avoid overruns)
        std::vector<std::string> m_recordedFiles;
        std::map<MD5Code, std::string, HashComparator> m_hashToPath; //container for saved hash of used data
        RecorderContextMap m_contexts;     // container for program specific contexts
        llvm::sys::Mutex   m_contextsLock; // synchronization for the contexts container
        std::string        m_logsDir;      // optional path to log directory
        std::string        m_prefix;       // optional prefix to add to generated files
        const OclSourceRecorder* m_pSourceRecorder; //holds source-level recording data
    };
}
