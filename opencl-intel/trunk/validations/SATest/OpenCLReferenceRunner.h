/*****************************************************************************\

Copyright (c) Intel Corporation (2010, 2011).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  OpenCLReferenceRunner.h

\*****************************************************************************/
#ifndef OPEN_CL_REFERENCE_RUNNER_H
#define OPEN_CL_REFERENCE_RUNNER_H

#include <cstddef>                      // for std::size_t
#include <vector>

#include "IProgramRunner.h"
#include "IProgram.h"
#include "IMemoryObject.h"
#include "IBufferContainerList.h"
#include "IRunResult.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/Function.h"
#include "llvm/Type.h"
#include "DynamicLib.h"
#include "OpenCLRunConfiguration.h"
#include "OpenCLProgramConfiguration.h"
#include "cl_types.h"

#include "InterpreterPlugIn.h"
#include "InterpreterPluggable.h"
#include "PlugInNEAT.h"
namespace Validation
{
    class OpenCLKernelConfiguration;

    /// @brief This class enables to run a single OpenCL reference test
    class OpenCLReferenceRunner : public IProgramRunner
    {
    public:
        /// @brief Constructor
        OpenCLReferenceRunner(bool bUseNEAT);

        /// @brief Destructor
        virtual ~OpenCLReferenceRunner(void);

        /// @brief Executes a single test program
        /// @param [OUT] runResult Result of test program execution
        /// @param [IN] program Test program to execute
        /// @param [IN] programConfig Test program configuration options
        /// @param [IN] runConfig Configuration of the test run.
        virtual void Run( IRunResult* runResult, 
                          IProgram * program,
                          IProgramConfiguration* programConfig, 
                          const IRunComponentConfiguration* runConfig);

        /// @brief Load the output from file 
        /// @param [OUT] runResult Result of test program execution
        /// @param [IN] config Configuration of the test run
        virtual void LoadOutput(IRunResult* pRunResult, IProgramConfiguration* pConfig);

        /// @brief Load the output from file 
        /// @param [IN] runResult Result of test program execution
        /// @param [IN] config Configuration of the test run
        virtual void StoreOutput(IRunResult* pRunResult, IProgramConfiguration* pConfig);
    private:
        // TODO: Maybe we should detach copying from inBuffer to outBuffer into
        // another function.
        /// @brief Copies data from inBuffer to outBuffer, checks if data type
        ///        corresponds to argType and returns pointer to outBuffer data.
        ///        The data is copied because it may be overwritten by program, but
        ///        input buffer could be used by other runners and must be
        ///        unchanged.
        /// @param [IN] inBuffer Buffer with input data intended to be passed via
        ///                      pointer.
        /// @param [OUT] outBuffer Buffer with the copy of input data, which will be
        ///                        passed in reality.
        /// @param [IN] argType Type of function argument expected to be passed via
        ///                     pointer.
        /// @return Pointer to the argument value.
        void* GetPointerToTheArgValues( const IMemoryObject* inBuffer, IMemoryObject*
            outBuffer, const llvm::Type* argType );

        /// @brief Reads integer value from buffer of vectors, copies it to the
        ///        outBuffer.
        /// @param [IN] inBuffer            Buffer with integer vector values.
        /// @param [OUT] outBuffer          Buffer with copy of inBuffer data.
        /// @param [IN] vectorId            Vector index in buffer.
        /// @param [IN] elementInVectorId   Element index in vector.
        /// @param [IN] numOfBits           Number of bits in integer value (8, 16,
        ///                                 32 or 64).
        /// @param [OUT] val                Integer value read from inBuffer.
        void ReadIntegerFromBuffer( const IMemoryObject* inBuffer, IMemoryObject* outBuffer,
            const std::size_t vectorId,
            const std::size_t elementInVectorId,
            const unsigned int numOfBits,
            llvm::GenericValue &val );

        /// @brief Extracts LLVM program from 'program' and parse it
        ///        and create a LLVM module.
        /// @param [IN] program   Test program to execute.
        void ParseToModule( const IProgram* program );

        /// @brief Prepares list of arguments to run test program.
        /// @param [OUT] ArgVals  List of LLVM data type values, which contains 
        ///                       references to 'real values' stored into 'Args'.
        /// @param [IN]  input    Data Manager structure containing input argument
        ///                       values.
        /// @param [IN]  kernelName Name of the kernel.
        /// @param [OUT] Args     Data Manager structure containing copy of 
        ///                       argument values to run test program with.
        /// @param [OUT] neatArgs Data Manager structure containing copy of 
        ///                       argument values for neat plug-in.
        void ReadKernelArgs( std::vector<llvm::GenericValue> &ArgVals,
            const IBufferContainerList * input,
            const std::string& kernelName,
            IBufferContainerList * Args,
            IBufferContainerList * neatArgs );

        /// @brief Runs kernel.
        /// @param [INOUT] runResult    Kernel argument values. Some of them are input,
        ///                             some are output and some of them could be both.
        /// @param [IN] pKernelConfig   Configuration containing kernel-specific parameters.
        /// @param [IN] runConfig       Configuration containing kernel run options.
        void RunKernel( IRunResult * runResult,
            OpenCLKernelConfiguration * pKernelConfig,
            const ReferenceRunOptions* runConfig );

        /// @brief Loads the given container from the given file in the given format
        /// @param filename [in] file to load the container from
        /// @param filetype [in] file type of the storage
        /// @param pContainer  [in,out] data container
        void ReadBufferContainer(const std::string& filename, 
            DataFileType filetype, 
            IContainer* pContainer);

        /// @brief Stores the given container to the given file in the given format
        /// @param filename [in] file to store the container into
        /// @param filetype [in] file type of the storage
        /// @param pContainer  [in,out] data container
        void WriteBufferContainer(const std::string& filename, 
            DataFileType filetype, 
            IContainer* pContainer);

        /// @brief Loads the input buffer according to kernel configuration.
        /// @param pKernelConfig    [in] kernel specific configuration
        /// @param pContainer       [in,out] input data container
        void ReadInputBuffer(OpenCLKernelConfiguration* pKernelConfig, IContainer* pContainer );

        /// @brief Loads the reference buffer according to kernel configuration.
        /// @param pKernelConfig    [in] kernel specific configuration
        /// @param pContainer       [in,out] reference data container
        void ReadReferenceBuffer(OpenCLKernelConfiguration* pKernelConfig, IContainer* pContainer );

        /// @brief Loads the NEAT buffer according to kernel configuration.
        /// @param pKernelConfig    [in] kernel specific configuration
        /// @param pContainer       [in,out] NEAT data container
        void ReadNEATBuffer(OpenCLKernelConfiguration* pKernelConfig, IContainer* pContainer );

        /// @brief Stores the reference buffer according to kernel configuration.
        /// @param pKernelConfig    [in] kernel specific configuration
        /// @param pContainer       [in,out] reference data container
        void WriteReferenceBuffer(OpenCLKernelConfiguration* pKernelConfig, IContainer* pContainer );

        /// @brief Stores the NEAT buffer according to kernel configuration.
        /// @param pKernelConfig    [in] kernel specific configuration
        /// @param pContainer       [in,out] NEAT data container
        void WriteNEATBuffer(OpenCLKernelConfiguration* pKernelConfig, IContainer* pContainer );

    private:
        /// Values of kernel arguments which are passed via pointers.
        std::vector<void*> m_pointerArgs;

        llvm::LLVMContext*      m_pLLVMContext;
        llvm::Module*           m_pModule;      // test program module
        llvm::Module*           m_pRTModule;    // run-time module
        llvm::Function*         m_pKernel;      // kernel to run
        llvm::ExecutionEngine*  m_pExecEngine;  // interpreter
        llvm::NEATPlugIn*       m_pNEAT;        // NEAT plug-in
        /// map of OCL program inputs to NEATGenericValue s
        std::map<llvm::Value *, llvm::NEATGenericValue> m_NEATArgValues;
        // pointers to allocated cl_mem_obj_descriptor structures
        std::list<cl_mem_obj_descriptor*> m_ClMemObjScratchMemList;

        /// Generate the NEAT results if specified
        bool m_bUseNEAT;

    };
}

#endif // OPEN_CL_REFERENCE_RUNNER_H
