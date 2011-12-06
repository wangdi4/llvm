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

File Name:  OpenCLReferenceRunner.cpp

\*****************************************************************************/

#define DEBUG_TYPE "OpenCLReferenceRunner"

#include <string>
#include <exception>
#include <vector>
#include <list>
#include <algorithm>
#include "mem_utils.h"
using std::exception;
#include "cpu_dev_limits.h"
#include "llvm/LLVMContext.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Function.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
// debug macros
#include "llvm/Support/Debug.h"
// Command line options
#include "llvm/Support/CommandLine.h"

#include "RunResult.h"
#include "OpenCLReferenceRunner.h"
#include "OpenCLRunConfiguration.h"
#include "OCLUpdatePass.h"
#include "SATestException.h"
#include "OpenCLProgram.h"
#include "cl_device_api.h"
#include "SystemInfo.h"
#include "CPUDetect.h"
#include "Buffer.h"
#include "XMLDataWriter.h"
#include "XMLDataReader.h"
#include "BinaryDataReader.h"
#include "BinaryDataWriter.h"
#include "BufferContainerList.h"
#include "OpenCLArgsBuffer.h"
#include "ContainerCopier.h"

#include "InterpreterPlugIn.h"
#include "InterpreterPluggable.h"
#include "PlugInNEAT.h"
using namespace llvm;
using std::string;

extern "C" void LLVMLinkInInterpreterPluggable();

using namespace Validation;
using namespace Validation::Exception;
using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;
using namespace Intel::OpenCL::DeviceBackend::Utils;

//headers
#if defined(__LP64__)
#include <linux/limits.h>
#define MAX_PATH PATH_MAX
#elif defined(_WIN32)
#include <windows.h>
#endif

static const size_t MAX_LOCAL_MEMORY_SIZE = 30000;

OpenCLReferenceRunner::OpenCLReferenceRunner(bool bUseNEAT):
    m_pLLVMContext(NULL),
    m_pModule(NULL),
    m_pRTModule(NULL),
    m_pKernel(NULL),
    m_pExecEngine(NULL),
    m_pNEAT(NULL),
    m_bUseNEAT(bUseNEAT)
{
}

OpenCLReferenceRunner::~OpenCLReferenceRunner(void)
{
    for(std::list<cl_mem_obj_descriptor*>::iterator it = m_ClMemObjScratchMemList.begin();
        it != m_ClMemObjScratchMemList.end();
        ++it)
    {
        align_free(*it);
    }
    m_ClMemObjScratchMemList.clear();
}


void OpenCLReferenceRunner::Run(IRunResult* runResult,
                                IProgram* program,
                                IProgramConfiguration* programConfig,
                                const IRunComponentConfiguration* runConfig)
{
    OpenCLProgramConfiguration *pProgramConfig = static_cast<OpenCLProgramConfiguration *>(programConfig);
    const ReferenceRunOptions *pRunConfig = static_cast<const ReferenceRunOptions *>(runConfig);

    ParseToModule(program);

    for(OpenCLProgramConfiguration::KernelConfigList::const_iterator it = pProgramConfig->beginKernels();
        it != pProgramConfig->endKernels();
        ++it )
    {
        std::string kernelName = (*it)->GetKernelName();
        RunKernel( runResult, *it, pRunConfig );
    }
}

void OpenCLReferenceRunner::LoadOutput(IRunResult* pRunResult, IProgramConfiguration* pConfig)
{
    OpenCLProgramConfiguration *pProgramConfig = static_cast<OpenCLProgramConfiguration *>(pConfig);

    for(OpenCLProgramConfiguration::KernelConfigList::const_iterator it = pProgramConfig->beginKernels();
        it != pProgramConfig->endKernels();
        ++it )
    {
        std::string kernelName = (*it)->GetKernelName();
        ReadReferenceBuffer(*it, &pRunResult->GetOutput( kernelName.c_str()));

        if( m_bUseNEAT )
        {
            ReadNEATBuffer(*it, &pRunResult->GetNEATOutput(kernelName.c_str()));
        }
    }
}

void OpenCLReferenceRunner::StoreOutput(IRunResult* pRunResult, IProgramConfiguration* pConfig)
{
    typedef std::map<std::string, uint32_t> BcIdxMap;

    OpenCLProgramConfiguration *pProgramConfig = static_cast<OpenCLProgramConfiguration *>(pConfig);

    /// Map to track how many buffer containers were saved to files for different kernels
    BcIdxMap bcIndex;
    BcIdxMap::iterator bcIdIter;

    for(OpenCLProgramConfiguration::KernelConfigList::const_iterator it = pProgramConfig->beginKernels();
        it != pProgramConfig->endKernels();
        ++it )
    {
        std::string kernelName = (*it)->GetKernelName();
        /// Search for kernel name in the map
        bcIdIter = bcIndex.find(kernelName);
        int bcIdx;
        /// If it is the first time we execute this kernel
        if(bcIdIter == bcIndex.end())
        {
            /// Add this kernel to the map and set index of last
            /// saved buffer to zero
            bcIndex.insert(BcIdxMap::value_type(kernelName, 0));
            bcIdx = 0;
        }
        else
        {
            /// This kernel was run multiple times. We saved data for previous executions
            /// Now we should save the next buffer container
            (*bcIdIter).second++;
            bcIdx = (*bcIdIter).second;
        }

        /// Create Buffer container list for current kernel name and
        /// Execution index
        ContainerCopier cc;
        IBufferContainerList* pBcl = &pRunResult->GetOutput( kernelName.c_str() );
        std::auto_ptr<IBufferContainerList> pNewBcl(cc.CloneBufferContainer(pBcl, bcIdx));
        /// Save it to file
        WriteReferenceBuffer( *it, pNewBcl.get());

        if( m_bUseNEAT )
        {
            pBcl = &pRunResult->GetNEATOutput( kernelName.c_str() );
            std::auto_ptr<IBufferContainerList> pNewBclNeat(cc.CloneBufferContainer(pBcl, bcIdx));
            WriteNEATBuffer(*it, pNewBclNeat.get());
        }
    }
}

void OpenCLReferenceRunner::ReadBufferContainer(const std::string& filename,
                                                DataFileType filetype,
                                                IContainer* pContainer)
{
    switch( filetype )
    {
    case Binary:
        {
            BinaryContainerListReader reader( filename );
            reader.Read(pContainer);
            break;
        }
    case Xml:
        {
            XMLBufferContainerListReader reader( filename );
            reader.Read(pContainer);
            break;
        }
    default:
        throw TestReferenceRunnerException("Unsupported input file type");
    }
}

void OpenCLReferenceRunner::WriteBufferContainer(const std::string& filename,
                                                 DataFileType filetype,
                                                 IContainer* pContainer)
{
    switch( filetype )
    {
    case Binary:
        {
            BinaryContainerListWriter writer( filename );
            writer.Write(pContainer);
            break;
        }
    case Xml:
        {
            XMLBufferContainerListWriter writer( filename );
            writer.Write(pContainer);
            break;
        }
    default:
        throw TestReferenceRunnerException("Unsupported input file type");
    }
}



void OpenCLReferenceRunner::ReadInputBuffer(OpenCLKernelConfiguration* pKernelConfig, IContainer* pContainer )
{
    assert( NULL != pKernelConfig);
    assert( NULL != pContainer);

    ReadBufferContainer(pKernelConfig->GetInputFilePath(),
                        pKernelConfig->GetInputFileType(),
                        pContainer);
}

void OpenCLReferenceRunner::ReadReferenceBuffer(OpenCLKernelConfiguration* pKernelConfig, IContainer* pContainer )
{
    assert( NULL != pKernelConfig);
    assert( NULL != pContainer);

    std::string filename = pKernelConfig->GetReferenceFilePath();
    if( filename.empty() )
    {
        throw Exception::IOError("BinaryContainerListReader: cannot open reference file");
    }

    ReadBufferContainer(pKernelConfig->GetReferenceFilePath(),
                        pKernelConfig->GetReferenceFileType(),
                        pContainer);
}

void OpenCLReferenceRunner::ReadNEATBuffer(OpenCLKernelConfiguration* pKernelConfig, IContainer* pContainer )
{
    assert( NULL != pKernelConfig);
    assert( NULL != pContainer);

    std::string filename = pKernelConfig->GetNeatFilePath();
    if( filename.empty() )
    {
        throw Exception::IOError("BinaryContainerListReader: cannot open NEAT file");
    }

    ReadBufferContainer(pKernelConfig->GetNeatFilePath(),
                        pKernelConfig->GetNeatFileType(),
                        pContainer);
}



void OpenCLReferenceRunner::WriteReferenceBuffer(OpenCLKernelConfiguration* pKernelConfig, IContainer* pContainer )
{
    assert( NULL != pKernelConfig);
    assert( NULL != pContainer);

    std::string filename = pKernelConfig->GetReferenceFilePath();
    if( filename.empty() )
        return;

    WriteBufferContainer(pKernelConfig->GetReferenceFilePath(),
                         pKernelConfig->GetReferenceFileType(),
                         pContainer);
}

void OpenCLReferenceRunner::WriteNEATBuffer(OpenCLKernelConfiguration* pKernelConfig, IContainer* pContainer )
{
    assert( NULL != pKernelConfig);
    assert( NULL != pContainer);

    std::string filename = pKernelConfig->GetNeatFilePath();
    if( filename.empty() )
        return;

    WriteBufferContainer(pKernelConfig->GetNeatFilePath(),
                         pKernelConfig->GetNeatFileType(),
                         pContainer);
}



void OpenCLReferenceRunner::ReadKernelArgs(
    std::vector<llvm::GenericValue> &ArgVals,
    const IBufferContainerList * input,
    const std::string& kernelName,
    IBufferContainerList * Args,
    IBufferContainerList * neatArgs)
{
    // Extract 'kernel' function from program
    NamedMDNode* metadata = m_pModule->getNamedMetadata("opencl.kernels");
    if ( NULL == metadata )
    {
        throw TestReferenceRunnerException("There are no OpenCL kernels in the\
                                           program");
    }

    // WARNING! Get the first buffer container ONLY. Assume that it contains
    // input arguments.
    IBufferContainer* currContainer = input->GetBufferContainer(0);
    if ( NULL == currContainer )
    {
        throw TestReferenceRunnerException("There are no input data for the\
                                           program.");
    }
    // Copy of arguments for reference runner.
    IBufferContainer* outputContainer =  Args->CreateBufferContainer();
    if ( NULL == currContainer )
    {
        throw TestReferenceRunnerException("Unable to create buffer container\
                                           for program arguments.");
    }

    for (uint32_t k = 0, e = metadata->getNumOperands(); k != e; ++k)
    {
        // Obtain kernel function from annotation
        MDNode *elt = metadata->getOperand(k);

        m_pKernel = dyn_cast<Function>(elt->getOperand(0)->stripPointerCasts());
        if ( NULL == m_pKernel )
        {
            continue;   // Not a function pointer
        }
        // if not needed name skip
        if ( m_pKernel->getNameStr() != kernelName && !kernelName.empty())
        {
            m_pKernel = NULL;
            continue;
        }

        // Obtain parameters definition and prepare argument values.
        const std::size_t numOfArguments = currContainer->GetMemoryObjectCount();

        Function::arg_iterator arg_it = m_pKernel->arg_begin();
        for (std::size_t i = 0;
            arg_it != m_pKernel->arg_end() && i < numOfArguments;
            ++arg_it, ++i)
        {
            IMemoryObject* currBuffer = currContainer->GetMemoryObject(i);
            GenericValue currArg;

            if(Image::GetImageName() == currBuffer->GetName())
            {   // image

                // TODO: This code is almost identical to the next branch. Rewrite it using common function.
                // Kernel argument is an image - need to pass a pointer in the arguments buffer
                ImageDesc imageDesc = GetImageDescription(currBuffer->GetMemoryObjectDesc());
                size_t imageSize = imageDesc.GetImageSizeInBytes();
                IMemoryObject* outputImage = outputContainer->CreateImage(imageDesc);

                // copy image data
                ::memcpy(outputImage->GetDataPtr(), currBuffer->GetDataPtr(), imageSize);

                // Kernel execution assumes all buffer arguments are aligned
                // If we do not align the buffer the execution crashes
                auto_ptr_ex<cl_mem_obj_descriptor> spMemDesc((cl_mem_obj_descriptor*)align_malloc(sizeof(cl_mem_obj_descriptor), CPU_DEV_MAXIMUM_ALIGN));
                FillMemObjDescriptor( *spMemDesc.get(), imageDesc, outputImage->GetDataPtr());
                currArg.PointerVal = spMemDesc.get();
                // push pointer to scratch memory
                m_ClMemObjScratchMemList.push_back(spMemDesc.release());
            }
            else if(Buffer::GetBufferName() == currBuffer->GetName())
            {   // buffer
                BufferDesc buffDsc = GetBufferDescription(currBuffer->GetMemoryObjectDesc());
                IMemoryObject* outputBuffer = outputContainer->CreateBuffer(buffDsc);
                switch(arg_it->getType()->getTypeID())
                {
                case Type::FloatTyID:
                    {
                        if (buffDsc.GetElementDescription().GetType() != TFLOAT)
                        {
                            throw TestReferenceRunnerException("Input data type doesn't match kernel argument type. \
                                                               Input data type: " +
                                                               buffDsc.GetElementDescription().TypeToString()+". \
                                                                                                              Expected data type: f32");
                        }
                        if (buffDsc.NumOfElements() != 1) // one float value.
                        {
                            throw TestReferenceRunnerException("Input data type doesn't match kernel signature type.. \
                                                               Expected buffer length: 1");
                        }
                        ((float*)outputBuffer->GetDataPtr())[0] = ((float*)currBuffer->GetDataPtr())[0];
                        currArg.FloatVal = ((float*)currBuffer->GetDataPtr())[0];
                        break;
                    }
                case Type::DoubleTyID:
                    {
                        if (buffDsc.GetElementDescription().GetType() != TDOUBLE)
                        {
                            throw TestReferenceRunnerException("Input data type doesn't match kernel argument type. \
                                                               Input data type: " +
                                                               buffDsc.GetElementDescription().TypeToString()+". \
                                                                                                              Expected data type: f64");
                        }
                        if (buffDsc.NumOfElements() != 1) // one double value.
                        {
                            throw TestReferenceRunnerException("Unexpected buffer length. \
                                                               Expected buffer length: 1");
                        }
                        ((double*)outputBuffer->GetDataPtr())[0] = ((double*)currBuffer->GetDataPtr())[0];
                        currArg.DoubleVal = ((double*)currBuffer->GetDataPtr())[0];
                        break;
                    }
                case Type::PointerTyID:
                    {
                        currArg.PointerVal = GetPointerToTheArgValues(currBuffer,
                            outputBuffer,
                            arg_it->getType());
                    }
                    break;
                case Type::IntegerTyID:
                    {
                        uint64_t  uVal;
                        int64_t   iVal;
                        bool sign = false;
#define COPY_INT(TYPE)  \
    iVal = (int64_t)(((TYPE*)currBuffer->GetDataPtr())[0]); \
    ((TYPE*)outputBuffer->GetDataPtr())[0] = ((TYPE*)currBuffer->GetDataPtr())[0];  \
    sign = true

#define COPY_UINT(TYPE) \
    uVal = (uint64_t)(((TYPE*)currBuffer->GetDataPtr())[0]);    \
    ((TYPE*)outputBuffer->GetDataPtr())[0] = ((TYPE*)currBuffer->GetDataPtr())[0];  \
    sign = false

                        switch(buffDsc.GetElementDescription().GetType())
                        {
                        case TCHAR:
                            {
                                COPY_INT(int8_t);
                            }
                            break;
                        case TUCHAR:
                            {
                                COPY_UINT(uint8_t);
                            }
                            break;
                        case TSHORT:
                            {
                                COPY_INT(int16_t);
                            }
                            break;
                        case TUSHORT:
                            {
                                COPY_UINT(uint16_t);
                            }
                            break;
                        case TINT:
                            {
                                COPY_INT(int32_t);
                            }
                            break;
                        case TUINT:
                            {
                                COPY_UINT(uint32_t);
                            }
                            break;
                        case TLONG:
                            {
                                COPY_INT(int64_t);
                            }
                            break;
                        case TULONG:
                            {
                                COPY_UINT(uint64_t);
                            }
                            break;
                        default:
                            throw Exception::OutOfRange("Unsupported integer type! " + buffDsc.GetElementDescription().TypeToString());
                        }
                        currArg.IntVal = sign ? APInt(arg_it->getType()->getPrimitiveSizeInBits(), *(uint64_t*)&iVal, true) :
                            APInt(arg_it->getType()->getPrimitiveSizeInBits(), uVal);
                        break;
#undef COPY_INT
#undef COPY_UINT
                    }
                case Type::VectorTyID:
                    {
                        std::size_t numOfElements =
                            dyn_cast<VectorType>(arg_it->getType())->getNumElements();
                        switch ( dyn_cast<VectorType> (
                            arg_it->getType())->getElementType()->getTypeID() )
                        {
                        case Type::FloatTyID:
                            {
                                for (std::size_t j = 0; j < numOfElements; ++j)
                                {
                                    GenericValue vecVal;
                                    BufferAccessor<float> flBufferAcc(*currBuffer),
                                        outBufferAcc(*outputBuffer);
                                    outBufferAcc.GetElem(0,j) = flBufferAcc.GetElem(0,j);
                                    vecVal.FloatVal = flBufferAcc.GetElem(0,j);
                                    currArg.AggregateVal.push_back(vecVal);
                                }
                            }
                            break;
                        case Type::DoubleTyID:
                            {
                                for (std::size_t j = 0; j < numOfElements; ++j)
                                {
                                    GenericValue vecVal;
                                    BufferAccessor<double> flBufferAcc(*currBuffer),
                                        outBufferAcc(*outputBuffer);
                                    outBufferAcc.GetElem(0,j) = flBufferAcc.GetElem(0,j);
                                    vecVal.DoubleVal = flBufferAcc.GetElem(0,j);
                                    currArg.AggregateVal.push_back(vecVal);
                                }
                            }
                            break;
                        case Type::IntegerTyID:
                            {
                                for (std::size_t j = 0; j < numOfElements; ++j)
                                {
                                    GenericValue vecVal;
                                    ReadIntegerFromBuffer(currBuffer, outputBuffer, 0, j,
                                        dyn_cast<VectorType>(arg_it->getType())->
                                        getElementType()->
                                        getPrimitiveSizeInBits(),
                                        vecVal);
                                    currArg.AggregateVal.push_back(vecVal);
                                }
                            }
                            break;
                        default:
                            throw TestReferenceRunnerException("Unhandled vector type");
                        }
                    }
                    break;
                default:
                    throw TestReferenceRunnerException("Unhandled parameter type");
                }
            }
            else throw TestReferenceRunnerException("Not a buffer or image");
            ArgVals.push_back(currArg);
        }
        // break the loop on kernels in LLVM file when needed kernel is found
        break;

    } // for (uint32_t k = 0, e = metadata->getNumOperands(); k != e; ++k)

    if (!m_pKernel)
    {
        throw TestReferenceRunnerException(kernelName+" kernel not found!");
    }

    if(m_bUseNEAT)
    {
        // clear map
        m_NEATArgValues.clear();
        // create NEAT "mirror" of OCL program inputs. it is for zero BufferContainer
        IBufferContainer *pBCNeat = CreateNEATBufferContainer(*outputContainer, neatArgs);
        // create NEATGenericValues arguments to program
        CreateNEATBufferContainerMap(m_pKernel, *pBCNeat, m_NEATArgValues);
    }

}


void* OpenCLReferenceRunner::GetPointerToTheArgValues( const IMemoryObject* buffer,
                                                      IMemoryObject* outBuffer,
                                                      const llvm::Type*
                                                      argType )
{
    BufferDesc buffDsc = GetBufferDescription(buffer->GetMemoryObjectDesc());
    const PointerType *ptr = cast<PointerType>(argType);
    switch(ptr->getElementType()->getTypeID())
    {
    case Type::FloatTyID:
    case Type::DoubleTyID:
    case Type::IntegerTyID:
    case Type::VectorTyID:
    case Type::StructTyID:
        {
            std::copy((char*)(buffer->GetDataPtr()), (char*)(buffer->GetDataPtr()) + buffDsc.GetBufferSizeInBytes(), (char*)(outBuffer->GetDataPtr()));
            m_pointerArgs.push_back(outBuffer->GetDataPtr());
            break;
        }
    case Type::PointerTyID:
        {
      throw TestReferenceRunnerException("According to OCL specifications 1.1"
        "rev 36. Arguments to __kernel functions in a program cannot be"
        "declared as a pointer to a pointer(s).");
        }
        break;
    default:
        throw TestReferenceRunnerException("Unhelded parameter type");
    }
    return m_pointerArgs[m_pointerArgs.size() - 1];
}


void
OpenCLReferenceRunner::ReadIntegerFromBuffer(
    const IMemoryObject* inBuffer,
    IMemoryObject* outBuffer,
    const std::size_t vectorId,
    const std::size_t elementInVectorId,
    const unsigned int numOfBits,
    GenericValue &val )
{
    BufferDesc buffDsc = GetBufferDescription(inBuffer->GetMemoryObjectDesc());
    uint64_t  uVal = 0;
    int64_t   iVal;
    bool sign = false;
    TypeDesc elemDesc = buffDsc.GetElementDescription();
    if (elemDesc.GetType() == TVECTOR || elemDesc.GetType() == TARRAY)
        elemDesc = elemDesc.GetSubTypeDesc(0);
    switch(elemDesc.GetType())
    {
    case TCHAR:
        {
            BufferAccessor<int8_t> intBufferAcc(*inBuffer), outBufferAcc(*outBuffer);
            outBufferAcc.GetElem(vectorId, elementInVectorId) =
                intBufferAcc.GetElem(vectorId, elementInVectorId);
            iVal = intBufferAcc.GetElem(vectorId, elementInVectorId);
            sign = true;
        }
        break;
    case TUCHAR:
        {
            BufferAccessor<uint8_t> intBufferAcc(*inBuffer), outBufferAcc(*outBuffer);
            outBufferAcc.GetElem(vectorId, elementInVectorId) =
                intBufferAcc.GetElem(vectorId, elementInVectorId);
            uVal = intBufferAcc.GetElem(vectorId, elementInVectorId);
        }
        break;
    case TSHORT:
        {
            BufferAccessor<int16_t> intBufferAcc(*inBuffer), outBufferAcc(*outBuffer);
            outBufferAcc.GetElem(vectorId, elementInVectorId) =
                intBufferAcc.GetElem(vectorId, elementInVectorId);
            iVal = intBufferAcc.GetElem(vectorId, elementInVectorId);
            sign = true;
        }
        break;
    case TUSHORT:
        {
            BufferAccessor<uint16_t> intBufferAcc(*inBuffer),
                outBufferAcc(*outBuffer);
            outBufferAcc.GetElem(vectorId, elementInVectorId) =
                intBufferAcc.GetElem(vectorId, elementInVectorId);
            uVal = intBufferAcc.GetElem(vectorId, elementInVectorId);
        }
        break;
    case TINT:
        {
            BufferAccessor<int32_t> intBufferAcc(*inBuffer), outBufferAcc(*outBuffer);
            outBufferAcc.GetElem(vectorId, elementInVectorId) =
                intBufferAcc.GetElem(vectorId, elementInVectorId);
            iVal = intBufferAcc.GetElem(vectorId, elementInVectorId);
            sign = true;
        }
        break;
    case TUINT:
        {
            BufferAccessor<uint32_t> intBufferAcc(*inBuffer),
                outBufferAcc(*outBuffer);
            outBufferAcc.GetElem(vectorId, elementInVectorId) =
                intBufferAcc.GetElem(vectorId, elementInVectorId);
            uVal = intBufferAcc.GetElem(vectorId, elementInVectorId);
        }
        break;
    case TLONG:
        {
            BufferAccessor<int64_t> intBufferAcc(*inBuffer), outBufferAcc(*outBuffer);
            outBufferAcc.GetElem(vectorId, elementInVectorId) =
                intBufferAcc.GetElem(vectorId, elementInVectorId);
            iVal = intBufferAcc.GetElem(vectorId, elementInVectorId);
            sign = true;
        }
        break;
    case TULONG:
        {
            BufferAccessor<uint64_t> intBufferAcc(*inBuffer),
                outBufferAcc(*outBuffer);
            outBufferAcc.GetElem(vectorId, elementInVectorId) =
                intBufferAcc.GetElem(vectorId, elementInVectorId);
            uVal = intBufferAcc.GetElem(vectorId, elementInVectorId);
        }
        break;
    default:
        break;
    }
    val.IntVal = sign ? APInt(numOfBits, *(uint64_t*)&iVal, true) :
        APInt(numOfBits, uVal);
}

void OpenCLReferenceRunner::ParseToModule( const IProgram* program )
{
    const OpenCLProgram *oclProgram = static_cast<const OpenCLProgram *>(program);

    cl_prog_container_header *oclProgramContainerHeader =
        oclProgram->GetProgramContainer();
    unsigned int oclProgramContainerSize = oclProgram->GetProgramContainerSize();

    // Input parameters validation
    if(0 == oclProgramContainerSize || 0 == oclProgramContainerHeader)
    {
        throw TestReferenceRunnerException("Program container is invalid.");
    }

    //////////////////////////////////////////////////////////////////////
    // Create llvm module from program.

    // TODO: check all pointer initializations.
    m_pLLVMContext = new LLVMContext;
    if (0 == m_pLLVMContext)
    {
        throw TestReferenceRunnerException("Unable to create LLVM context.");
    }

    const char* pIR;    // Pointer to LLVM representation
    pIR = (const char*)oclProgramContainerHeader +
        sizeof(cl_prog_container_header) + sizeof(cl_llvm_prog_header);
    size_t stIRsize = oclProgramContainerHeader->container_size -
        sizeof(cl_llvm_prog_header);
    // Create Memory buffer to store IR data
    StringRef bitCodeStr(pIR, stIRsize);
    MemoryBuffer* pMemBuffer = MemoryBuffer::getMemBufferCopy(bitCodeStr);
    if (0 == pMemBuffer)
    {
        throw TestReferenceRunnerException("Can't create LLVM memory buffer from\
                                           program bytecode.");
    }

    std::string strLastError;
    m_pModule = ParseBitcodeFile(pMemBuffer, *m_pLLVMContext, &strLastError);
    if (0 == pMemBuffer)
    {
        throw TestReferenceRunnerException("Unable to parse bytecode into\
                                           LLVM module");
    }

  //createBuiltInImportPass(m_pRTModule, m_pExecEngine)->runOnModule(*m_pModule);
  DEBUG(dbgs() << "Before optimization: " << *m_pModule << "\n");

  SmallVector<Function*, 16> vectFunctions;
    std::vector<std::string> UndefinedExternalFunctions;
    ModulePass *wiUpdate = createOCLReferenceKernelUpdatePass(0, vectFunctions,
        m_pLLVMContext, UndefinedExternalFunctions);
    wiUpdate->runOnModule(*m_pModule);
  DEBUG(dbgs() << "running : " << *m_pModule << "\n");
}


static std::vector<GlobalVariable *> getLocalVariables(llvm::Module* pModule)
{
    std::vector<GlobalVariable *> locList;

    // Extract pointer to module annotations
    NamedMDNode *pMetadata = pModule->getNamedMetadata("opencl.kernels");
    if ( NULL == pMetadata )
    {
        throw Exception::InvalidArgument("getLocalVariables::"
            " Invalid input OpenCL module does not contain metadata");
    }

    // now we need to pass every kernel and make relevant changes to match our environment
    for (uint32_t i = 0, e = pMetadata->getNumOperands(); i != e; ++i)
    {
        // Obtain kernel function from annotation
        MDNode *elt = pMetadata->getOperand(i);
        Function *pFunc = dyn_cast<Function>(elt->getOperand(0)->stripPointerCasts());
        if ( NULL == pFunc )
        {
            continue; // Not a function pointer
        }

        // Obtain local variables array
        // magic const describes offset of local memory specifier in metadata
        const uint32_t METADATA_LOCAL_MEMORY_IDX = 5;
        const string LocalStr = dyn_cast<llvm::MDString>(
            elt->getOperand(METADATA_LOCAL_MEMORY_IDX))->getString();
        NamedMDNode *pFuncLocals = pModule->getNamedMetadata( LocalStr );

        if(pFuncLocals == NULL)
        {
            // no local variables in the function
            continue;
        }

        // Iterate through local vars in function
        for (uint32_t i = 0, e = pFuncLocals->getNumOperands(); i != e; ++i)
        {
            // Obtain local var name
            MDNode *elt = pFuncLocals->getOperand(i);
            std::string lclStr = dyn_cast<MDString>(
                elt->getOperand(0))->getString();

            GlobalVariable *pLclVar = pModule->getGlobalVariable(lclStr, true);
            if( NULL == pLclVar )
            {
                throw Exception::InvalidArgument("getLocalVariables::"
                    " Cannot retrieve local variable "+lclStr);
            }

            DEBUG(dbgs() << "local variable:\n " << *pLclVar << "\n");
            locList.push_back(pLclVar);
        }
    }
    return locList;
}


void OpenCLReferenceRunner::RunKernel( IRunResult * runResult,
                                       OpenCLKernelConfiguration * pKernelConfig,
                                       const ReferenceRunOptions* runConfig )
{
    assert(pKernelConfig != NULL && "There is no kernel to run!");

    BufferContainerList input;
    ReadInputBuffer(pKernelConfig, &input);

    // memory for storing kernel data marked with local addr space
    // Warning!: method delete [] is called on scratchMem
    // be careful that pointers in scratchMem are allocated by operator new[]
    std::vector<int8_t*> localsMemVec;

    std::vector<GenericValue> ArgVals;
    std::string kernelName = pKernelConfig->GetKernelName();

    // get work dimensions
    cl_uint workDim = pKernelConfig->GetWorkDimension();

    ReadKernelArgs(ArgVals,
        &input,
        kernelName,
        &runResult->GetOutput(kernelName.c_str()),
        &runResult->GetNEATOutput(kernelName.c_str()));

    // get global workgroup sizes
    size_t globalWGSizes[MAX_WORK_DIM];
    std::copy(pKernelConfig->GetGlobalWorkSize(),
        pKernelConfig->GetGlobalWorkSize() + MAX_WORK_DIM,
        globalWGSizes);

    // get local work group sizes
    size_t localWGSizes[MAX_WORK_DIM];
    std::copy(pKernelConfig->GetLocalWorkSize(),
        pKernelConfig->GetLocalWorkSize() + MAX_WORK_DIM,
        localWGSizes);

    // setup global sizes
    size_t global_size_x=globalWGSizes[0];
    size_t global_size_y=workDim<2?1:globalWGSizes[1];
    size_t global_size_z=workDim<3?1:globalWGSizes[2];

    // check usage of default local work size ( == 0 )
    for(cl_uint i=0; i < workDim; ++i)
    {
        if(localWGSizes[i] == 0)
        {   // set local Work size
            localWGSizes[i] = std::min<size_t>(
                globalWGSizes[i],
                size_t(runConfig->GetValue<uint32_t>(RC_COMMON_DEFAULT_LOCAL_WG_SIZE, 0)));
        }
    }

    // setup local sizes
    const size_t local_size_x=              localWGSizes[0];
    const size_t local_size_y=workDim<2?1:  localWGSizes[1];
    const size_t local_size_z=workDim<3?1:  localWGSizes[2];

    size_t num_groups[MAX_WORK_DIM] = {(global_size_x + local_size_x - 1) / local_size_x,
                                       (global_size_y + local_size_y - 1) / local_size_y,
                                       (global_size_z + local_size_z - 1) / local_size_z};

    // hack to speed up reference
    // reference runs only single work group
    if(runConfig->GetValue<bool>(RC_COMMON_RUN_SINGLE_WG, false))
    {
        global_size_x = local_size_x;
        global_size_y = local_size_y;
        global_size_z = local_size_z;
    }

    // index at which workload parameters are pushed into function
    // argument list
    uint32_t KernelArgBaseIdx = ArgVals.size();
    ///////////////////////////////////////////////////////////////////////////
    // Add pointer to a WorkDim.
    //struct sWorkInfoVal
    //{
    //    unsigned int	uiWorkDim;
    //    size_t            GlobalOffset[MAX_WORK_DIM];
    //    size_t            GlobalSize[MAX_WORK_DIM];
    //    size_t            LocalSize[MAX_WORK_DIM];
    //    size_t            WGNumber[MAX_WORK_DIM];
    //};
    Validation::sWorkInfoVal workDimMemory;
    Validation::sWorkInfoVal* pWorkDimMemory = &workDimMemory;

    pWorkDimMemory->uiWorkDim = workDim;

    ::memcpy(pWorkDimMemory->GlobalOffset,
        pKernelConfig->GetGlobalWorkOffset(),
        MAX_WORK_DIM * sizeof(size_t));

    ::memcpy(pWorkDimMemory->GlobalSize,
        globalWGSizes,
        MAX_WORK_DIM * sizeof(size_t));

    ::memcpy(pWorkDimMemory->LocalSize,
        localWGSizes,
        MAX_WORK_DIM * sizeof(size_t));

    ::memcpy(pWorkDimMemory->WGNumber, num_groups, MAX_WORK_DIM * sizeof(size_t));

    ArgVals.push_back(GenericValue(pWorkDimMemory));

//////////////////////////////////////////////////////////////////////////////////
    // Add pointer to a WorkGroupID.
    size_t workGroupIDMemory[MAX_WORK_DIM];
    size_t * pWorkGroupIDMemory = &workGroupIDMemory[0];
    ::memset(pWorkGroupIDMemory, 0, MAX_WORK_DIM*sizeof(size_t));
    ArgVals.push_back(GenericValue(pWorkGroupIDMemory));
///////////////////////////////////////////////////////////////////////////////
    // Add pointer to a base global ID.
    size_t baseGlobalID[MAX_WORK_DIM];
    size_t * pBaseGlobalID = &baseGlobalID[0];
    ::memset(pBaseGlobalID, 0, MAX_WORK_DIM*sizeof(size_t));
    ArgVals.push_back(GenericValue(pBaseGlobalID));
///////////////////////////////////////////////////////////////////////////////
    // Add pointer to a local ID.
    size_t * pLocalIDMemory = NULL;
    ArgVals.push_back(GenericValue(pLocalIDMemory));
    // remember index of local id storage in function argument vector
    uint32_t LocalIdIdx = KernelArgBaseIdx + 3;
///////////////////////////////////////////////////////////////////////////////

    // local engines
    std::vector<ExecutionEngine *> localEngines;
    std::vector<NEATPlugIn *> localNEATs;

    typedef std::vector< std::vector<GenericValue> > KernelArgsVector;
    KernelArgsVector localKernelArgVector;

    // use interpreterPluggable instead of interpreter
    // this should be called prior to creating EngineBuilder
    LLVMLinkInInterpreterPluggable();

    EngineBuilder builder(m_pModule);
    builder.setEngineKind(EngineKind::Interpreter);

#define FOR3_LOCALWG \
    for(size_t idx=0,lid_z=0;lid_z<local_size_z;lid_z++)\
    for(size_t lid_y=0;lid_y<local_size_y;lid_y++)\
    for(size_t lid_x=0;lid_x<local_size_x;lid_x++,idx++)

    FOR3_LOCALWG
    {
        // Create interpreter instance.
        ExecutionEngine * pExecEngine = builder.create();
        if (0 == pExecEngine)
        {
            throw TestReferenceRunnerException("Unable to create LLVM interpreter.");
        }

        NEATPlugIn * pNEAT = NULL;
        // add plug in to interpreter
        if(m_bUseNEAT)
        {
            // create NEAT plug in
            pNEAT = new NEATPlugIn();
            InterpreterPluggable *pInterp = static_cast<InterpreterPluggable*>(pExecEngine);
            pInterp->addPlugIn(*pNEAT);
            // set arguments for NEAT
            pNEAT->SetArgValues(m_NEATArgValues);
        }

        // Run static constructors.
        pExecEngine->runStaticConstructorsDestructors(false);
        localEngines.push_back(pExecEngine);
        if(m_bUseNEAT){
            localNEATs.push_back(pNEAT);
        }

        // local ID
        std::vector<GenericValue> localM = ArgVals;
        size_t localIDMemory[MAX_WORK_DIM];
        size_t * pLocalIDMemory = &localIDMemory[0];
        memset(pLocalIDMemory, 0, MAX_WORK_DIM*sizeof(size_t));
        localM[LocalIdIdx] = GenericValue(pLocalIDMemory);
        localKernelArgVector.push_back(localM);
    }

    for(size_t tid_z=0;tid_z<global_size_z;tid_z+=local_size_z)
    {
        for(size_t tid_y=0;tid_y<global_size_y;tid_y+=local_size_y)
        {
            for(size_t tid_x=0;tid_x<global_size_x;tid_x+=local_size_x)
            {
                pBaseGlobalID[0] = tid_x + pWorkDimMemory->GlobalOffset[0];
                pBaseGlobalID[1] = tid_y + pWorkDimMemory->GlobalOffset[1];
                pBaseGlobalID[2] = tid_z + pWorkDimMemory->GlobalOffset[2];
                pWorkGroupIDMemory[0] = tid_x / local_size_x;
                pWorkGroupIDMemory[1] = tid_y / local_size_y;
                pWorkGroupIDMemory[2] = tid_z / local_size_z;

                // Run static constructors.
                FOR3_LOCALWG
                {
                    localEngines[idx]->runStaticConstructorsDestructors(false);
                }

                // detect variables from __local space
                // and push it to list
                // list of __local variables
                std::vector<GlobalVariable *> localList =
                    getLocalVariables(m_pModule);

                // allocate or set mapping to memory of __local variables
                std::vector<void*> localMemList;
                FOR3_LOCALWG
                {
                    ExecutionEngine *EE = localEngines[idx];
                    for(uint32_t i=0;i<localList.size();++i)
                    {
                        if(idx == 0)
                        {   // zero WI allocate space
                            void *globalMem =
                                EE->getOrEmitGlobalVariable(localList[i]);
                            localMemList.push_back(globalMem);
                        }
                        else
                        {   // next WIs add mapping
                            EE->updateGlobalMapping(localList[i],
                                localMemList[i]);
                        }
                    }
                }

                std::vector<void*> localNEATMemList;
                // handle local variables for NEAT
                if(m_bUseNEAT)
                {
                    FOR3_LOCALWG
                    {
                        NEATPlugIn *pNeat = localNEATs[idx];

                        for(uint32_t i=0, cntNeat=0;i<localList.size();++i)
                        {
                            GlobalVariable * GV = localList[i];
                            const Type *GlobalType =
                                GV->getType()->getElementType();

                            // skip unsupported variables
                            if(NEATTargetData::IsNEATSupported(GlobalType)
                                                            == false)
                                continue;

                            DEBUG(dbgs() << "About to add NEAT supported "
                                "__local variable:\n " << *GV << "\n");

                            if(idx == 0)
                            {   // zero WI allocate space
                                void *globalMem =
                                    pNeat->getOrEmitGlobalVariable(GV);
                                localNEATMemList.push_back(globalMem);
                                localsMemVec.push_back((int8_t*) globalMem);
                            }
                            else
                            {   // next WIs add mapping
                                pNeat->updateGlobalMapping(localList[i], localNEATMemList[cntNeat]);
                            }
                            cntNeat++;
                        }
                    }
                } // if(m_bUseNEAT)


                // flag work group has not finished execution
                bool WGNotDone = true;
                // loop while Work group execution is not done
                while( WGNotDone )
                {
                    // flag barrier was detected during execution
                    bool FoundBarrier = false;
                    // loop over work items
                    FOR3_LOCALWG
                    {
                        // get pointer to local ID memory storage for current work item
                        GenericValue GVLocalId = localKernelArgVector[idx].at(LocalIdIdx);
                        size_t * pLocalIDMemory = (size_t*) GVTOP(GVLocalId);

                        DEBUG(dbgs() << "About to run local workitem idx :\n " << idx << "\n");
                        // setup local work item IDs
                        pLocalIDMemory[0] = lid_x;
                        pLocalIDMemory[1] = lid_y;
                        pLocalIDMemory[2] = lid_z;

                        // Run function.
                        GenericValue Result = localEngines[idx]->runFunction(
                            m_pKernel, localKernelArgVector[idx]);

                        // check return value
                        if( Result.IntVal.getBitWidth() == 8 &&
                            Result.IntVal == APInt(8, InterpreterPluggable::BARRIER))
                        {
                            // if barrier set flag barrier is found
                            FoundBarrier = true;
                        }
                    }
                    // if barrier found mark work group execution is not completed
                    WGNotDone = FoundBarrier ? true : false;
                }

                // Run static destructor s
                FOR3_LOCALWG
                {
                    localEngines[idx]->runStaticConstructorsDestructors(true);
                }
            }
        }
    }

#undef FOR3_LOCALWG

    for (uint32_t i=0; i<localEngines.size(); i++) {
        localEngines[i]->removeModule(m_pModule);
        delete localEngines[i];
    }

    for(std::size_t i=0; i<localsMemVec.size(); ++i)
        delete [] localsMemVec[i];
}

