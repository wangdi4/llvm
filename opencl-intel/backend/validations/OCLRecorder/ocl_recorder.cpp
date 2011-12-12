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

File Name:  ocl_recorder.cpp

\*****************************************************************************/

#include "stdafx.h"
#include "ocl_recorder.h"
#include "cl_device_api.h"
#include <memory>
#include <sstream>
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/MutexGuard.h"
#include "llvm/Support/Casting.h"
#include "llvm/Argument.h"
#include "llvm/Function.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/LLVMContext.h"
#include "llvm/Target/TargetData.h"
#include "cl_device_api.h"
#include "IBufferContainerList.h"
#include "BinaryDataWriter.h"
#include "BufferContainerList.h"
#include "BufferDesc.h"

using namespace Intel::OpenCL::DeviceBackend;

namespace Validation
{
    const char* FILE_PREFIX = "ocl_recorder";
    const char* BINFILE_SUFFIX = "bin";
    const char* CFGFILE_SUFFIX = "cfg";
    const char* DATAFILE_SUFFIX = "dat";
    const char* REFERENCE_SUFFIX = "ref";
    const char* NEAT_SUFFIX = "neat";
    llvm::sys::cas_flag RecorderContext::s_counter = 0;

    namespace Utils
    {
    /// \brief Converts type description from LLVM data structures to Data Manager structures.
    TypeDesc GetTypeDesc(const llvm::Type* type, const llvm::TargetData* td)
    {
        assert(NULL != type);
        TypeDesc ret;
        switch(type->getTypeID())
        {
        case llvm::Type::FloatTyID:
            {
                ret.SetType(TFLOAT);
                break;
            }
        case llvm::Type::DoubleTyID:
            {
                ret.SetType(TDOUBLE);
                break;
            }
        case llvm::Type::IntegerTyID:
            {
                const llvm::IntegerType* ITy = llvm::cast<llvm::IntegerType>(type);
                unsigned intSize = ITy->getBitWidth();
                switch (intSize)
                {
                case 1: ret.SetType(TBOOL); break;
                case 8: ret.SetType(TUCHAR); break;
                case 16: ret.SetType(TUSHORT); break;
                case 32: ret.SetType(TUINT); break;
                case 64: ret.SetType(TULONG); break;
                default:
                    throw Exception::InvalidArgument("Unsupported integer size");
                }
                break;
            }
        case llvm::Type::VoidTyID:
            {
                ret.SetType(TVOID);
                break;
            }
        case llvm::Type::PointerTyID:
            {
                ret.SetType(TPOINTER);
                ret.SetNumberOfElements(1);
                TypeDesc subElemType = GetTypeDesc(llvm::cast<llvm::PointerType>(type)->getElementType(), td);
                ret.SetSubTypeDesc(0, subElemType);
                break;
            }
        case llvm::Type::ArrayTyID:
            {
                ret.SetType(TARRAY);
                ret.SetNumberOfElements(llvm::cast<llvm::ArrayType>(type)->getNumElements());
                TypeDesc subElemType = GetTypeDesc(llvm::cast<llvm::ArrayType>(type)->getElementType(), td);
                ret.SetSubTypeDesc(0, subElemType);
                break;
            }
        case llvm::Type::VectorTyID:
            {
                ret.SetType(TVECTOR);
                ret.SetNumberOfElements(llvm::cast<llvm::VectorType>(type)->getNumElements());
                TypeDesc subElemType = GetTypeDesc(llvm::cast<llvm::VectorType>(type)->getElementType(), td);
                ret.SetSubTypeDesc(0, subElemType);
                ret.SetTypeAllocSize(td->getTypeAllocSize(type));
                return ret;
            }
        case llvm::Type::StructTyID:
            {
                const llvm::StructType* STy = llvm::cast<llvm::StructType>(type);
                const llvm::StructLayout *SLO = td->getStructLayout(STy);
                ret.SetType(TSTRUCT);
                ret.SetNumOfSubTypes(STy->getNumElements());
                for (uint64_t i = 0; i < STy->getNumElements(); ++i)
                {
                    TypeDesc subElemType = GetTypeDesc(STy->getElementType(i), td);
                    subElemType.SetOffsetInStruct(SLO->getElementOffset(i));
                    ret.SetSubTypeDesc(i, subElemType);
                }
                if (SLO->getSizeInBytes() != td->getTypeAllocSize(type))
                {
                    throw Exception::ValidationExceptionBase("Target data struct size is not equal to StructLayout size!");
                }
                break;
            }
        default:
            throw Exception::InvalidArgument("Unsupported type.");
        }
        ret.SetTypeAllocSize(td->getTypeAllocSize(type));
        return ret;
    }

    size_t GetBufferSizeInBytes(cl_mem_obj_descriptor* pmem_obj)
    {
        assert(NULL != pmem_obj);
        size_t size = pmem_obj->dimensions.dim[0];

        for(unsigned int i = 1; i < pmem_obj->dim_count; ++i)
        {
            size += pmem_obj->dimensions.dim[i] * pmem_obj->pitch[i-1];
        }
        return size;
    }

    const BufferDesc GetBufferDesc(size_t size, const llvm::Argument& func_arg, const llvm::TargetData* td)
    {
        BufferDesc bd;
        // Do not record pointer.
        const llvm::Type* elType = func_arg.getType();
        if (elType->isPointerTy())
        {
            elType = (llvm::cast<llvm::PointerType>(elType))->getElementType();
        }
        TypeDesc elemType = GetTypeDesc(elType, td);
        /// When size of buffer is not multiple of element size
        /// We will round up number of elements
        bd.SetNumOfElements(ceil((double)size/elemType.GetSizeInBytes()));
        bd.SetElementDecs(elemType);
        return bd;
    }

    const BufferDesc GetBufferDesc(size_t elemSize, size_t numElements, const llvm::Argument& func_arg, const llvm::TargetData* td)
    {
        return GetBufferDesc( elemSize * numElements, func_arg, td);
    }

    const BufferDesc GetBufferDesc(cl_mem_obj_descriptor* pmem_obj, const llvm::Argument& func_arg, const llvm::TargetData* td)
    {
        assert(NULL != pmem_obj);
        return GetBufferDesc(GetBufferSizeInBytes(pmem_obj), func_arg, td);
    }

    DataTypeVal GetDataType(const llvm::Type* type)
    {
        assert(NULL != type);
        switch( type->getTypeID() )
        {
            case llvm::Type::FloatTyID:   return F32;
            case llvm::Type::DoubleTyID:  return F64;
            case llvm::Type::IntegerTyID: return I32;
            case llvm::Type::VectorTyID:  return GetDataType(llvm::cast<llvm::VectorType>(type)->getElementType());
            case llvm::Type::PointerTyID: return GetDataType(llvm::cast<llvm::PointerType>(type)->getElementType());
            default:
                assert(false && "Unsupported parameter type");
                throw Exception::InvalidArgument("Unsupported parameter type");
        }
    }



    ImageChannelDataTypeVal GetImageChannelDataTypeVal(cl_channel_type t)
    {
        ImageChannelDataTypeVal res = UNSPECIFIED_IMAGE_DATA_TYPE;
        switch(t)
        {
        case CLK_SNORM_INT8:         res = OpenCL_SNORM_INT8;        break;
        case CLK_SNORM_INT16:        res = OpenCL_SNORM_INT16;       break;
        case CLK_UNORM_INT8:         res = OpenCL_UNORM_INT8;        break;
        case CLK_UNORM_INT16:        res = OpenCL_UNORM_INT16;       break;
        case CLK_UNORM_SHORT_565:    res = OpenCL_UNORM_SHORT_565;   break;
        case CLK_UNORM_SHORT_555:    res = OpenCL_UNORM_SHORT_555;   break;
        case CLK_UNORM_INT_101010:   res = OpenCL_UNORM_INT_101010;  break;
        case CLK_SIGNED_INT8:        res = OpenCL_SIGNED_INT8;       break;
        case CLK_SIGNED_INT16:       res = OpenCL_SIGNED_INT16;      break;
        case CLK_SIGNED_INT32:       res = OpenCL_SIGNED_INT32;      break;
        case CLK_UNSIGNED_INT8:      res = OpenCL_UNSIGNED_INT8;     break;
        case CLK_UNSIGNED_INT16:     res = OpenCL_UNSIGNED_INT16;    break;
        case CLK_UNSIGNED_INT32:     res = OpenCL_UNSIGNED_INT32;    break;
        case CLK_HALF_FLOAT:         res = OpenCL_HALF_FLOAT;        break;
        case CLK_FLOAT:              res = OpenCL_FLOAT;             break;
        default:                    res = INVALID_IMAGE_DATA_TYPE;  break;
        }
        return res;
    }

    ImageChannelOrderVal GetImageChannelOrderVal(cl_channel_order t)
    {
        ImageChannelOrderVal res = UNSPECIFIED_CHANNEL_ORDER;
        switch(t)
        {
        case CLK_R:          res = OpenCL_R;                 break;
        //case :         res = OpenCL_Rx;                break;
        case CLK_A:          res = OpenCL_A;                 break;
        case CLK_INTENSITY:  res = OpenCL_INTENSITY;         break;
        case CLK_LUMINANCE:  res = OpenCL_LUMINANCE;         break;
        case CLK_RG:         res = OpenCL_RG;                break;
        //case :        res = OpenCL_RGx;               break;
        case CLK_RA:         res = OpenCL_RA;                break;
        case CLK_RGB:        res = OpenCL_RGB;               break;
        //case :       res = OpenCL_RGBx;              break;
        case CLK_RGBA:       res = OpenCL_RGBA;              break;
        case CLK_ARGB:       res = OpenCL_ARGB;              break;
        case CLK_BGRA:       res = OpenCL_BGRA;              break;
        default:            res = INVALID_CHANNEL_ORDER;    break;
        }
        return res;
    }


    const ImageDesc GetImageDesc(cl_mem_obj_descriptor* pmem_obj, const llvm::Argument& func_arg)
    {
        assert(NULL != pmem_obj);
        ImageSizeDesc imSizes;
        imSizes.width = pmem_obj->dimensions.dim[0];
        imSizes.height = pmem_obj->dimensions.dim[1];
        imSizes.depth = pmem_obj->dimensions.dim[2];
        imSizes.row = pmem_obj->pitch[0];
        imSizes.slice = pmem_obj->pitch[1];

        ImageTypeVal imageType = GetImageTypeFromDimCount(pmem_obj->dim_count);

        return ImageDesc (
            imageType,
            imSizes,
            GetImageChannelDataTypeVal(pmem_obj->format.image_channel_data_type),
            GetImageChannelOrderVal(pmem_obj->format.image_channel_order));
    }

    }


    const std::string BinaryContext::getBaseName( ) const
    {
        std::ostringstream filename;
        if( m_index > 0 )
            filename << m_name << "." << (int)m_index ;
        else
            filename << m_name ;

        return filename.str();
    }

    BinaryContext*
    KernelContext::getOrCreateBinaryContext(const char* name,
                                            const _cl_work_description_type* workDesc,
                                            bool& created)
    {
        BinaryContext context(name, workDesc, m_binaries.size());

        BinaryContextList::iterator it = std::find( m_binaries.begin(),
                                                 m_binaries.end(),
                                                 context );

        if( m_binaries.end() == it )
        {
            m_binaries.push_back( context );
            created = true;
            return &m_binaries.back();
        }

        created = false;
        return &*it;
    }

    RecorderContext::RecorderContext( const llvm::sys::Path& logsPath, const std::string& prefix):
        m_TD(NULL),
        m_logsPath(logsPath.c_str())
    {
        // create the unique path for current context
        std::ostringstream filename;
        int index = (int)llvm::sys::AtomicIncrement(&s_counter);

        if( index > 1 )
        {
            filename << prefix << "." << index;
            m_baseName = filename.str();
        }
        else
            m_baseName = prefix;

        // initialize the xml document for
        m_config.InsertEndChild( TiXmlDeclaration( "1.0", "", "" ) );
        m_pRunConfig = new TiXmlElement("RunConfiguration");
        m_config.LinkEndChild(m_pRunConfig);
    }

    RecorderContext::~RecorderContext()
    {
        delete m_TD;
    }

    const std::string RecorderContext::getPath( const std::string& suffix ) const
    {
        llvm::sys::Path path( m_logsPath );
        path.appendComponent( m_baseName);
        path.appendSuffix(suffix);
        return path.str();
    }

    const std::string RecorderContext::getPath( const std::string& kernelName,  const std::string& suffix ) const
    {
        llvm::sys::Path path( m_logsPath );
        path.appendComponent( m_baseName);
        path.appendSuffix(kernelName);
        path.appendSuffix(suffix);
        return path.str();
    }

    const std::string RecorderContext::getFileName( const std::string& suffix ) const
    {
        llvm::sys::Path path( m_baseName);
        path.appendSuffix(suffix);
        return path.str();
    }

    const std::string RecorderContext::getFileName( const std::string& kernelName, const std::string& suffix  ) const
    {
        llvm::sys::Path path( m_baseName);
        path.appendSuffix(kernelName);
        path.appendSuffix(suffix);
        return path.str();
    }


    const std::string RecorderContext::getByteCodeFilePath() const
    {
        return getPath(BINFILE_SUFFIX);
    }

    const std::string RecorderContext::getByteCodeFileName() const
    {
        return getFileName(BINFILE_SUFFIX);
    }

    const std::string RecorderContext::getConfigFilePath() const
    {
        return getPath(CFGFILE_SUFFIX);
    }

    const std::string RecorderContext::getInputFilePath( const std::string& kernelName ) const
    {
        return getPath(kernelName, DATAFILE_SUFFIX);
    }

    const std::string RecorderContext::getInputFileName( const std::string& kernelName ) const
    {
        return getFileName(kernelName, DATAFILE_SUFFIX);
    }

    const std::string RecorderContext::getReferenceFilePath( const std::string& kernelName ) const
    {
        return getPath(kernelName, REFERENCE_SUFFIX);
    }

    const std::string RecorderContext::getReferenceFileName(const std::string& kernelName ) const
    {
        return getFileName(kernelName, REFERENCE_SUFFIX);
    }

    const std::string RecorderContext::getNeatFilePath( const std::string& kernelName ) const
    {
        return getPath(kernelName, NEAT_SUFFIX);
    }

    const std::string RecorderContext::getNeatFileName( const std::string& kernelName ) const
    {
        return getFileName(kernelName, NEAT_SUFFIX);
    }

    void RecorderContext::Flush()
    {
        m_config.SaveFile( getConfigFilePath().c_str() );
    }

    bool RecorderContext::containsKernel( const ICLDevBackendKernel_* pKernel )
    {
        llvm::MutexGuard lock(m_kernelsLock);
        return m_kernels.end() != m_kernels.find( pKernel );
    }

    KernelContext* RecorderContext::getKernelContext(const ICLDevBackendKernel_* pKernel )
    {
        llvm::MutexGuard lock(m_kernelsLock);

        KernelContextMap::iterator itContext = m_kernels.find(pKernel);
        assert( m_kernels.end() != itContext);

        return &(itContext->second);
    }

    void RecorderContext::createKernelContext(const ICLDevBackendKernel_* pKernel,
                                              const llvm::Function* pFunction)
    {
        llvm::MutexGuard lock(m_kernelsLock);
        assert( m_kernels.end() == m_kernels.find( pKernel ));

        m_kernels.insert(std::make_pair(pKernel, KernelContext( pKernel->GetKernelName(), pFunction)));
    }

    OCLRecorder::OCLRecorder( const llvm::sys::Path& logsDir, const std::string& prefix ):
        m_logsDir(logsDir),
        m_prefix(prefix),
        m_pLLVMContext(new llvm::LLVMContext)
    {
    }

    OCLRecorder::~OCLRecorder()
    {
        delete m_pLLVMContext;
    }

    RecorderContext* OCLRecorder::GetProgramContext(const ICLDevBackendProgram_* pProgram)
    {
        llvm::MutexGuard lock(m_contextsLock);

        RecorderContextMap::iterator itContext = m_contexts.find(pProgram);
        assert( m_contexts.end() != itContext);

        RecorderContext* pContext = itContext->second;
        assert( NULL != pContext);

        return pContext;
    }

    RecorderContext* OCLRecorder::GetProgramContextForKernel(const ICLDevBackendKernel_* pKernel)
    {
        llvm::MutexGuard lock(m_contextsLock);

        for( RecorderContextMap::iterator i= m_contexts.begin(), e = m_contexts.end(); i != e; ++i )
        {
            RecorderContext* pContext = i->second;
            assert( NULL != pContext);

            if( pContext->containsKernel(pKernel) )
            {
                return pContext;
            }
        }
        assert(false && "Can't find the given kernel in context");
        throw Exception::ValidationExceptionBase("Can't find the given kernel in context");
    }



    void OCLRecorder::AddNewProgramContext(const ICLDevBackendProgram_* pProgram, RecorderContext* pContext)
    {
        llvm::MutexGuard lock(m_contextsLock);
        m_contexts[pProgram] = pContext;
    }

    void OCLRecorder::RemoveProgramContext(const ICLDevBackendProgram_* pProgram)
    {
        llvm::MutexGuard lock(m_contextsLock);

        RecorderContextMap::iterator itContext = m_contexts.find(pProgram);
        assert( m_contexts.end() != itContext);

        m_contexts.erase( itContext );
    }


    void OCLRecorder::OnCreateBinary(const ICLDevBackendKernel_* pKernel,
                                     const _cl_work_description_type* pWorkDesc,
                                     size_t bufSize,
                                     void* pArgsBuffer)
    {
        assert(pWorkDesc);
        assert(pKernel);
        bool created = false;
        RecorderContext* pProgramContext = GetProgramContextForKernel(pKernel);
        KernelContext*   pKernelContext  = pProgramContext->getKernelContext(pKernel);
        BinaryContext*   pBinaryContext  = pKernelContext->getOrCreateBinaryContext(pKernel->GetKernelName(),
                                                                                    pWorkDesc,
                                                                                    created);

        if( created )
        {
            RecordKernelConfig( *pProgramContext, *pKernelContext, *pBinaryContext);
            RecordKernelInputs( *pProgramContext, *pKernelContext, *pBinaryContext, pKernel, bufSize, pArgsBuffer);
        }
    }

    void OCLRecorder::OnCreateKernel(const ICLDevBackendProgram_* pProgram,
                                     const ICLDevBackendKernel_* pKernel,
                                     const llvm::Function* pFunction)
    {
        RecorderContext* pProgramContext = GetProgramContext(pProgram);
        if (!pProgramContext->containsKernel(pKernel))
        {
            pProgramContext->createKernelContext(pKernel, pFunction);
        }
    }

    void OCLRecorder::OnCreateProgram(const _cl_prog_container_header* pContainer,
                                      const ICLDevBackendProgram_* pProgram)
    {
        assert( m_contexts.end()== m_contexts.find(pProgram));
        std::auto_ptr<RecorderContext> spContext(new RecorderContext(m_logsDir, m_prefix));

        RecordByteCode(pContainer, *spContext);

        // Create target data object.
        char * fileData = (char*)pContainer + sizeof(_cl_prog_container_header)+ sizeof(cl_llvm_prog_header);
        size_t fileDataSize = pContainer->container_size - sizeof(cl_llvm_prog_header);
        llvm::StringRef bitCodeStr(fileData, fileDataSize);
        llvm::MemoryBuffer* pMemBuff = llvm::MemoryBuffer::getMemBufferCopy(bitCodeStr);
        if ( NULL == pMemBuff )
        {
            throw Exception::ValidationExceptionBase("Can't create memory buffer from IR.");
        }
        std::string strLastError;
        llvm::Module *pModule = ParseBitcodeFile(pMemBuff, *m_pLLVMContext, &strLastError);
        if ( NULL == pModule )
        {
            throw Exception::ValidationExceptionBase("Failed to parse IR");
        }
        spContext->m_TD = new llvm::TargetData(pModule);

        RecordProgramConfig(*spContext);

        AddNewProgramContext(pProgram, spContext.release());
    }

    void OCLRecorder::OnReleaseProgram(const ICLDevBackendProgram_* pProgram)
    {
        RecorderContext* pContext = GetProgramContext(pProgram);
        pContext->Flush();
        delete pContext;

        RemoveProgramContext(pProgram);
    }

    void OCLRecorder::RecordProgramConfig(RecorderContext& context)
    {
        AddChildTextNode(context.m_pRunConfig, "ByteCodeFile", context.getByteCodeFileName());
        context.Flush();
    }


    void OCLRecorder::AddChildTextNode( TiXmlElement* pParentNode, const char* childName, const std::string& value)
    {
        TiXmlText *pText = new TiXmlText( value );
        TiXmlElement *pNode = new TiXmlElement(childName);
        pNode->LinkEndChild(pText);
        pParentNode->LinkEndChild(pNode);
    }

    void OCLRecorder::RecordKernelConfig(RecorderContext& programContext,
                                         const KernelContext& kernelContext,
                                         const BinaryContext& binaryContext)
    {
        TiXmlElement *pNodeKernelConfig = new TiXmlElement("KernelConfiguration");
        pNodeKernelConfig->SetAttribute( "Name", kernelContext.getName().c_str());

        // Write workDimention
        {
            std::stringstream sWorkDimention;
            sWorkDimention << binaryContext.getWorkDimention();
            AddChildTextNode( pNodeKernelConfig, "WorkDimention", sWorkDimention.str());
        }
        // Write LocalWorkSize
        {
            std::stringstream value;
            for (unsigned i = 0; i < binaryContext.getWorkDimention(); ++i)
            {
                value << binaryContext.getLocalWorkSize()[i] << ' ';
            }
            AddChildTextNode( pNodeKernelConfig, "LocalWorkSize", value.str());
        }
        // Write GlobalWorkSize
        {
            std::stringstream value;
            for (unsigned i = 0; i < binaryContext.getWorkDimention(); ++i)
            {
                value << binaryContext.getGlobalWorkSize()[i] << ' ';
            }
            AddChildTextNode( pNodeKernelConfig, "GlobalWorkSize", value.str());
        }
        // Write GlobalWorkOffset
        {
            std::stringstream value;
            for (unsigned i = 0; i < binaryContext.getWorkDimention(); ++i)
            {
                value << binaryContext.getGlobalWorkOffset()[i] << ' ';
            }
            AddChildTextNode( pNodeKernelConfig, "GlobalWorkOffset", value.str());
        }
        // Write Input data filename
        {
            AddChildTextNode( pNodeKernelConfig, "InputDataFile", programContext.getInputFileName(binaryContext.getBaseName()));
        }
        // Write Input data type
        {
            AddChildTextNode( pNodeKernelConfig, "InputDataFileType", "binary");
        }
        // Write Reference data filename
        {
            AddChildTextNode( pNodeKernelConfig, "ReferenceDataFile", programContext.getReferenceFileName(binaryContext.getBaseName()));
        }
        // Write Reference data type
        {
            AddChildTextNode( pNodeKernelConfig, "ReferenceDataFileType", "binary");
        }
        // Write Neat data filename
        {
            AddChildTextNode( pNodeKernelConfig, "NeatDataFile", programContext.getNeatFileName(binaryContext.getBaseName()));
        }
        // Write Neat data type
        {
            AddChildTextNode( pNodeKernelConfig, "NeatDataFileType", "binary");
        }

        llvm::MutexGuard lock(programContext.m_configLock);
        programContext.m_pRunConfig->LinkEndChild(pNodeKernelConfig);
        programContext.Flush();
    }


    void OCLRecorder::RecordKernelInputs( const RecorderContext& programContext,
                                          const KernelContext& kernelContext,
                                          const BinaryContext& binaryContext,
                                          const ICLDevBackendKernel_* pKernel,
                                          size_t bufSize,
                                          void* pArgsBuffer)
    {
        assert(NULL != pKernel);
        // get kernel arguments
        unsigned int argsCount = pKernel->GetKernelParamsCount();
        const cl_kernel_argument *pKernelArgs = pKernel->GetKernelParams();

        // setup the buffer container list
        BufferContainerList bufferList;
        IBufferContainer *pBufferContainer = bufferList.CreateBufferContainer();

        // iterate over all the kernel parameters and populate the buffer container
        size_t stArgsOffset = 0;
        llvm::Function::const_arg_iterator arg_it = kernelContext.getFuncPtr()->arg_begin();
        for(unsigned int i=0; i<argsCount; ++i)
        {
            if ( CL_KRNL_ARG_PTR_IMG_2D <= pKernelArgs[i].type )
            {
                cl_mem_obj_descriptor* mem_descriptor = *(cl_mem_obj_descriptor**)((char*)pArgsBuffer+stArgsOffset);
                // create image
                ImageDesc desc = Utils::GetImageDesc(mem_descriptor, *arg_it);
                IMemoryObject* pImage = pBufferContainer->CreateImage(desc);

                // fill it with data
                char* pData = (char*)pImage->GetDataPtr();
                size_t size  = desc.GetImageSizeInBytes();
                memcpy( pData, mem_descriptor->pData, size);

                stArgsOffset += sizeof(void*);
            }
            else if ( CL_KRNL_ARG_PTR_GLOBAL <= pKernelArgs[i].type )
            {
                cl_mem_obj_descriptor* mem_descriptor = *(cl_mem_obj_descriptor**)((char*)pArgsBuffer+stArgsOffset);
                // create buffer
                BufferDesc desc = Utils::GetBufferDesc(mem_descriptor, *arg_it, programContext.m_TD);
                IMemoryObject* pBuffer = pBufferContainer->CreateBuffer(desc);

                // fill it with data
                char* pData = (char*)pBuffer->GetDataPtr();
                size_t size  = Utils::GetBufferSizeInBytes(mem_descriptor);
                memset( pData, 0, desc.GetBufferSizeInBytes());
                memcpy( pData, mem_descriptor->pData, size);

                stArgsOffset += sizeof(void*);
            }
            else if (CL_KRNL_ARG_PTR_LOCAL == pKernelArgs[i].type)
            {
                size_t localMemSize = *(size_t*)((char*)pArgsBuffer+stArgsOffset);
                BufferDesc desc = Utils::GetBufferDesc(localMemSize, *arg_it, programContext.m_TD);
                IMemoryObject* pBuffer = pBufferContainer->CreateBuffer(desc);

                // fill it with data
                char* pData = (char*)pBuffer->GetDataPtr();
                size_t size  = desc.GetBufferSizeInBytes();
                memset( pData, 0, size);

                stArgsOffset += sizeof(void*);
            }
            else if (CL_KRNL_ARG_VECTOR == pKernelArgs[i].type)
            {
                size_t elemSize = pKernelArgs[i].size_in_bytes >> 16;
                size_t numElements = (pKernelArgs[i].size_in_bytes) & 0xFFFF;
                size_t uiSize = elemSize * numElements;

                BufferDesc desc = Utils::GetBufferDesc(elemSize, numElements, *arg_it, programContext.m_TD);
                IMemoryObject* pBuffer = pBufferContainer->CreateBuffer(desc);
                memcpy( pBuffer->GetDataPtr(), (void *)((char*)pArgsBuffer + stArgsOffset), uiSize);
                stArgsOffset += uiSize;
            }
            else if (CL_KRNL_ARG_SAMPLER == pKernelArgs[i].type)
            {
                BufferDesc desc = Utils::GetBufferDesc(sizeof(unsigned int), *arg_it, programContext.m_TD);
                IMemoryObject* pBuffer = pBufferContainer->CreateBuffer(desc);
                memcpy( pBuffer->GetDataPtr(), (void *)((char*)pArgsBuffer + stArgsOffset), sizeof(unsigned int));
                stArgsOffset += sizeof(unsigned int);
            }
            else
            {
                BufferDesc desc = Utils::GetBufferDesc((size_t)pKernelArgs[i].size_in_bytes, *arg_it, programContext.m_TD);
                IMemoryObject* pBuffer = pBufferContainer->CreateBuffer(desc);
                memcpy( pBuffer->GetDataPtr(), (void *)((char*)pArgsBuffer + stArgsOffset), pKernelArgs[i].size_in_bytes);
                stArgsOffset += pKernelArgs[i].size_in_bytes;
            }
            ++arg_it;
        }

        BinaryContainerListWriter bufferWriter(programContext.getInputFilePath( binaryContext.getBaseName()));
        bufferWriter.Write(&bufferList);
    }

    void OCLRecorder::RecordByteCode(const _cl_prog_container_header* pContainer, const RecorderContext& context)
    {
        std::string error;
        llvm::raw_fd_ostream binStream( context.getByteCodeFilePath().c_str(), error, llvm::raw_fd_ostream::F_Binary);

        if( !error.empty() )
        {
            throw Exception::ValidationExceptionBase("Can't open the file for output");
        }

        char * fileData = (char*)pContainer + sizeof(_cl_prog_container_header)+ sizeof(cl_llvm_prog_header);
        size_t fileDataSize = pContainer->container_size - sizeof(cl_llvm_prog_header);
        binStream.write( fileData, fileDataSize );
    }


}

// Defines the exported functions for the DLL application.
#ifdef __cplusplus
extern "C"
{
#endif
    OCL_RECORDER_API ICLDevBackendPlugin* CreatePlugin(void)
    {
        char* sz_logdir = getenv("OCLRECORDER_LOGDIR");
        char* sz_dumpprefix = getenv("OCLRECORDER_DUMPPREFIX");

        llvm::sys::Path logpath = (NULL == sz_logdir) ? llvm::sys::Path::GetCurrentDirectory()
                                                      : llvm::sys::Path(sz_logdir);
        std::string prefix = (NULL == sz_dumpprefix) ? std::string(Validation::FILE_PREFIX)
                                                     : sz_dumpprefix;
        logpath.makeAbsolute();

        return new Validation::OCLRecorder(logpath, prefix);
    }

    OCL_RECORDER_API void ReleasePlugin(ICLDevBackendPlugin* pPlugin)
    {
        delete pPlugin;
    }
#ifdef __cplusplus
}
#endif
