/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2012).

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

#include "BinaryDataReader.h"
#include "BinaryDataWriter.h"
#include "Buffer.h"
#include "BufferContainerList.h"
#include "CPUDetect.h"
#include "ContainerCopier.h"
#include "DataVersion.h"
#include "Exception.h"
#include "InterpreterPlugIn.h"
#include "InterpreterPluggable.h"
#include "OCLKernelDataGenerator.h"
#include "OpenCLArgsBuffer.h"
#include "OpenCLCompilationFlags.h"
#include "OpenCLKernelArgumentsParser.h"
#include "OpenCLProgram.h"
#include "OpenCLReferenceRunner.h"
#include "OpenCLRunConfiguration.h"
#include "PlugInNEAT.h"
#include "RunResult.h"
#include "SATestException.h"
#include "SystemInfo.h"
#include "WorkGroupStorage.h"
#include "WorkItemStorage.h"
#include "XMLDataReader.h"
#include "XMLDataWriter.h"
#include "cl_device_api.h"
#include "cpu_dev_limits.h"
#include "mem_utils.h"

// Some header above icludes windows.h, which defines MemoryFence colliding
// with LLVM's MemoryFence function definition.
#undef MemoryFence

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Mutex.h"
#include "llvm/Transforms/Utils/Cloning.h"

#define DEBUG_TYPE "OpenCLReferenceRunner"
#include "llvm/Support/Debug.h"

#include <algorithm>
#include <list>
#include <string>
#include <vector>

//headers
#if defined(__LP64__)
#include <linux/limits.h>
#define MAX_PATH PATH_MAX
#elif defined(_WIN32)
#include <windows.h>
#endif

extern "C" void LLVMLinkInInterpreterPluggable();

using namespace Intel::OpenCL::DeviceBackend;
using namespace Intel::OpenCL::DeviceBackend::Utils;
using namespace Validation;
using namespace Validation::Exception;
using namespace llvm;

static const size_t MAX_LOCAL_MEMORY_SIZE = 30000;
// mutex for RunKernel to be thread-safe
static ManagedStatic<sys::Mutex> g_interpreterLock;

extern "C" void initOCLBuiltinsAsync();
extern "C" void initOCLBuiltinsAtomic();
extern "C" void initOCLBuiltinsCommon();
extern "C" void initOCLBuiltinsConvChar();
extern "C" void initOCLBuiltinsConvShort();
extern "C" void initOCLBuiltinsConvInt();
extern "C" void initOCLBuiltinsConvLong();
extern "C" void initOCLBuiltinsConvFPoint();
extern "C" void initOCLBuiltinsGeometric();
extern "C" void initOCLBuiltinsImages();
extern "C" void initOCLBuiltinsInteger();
extern "C" void initOCLBuiltinsMath();
extern "C" void initOCLBuiltinsMisc();
extern "C" void initOCLBuiltinsRelational();
extern "C" void initOCLBuiltinsWorkItem();
extern "C" void initOCLBuiltinsWorkGroup();
extern "C" void initOCLBuiltinsVLoadStore();
extern "C" void initOCLBuiltinsExplMemFenceOps();


OpenCLReferenceRunner::OpenCLReferenceRunner(bool bUseNEAT):
    m_pLLVMContext(NULL),
    m_pModule(NULL),
    m_pRTModule(NULL),
    m_pKernel(NULL),
    m_pExecEngine(NULL),
    m_pNEAT(NULL),
    m_bUseNEAT(bUseNEAT),
    m_bUseFmaNEAT(false)
{
    initOCLBuiltinsAsync();
    initOCLBuiltinsAtomic();
    initOCLBuiltinsCommon();
    initOCLBuiltinsConvChar();
    initOCLBuiltinsConvShort();
    initOCLBuiltinsConvInt();
    initOCLBuiltinsConvLong();
    initOCLBuiltinsConvFPoint();
    initOCLBuiltinsGeometric();
    initOCLBuiltinsImages();
    initOCLBuiltinsInteger();
    initOCLBuiltinsMath();
    initOCLBuiltinsMisc();
    initOCLBuiltinsRelational();
    initOCLBuiltinsWorkItem();
    initOCLBuiltinsWorkGroup();
    initOCLBuiltinsVLoadStore();
    initOCLBuiltinsExplMemFenceOps();
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
                                const IProgram* program,
                                const IProgramConfiguration* programConfig,
                                const IRunComponentConfiguration* runConfig)
{
    const OpenCLProgramConfiguration *pProgramConfig = static_cast<const OpenCLProgramConfiguration *>(programConfig);
    const ReferenceRunOptions *pRunConfig = static_cast<const ReferenceRunOptions *>(runConfig);

    m_pModule = static_cast<const OpenCLProgram*>(program)->ParseToModule();

    // if FP_CONTRACT is on, use fma in NEAT
    m_bUseFmaNEAT = m_pModule->getNamedMetadata("opencl.enable.FP_CONTRACT");

    for(OpenCLProgramConfiguration::KernelConfigList::const_iterator it = pProgramConfig->beginKernels();
        it != pProgramConfig->endKernels();
        ++it )
    {
        std::string kernelName = (*it)->GetKernelName();
        RunKernel( runResult, *it, pRunConfig );
    }
}

void OpenCLReferenceRunner::LoadOutput(IRunResult* pRunResult, const IProgramConfiguration* pConfig)
{
    const OpenCLProgramConfiguration *pProgramConfig = static_cast<const OpenCLProgramConfiguration *>(pConfig);

    for(OpenCLProgramConfiguration::KernelConfigList::const_iterator it = pProgramConfig->beginKernels();
        it != pProgramConfig->endKernels();
        ++it )
    {
        std::string kernelName = (*it)->GetKernelName();
        std::cout << " trying to load reference " << (*it)->GetStampedPathReference() << std::endl;
        ReadReferenceBuffer(*it, &pRunResult->GetOutput( kernelName.c_str()));

        if( m_bUseNEAT )
        {
            std::cout << " trying to load NEAT " << (*it)->GetStampedPathNeat() << std::endl;
            ReadNEATBuffer(*it, &pRunResult->GetNEATOutput(kernelName.c_str()));
        }
    }
}

void OpenCLReferenceRunner::StoreOutput(const IRunResult* pRunResult, const IProgramConfiguration* pConfig) const
{
    typedef std::map<std::string, uint32_t> BcIdxMap;

    const OpenCLProgramConfiguration *pProgramConfig = static_cast<const OpenCLProgramConfiguration *>(pConfig);

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
        const IBufferContainerList& pBcl = pRunResult->GetOutput( kernelName.c_str() );
        std::auto_ptr<IBufferContainerList> pNewBcl(cc.CloneBufferContainer(&pBcl, bcIdx));
        /// Save it to file
        std::cout << " generating reference " << (*it)->GetStampedPathReference() << std::endl;
        WriteReferenceBuffer( *it, pNewBcl.get());

        if( m_bUseNEAT )
        {
            const IBufferContainerList& neatBCL = pRunResult->GetNEATOutput( kernelName.c_str() );
            std::auto_ptr<IBufferContainerList> pNewBclNeat(cc.CloneBufferContainer(&neatBCL, bcIdx));
            std::cout << " generating NEAT " << (*it)->GetStampedPathNeat() << std::endl;
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
                                                 IContainer* pContainer) const
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
        throw TestReferenceRunnerException("Unsupported output file type");
    }
}



void OpenCLReferenceRunner::ReadInputBuffer(OpenCLKernelConfiguration* pKernelConfig, IContainer* pContainer, const uint64_t seed )
{
    assert( NULL != pKernelConfig);
    assert( NULL != pContainer);
    switch(pKernelConfig->GetInputFileType())
    {
        case Random:
        {
            OpenCLKernelArgumentsParser parser;
            if(pKernelConfig->GetGeneratorConfig() != 0 )
            {
                throw Exception::InvalidArgument("[OpenCLReferenceRunner]Unused OCLKernelDataGeneratorConfig found. \
                                                 Switch InputDataFileType to \'config\' or remove OCLKernelDataGeneratorConfig block");
            }
            OCLKernelArgumentsList args = parser.KernelArgumentsParser(pKernelConfig->GetKernelName(), m_pModule);
            args = OpenCLKernelArgumentsParser::KernelArgHeuristics(args, pKernelConfig->GetGlobalWorkSize(), pKernelConfig->GetWorkDimension());
            OCLKernelDataGeneratorConfig *cfg = OCLKernelDataGeneratorConfig::defaultConfig(args);
            cfg->setSeed(seed);
            OCLKernelDataGenerator gen(args, *cfg);

            gen.Read(pContainer);
            break;
        }
        case Config:
        {
            const OCLKernelDataGeneratorConfig *cfg = pKernelConfig->GetGeneratorConfig();
            //if there is no OCLKernelDataGeneratorConfig in configuration file or it is incorrect
            //GetGeneratorConfig will return zero value
            if(cfg == 0)
            {
                throw Exception::InvalidArgument("[OpenCLReferenceRunner]No config is provided. \
                                                 Add OCLKernelDataGeneratorConfig block or try to use another InputDataFileType");
            }
            OpenCLKernelArgumentsParser parser;
            OCLKernelArgumentsList args = parser.KernelArgumentsParser(pKernelConfig->GetKernelName(), m_pModule);
            args = OpenCLKernelArgumentsParser::KernelArgHeuristics(args, pKernelConfig->GetGlobalWorkSize(), pKernelConfig->GetWorkDimension());
            OCLKernelDataGenerator gen(args, *cfg);

            gen.Read(pContainer);
            break;
        }
        default:
        {
            ReadBufferContainer(pKernelConfig->GetInputFilePath(),
                pKernelConfig->GetInputFileType(),
                pContainer);
        }
    }
}

void OpenCLReferenceRunner::ReadReferenceBuffer(OpenCLKernelConfiguration* pKernelConfig, IContainer* pContainer )
{
    assert( NULL != pKernelConfig);
    assert( NULL != pContainer);

    std::string filename = pKernelConfig->GetStampedPathReference();
    if( filename.empty() )
    {
        throw Exception::ParserBadTypeException("ReadReferenceBuffer: reference file name is empty.");
    }

    ReadBufferContainer(pKernelConfig->GetStampedPathReference(),
                        pKernelConfig->GetReferenceFileType(),
                        pContainer);
}

void OpenCLReferenceRunner::ReadNEATBuffer(OpenCLKernelConfiguration* pKernelConfig, IContainer* pContainer )
{
    assert( NULL != pKernelConfig);
    assert( NULL != pContainer);

    std::string filename = pKernelConfig->GetStampedPathNeat();
    if( filename.empty() )
    {
        throw Exception::ParserBadTypeException("ReadNEATBuffer: NEAT file name is empty.");
    }

    ReadBufferContainer(pKernelConfig->GetStampedPathNeat(),
                        pKernelConfig->GetNeatFileType(),
                        pContainer);
}



void OpenCLReferenceRunner::WriteReferenceBuffer(OpenCLKernelConfiguration* pKernelConfig, IContainer* pContainer ) const
{
    assert( NULL != pKernelConfig);
    assert( NULL != pContainer);

    std::string filename = pKernelConfig->GetStampedPathReference();
    if( filename.empty() )
        return;

    WriteBufferContainer(pKernelConfig->GetStampedPathReference(),
                         pKernelConfig->GetReferenceFileType(),
                         pContainer);
}

void OpenCLReferenceRunner::WriteNEATBuffer(OpenCLKernelConfiguration* pKernelConfig, IContainer* pContainer ) const
{
    assert( NULL != pKernelConfig);
    assert( NULL != pContainer);

    std::string filename = pKernelConfig->GetStampedPathNeat();
    if( filename.empty() )
        return;

    WriteBufferContainer(pKernelConfig->GetStampedPathNeat(),
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

        Constant * globVal = mdconst::extract<Function>(elt->getOperand(0));
        m_pKernel = cast<Function>(globVal->stripPointerCasts());
        if ( NULL == m_pKernel )
        {
            continue;   // Not a function pointer
        }
        // if not needed name skip
        if ( m_pKernel->getName() != kernelName && !kernelName.empty())
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
                size_t imageSize = imageDesc.GetSizeInBytes();
                IMemoryObject* outputImage = outputContainer->CreateImage(imageDesc);

                // copy image data
                ::memcpy(outputImage->GetDataPtr(), currBuffer->GetDataPtr(), imageSize);

                // Kernel execution assumes all buffer arguments are aligned
                // If we do not align the buffer the execution crashes
                auto_ptr_ex<cl_mem_obj_descriptor> spMemDesc((cl_mem_obj_descriptor*)align_malloc(sizeof(cl_mem_obj_descriptor), CPU_DEV_MAXIMUM_ALIGN));
                FillMemObjDescriptor( *spMemDesc.get(), imageDesc, outputImage->GetDataPtr(), NULL );
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
    case Type::HalfTyID:
    case Type::FloatTyID:
    case Type::DoubleTyID:
    case Type::IntegerTyID:
    case Type::VectorTyID:
    case Type::StructTyID:
        {
            std::copy((char*)(buffer->GetDataPtr()), (char*)(buffer->GetDataPtr()) + buffDsc.GetSizeInBytes(), (char*)(outBuffer->GetDataPtr()));
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
        throw TestReferenceRunnerException("Unhandled parameter type");
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

// find OpenCL __local variables in module
static std::vector<const GlobalVariable *> getLocalVariables(llvm::Module* pModule)
{
   std::vector<const GlobalVariable *> locList;
   // pass on global variables in module
   for (Module::const_global_iterator I = pModule->global_begin(), E = pModule->global_end();
       I != E; ++I)
   {
       GlobalVariable const *GV = &*I;
       const uint32_t LOCAL_MEMORY_ADDR_SPACE = 3;
       // if global variable belongs to address space == LOCAL_MEMORY_ADDR_SPACE
       // then it is local
       if(I->getType()->getAddressSpace() == LOCAL_MEMORY_ADDR_SPACE)
       {
            DEBUG(dbgs() << "local variable:\n " << *GV << "\n");
            locList.push_back(GV);
       }
   }
    return locList;
}

// convert vector from size_t format to uint64_t
static void ConvertSizeTtoUint64T(const size_t *pI, std::vector<uint64_t>& O, uint32_t num)
{
    for(uint32_t i=0; i < num; ++i)
        O[i] = (uint64_t) pI[i];
}

static bool isWGSizeMustBeUniform(llvm::Module *module)
{
    CompilationFlagsList flagsList = GetCompilationFlags(module);

    const bool cl20 = find(flagsList.begin(), flagsList.end(), CL_STD_20)
        != flagsList.end();

    const bool uniformWGSize = find(flagsList.begin(), flagsList.end(), CL_UNIFORM_WORK_GROUP_SIZE)
        != flagsList.end();

    return !cl20 || uniformWGSize;
}

void OpenCLReferenceRunner::RunKernel( IRunResult * runResult,
                                       OpenCLKernelConfiguration * pKernelConfig,
                                       const ReferenceRunOptions* runConfig )
{
    assert(pKernelConfig != NULL && "There is no kernel to run!");

    sys::ScopedLock scopedLock(*g_interpreterLock);

    BufferContainerList input;
    ReadInputBuffer(pKernelConfig, &input,
        runConfig->GetValue<uint64_t>(RC_COMMON_RANDOM_DG_SEED, 0));

    DataVersion::ConvertData (&input, m_pModule->getNamedMetadata("opencl.kernels"), pKernelConfig->GetKernelName());

    // memory for storing kernel data marked with local addr space
    NEATPlugIn::GlobalAddressMapTy NEATlocalMap;

    std::vector<GenericValue> ArgVals;
    std::string kernelName = pKernelConfig->GetKernelName();

    // get work dimensions
    cl_uint workDim = pKernelConfig->GetWorkDimension();

    ReadKernelArgs(ArgVals,
        &input,
        kernelName,
        &runResult->GetOutput(kernelName.c_str()),
        &runResult->GetNEATOutput(kernelName.c_str()));

    // init global workgroup sizes with ones
    std::vector<uint64_t> globalWGSizes(MAX_WORK_DIM, 1);
    ConvertSizeTtoUint64T(pKernelConfig->GetGlobalWorkSize(), globalWGSizes, workDim);

    // init global workgroup sizes for usage in execution loop
    std::vector<uint64_t> loopGlobalWGSizes(globalWGSizes);

    // init local workgroup sizes with ones
    std::vector<uint64_t> localWGSizes(MAX_WORK_DIM, 1);
    // convert size_t to uint64_t
    ConvertSizeTtoUint64T(pKernelConfig->GetLocalWorkSize(), localWGSizes, workDim);

    // init work offset workgroup sizes with zeros
    std::vector<uint64_t> GlobalWorkOffset(MAX_WORK_DIM, 0);
    // convert size_t to uint64_t
    ConvertSizeTtoUint64T(pKernelConfig->GetGlobalWorkOffset(), GlobalWorkOffset, workDim);

    const bool wgSizeMustUniform = isWGSizeMustBeUniform(m_pModule);

    // check usage of default local work size ( == 0 )
    for(uint32_t i=0; i < workDim; ++i)
    {
        if(localWGSizes[i] == 0)
        {   // set local Work size
            localWGSizes[i] = std::min<uint64_t>(
                globalWGSizes[i],
                uint64_t(runConfig->GetValue<uint32_t>(RC_COMMON_DEFAULT_LOCAL_WG_SIZE, 0)));
            // the values specified in globalWorkSize[0], ..,
            // globalWorkSize[work_dim - 1] must be evenly divisible by
            // the corresponding values specified in
            // localWorkSize[0], .., localWorkSize[work_dim - 1]
            if (globalWGSizes[i] % localWGSizes[i] != 0 && wgSizeMustUniform)
            {
                // SATest could get the value of localSize from .cfg only, so if globalWorkSize
                // is not evenly divisible by localWorkSize, set localWGSizes to 1, then print
                // warning and continue working check
                // globalWGSizes[i] % localWGSizes[i] if it is not OCL 2.0
                llvm::errs() << "[OpenCLReferenceRunner::RunKernel warning] workDim # "
                    << i << " globalWorkSize = " << globalWGSizes[i]
                    << " is not evenly divisible by localWorkSize = "
                    << localWGSizes[i] << ", localWorkSize is set to 1 \n";

                localWGSizes[i] = 1;
            }
        }
        else if (globalWGSizes[i] % localWGSizes[i] != 0 && wgSizeMustUniform)
        {
            // throw exeption if it is not OCL 2.0, because
            // globalWGSizes[i] % localWGSizes[i] != 0 is valid for OCL2.0
            std::ostringstream s;
            s << "workDim # " << i << " globalWorkSize = " << globalWGSizes[i] <<
            " is not evenly divisible by localWorkSize = " << localWGSizes[i] << "\n";
            throw TestReferenceRunnerException(s.str());
        }
    }

    // hack to speed up reference
    // reference runs only single work group
    if(runConfig->GetValue<bool>(RC_COMMON_RUN_SINGLE_WG, false))
    {
        loopGlobalWGSizes[0] = localWGSizes[0];
        loopGlobalWGSizes[1] = localWGSizes[1];
        loopGlobalWGSizes[2] = localWGSizes[2];
    }
    // work item storage
    Validation::WorkItemStorage wiStorage(workDim, globalWGSizes, localWGSizes, GlobalWorkOffset);
    Validation::WorkGroupStorage wgStorage;
    // set wiStorage as interface for OCLBuiltins workitem functions
    OCLBuiltins::WorkItemInterfaceSetter::inst()->SetWorkItemInterface(&wiStorage);
    OCLBuiltins::WorkItemInterfaceSetter::inst()->SetWorkGroupInterface(&wgStorage);

    // local engines
    std::vector<ExecutionEngine *> localEngines;
    std::vector<NEATPlugIn *> localNEATs;

    //program compilation flags
    CompilationFlagsList cFlags = GetCompilationFlags(m_pModule);

    // total number of local workitems
    const uint32_t totalLocalWIs = localWGSizes[0] * localWGSizes[1] * localWGSizes[2];
    // vector of kernel arguments. used for multiple execution arguments
    typedef std::vector< std::vector<GenericValue> > KernelArgsVector;
    // storage of kernel arguments. each per local work item
    KernelArgsVector localKernelArgVector(totalLocalWIs);

    // use interpreterPluggable instead of interpreter
    // this should be called prior to creating EngineBuilder
    LLVMLinkInInterpreterPluggable();

    // initialize interpreters
    for(uint64_t idx = 0; idx < totalLocalWIs; ++idx)
    {
        // GCC 4.7.3 denies to compile temporary (i.e. rvalue) std::unique_ptr<Module) passed
        // to the EngineBuilder ctor. So, create an lvalue std::unique_ptr and pass it to the
        // std::move instead
        // [LLVM 3.6 UPGRADE} TODO: calling ExecutionEngine::create moves ownership of
        // llvm::Module from EngineBuilder to ExecutionEngine and we lose control over Module lifetime.
        // Need to figure out a way of creating ExecutionEngine instances without llvm::Module cloning.
        EngineBuilder builder(std::unique_ptr<Module>(CloneModule(m_pModule)));
        builder.setEngineKind(EngineKind::Interpreter);

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
            pNEAT = new NEATPlugIn(m_bUseFmaNEAT, NEATlocalMap, cFlags);
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

        // copy kernel arguments
        localKernelArgVector[idx] = ArgVals;
    }

    // global work item execution loop
    // global offset is not added (included) to loop variables
    for(uint64_t tid_z=0;tid_z<loopGlobalWGSizes[2];tid_z+=localWGSizes[2])
    {
        for(uint64_t tid_y=0;tid_y<loopGlobalWGSizes[1];tid_y+=localWGSizes[1])
        {
            for(uint64_t tid_x=0;tid_x<loopGlobalWGSizes[0];tid_x+=localWGSizes[0])
            {

#define FOR3_LOCALWG \
    for(uint64_t idx = 0, lid_z = 0; lid_z < wiStorage.GetLocalSize(tid_z, 2); ++lid_z)\
    for(uint64_t lid_y = 0; lid_y < wiStorage.GetLocalSize(tid_y, 1); ++lid_y)\
    for(uint64_t lid_x = 0; lid_x < wiStorage.GetLocalSize(tid_x, 0); ++lid_x, ++idx)

                // Run static constructors.
                FOR3_LOCALWG {
                    localEngines[idx]->runStaticConstructorsDestructors(false);
                }

                // detect variables from __local space
                // and push it to list
                // list of __local variables
                std::vector<const GlobalVariable *> localList =
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

                // handle local variables for NEAT
                if(m_bUseNEAT)
                {
                    NEATPlugIn *pNeat = localNEATs[0];

                    for(uint32_t i=0, cntNeat=0;i<localList.size();++i)
                    {
                        const GlobalVariable * GV = localList[i];
                        const Type *GlobalType =
                            GV->getType()->getElementType();

                        // skip unsupported variables
                        if(NEATDataLayout ::IsNEATSupported(GlobalType) == false)
                            continue;

                        DEBUG(dbgs() << "About to add NEAT supported "
                            "__local variable:\n " << *GV << "\n");
                        // zero WI allocate space
                        pNeat->getOrEmitGlobalVariable(GV);
                        cntNeat++;
                    }
                } // if(m_bUseNEAT)


                // flag work group has not finished execution
                bool WGNotDone = true;
                // loop while Work group execution is not done
                while( WGNotDone )
                {
                    // flag barrier was detected during execution
                    bool FoundBarrierOrWgFunction = false;
                    // loop over work items
                    FOR3_LOCALWG
                    {
                        DEBUG(dbgs() << "About to run local workitem idx :\n " << idx << "\n");

                        // setup local work item IDs
                        wiStorage.SetLocalID(0, lid_x);
                        wiStorage.SetLocalID(1, lid_y);
                        wiStorage.SetLocalID(2, lid_z);

                        // setup global work item IDs
                        wiStorage.SetGlobalID(0, tid_x + lid_x);
                        wiStorage.SetGlobalID(1, tid_y + lid_y);
                        wiStorage.SetGlobalID(2, tid_z + lid_z);

                        // Run function.
                        GenericValue Result = localEngines[idx]->runFunction(
                            m_pKernel, localKernelArgVector[idx]);

                        // check return value
                        if( Result.IntVal.getBitWidth() == 8 &&
                            ( Result.IntVal == APInt(8, InterpreterPluggable::BARRIER) ||
                            Result.IntVal == APInt(8, InterpreterPluggable::BLOCKING_WG_FUNCTION)))
                        {
                            // if barrier set flag barrier is found
                            FoundBarrierOrWgFunction = true;
                        }
                    }
                    // if barrier found mark work group execution is not completed
                    WGNotDone = FoundBarrierOrWgFunction ? true : false;
                }

                // Run static destructor s
                FOR3_LOCALWG
                {
                    localEngines[idx]->runStaticConstructorsDestructors(true);
                }

#undef FOR3_LOCALWG

            }
        }
    }

    for (uint32_t i=0; i<localEngines.size(); i++) {
        localEngines[i]->removeModule(m_pModule);
        delete localEngines[i];
    }

    for(NEATPlugIn::GlobalAddressMapTy::iterator I = NEATlocalMap.begin(), E=NEATlocalMap.end();
        I!=E; ++I)
    {
        delete [] (int8_t*)(*I).second;
    }
    
    (void)scopedLock;
}

