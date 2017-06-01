/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  ImageCallbackLibrary.cpp

\*****************************************************************************/

#include "BitCodeContainer.h"
#include "CompiledModule.h"
#include "Compiler.h"
#include "exceptions.h"
#include "ImageCallbackLibrary.h"
#include "SystemInfo.h"
#include "ServiceFactory.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/Memory.h"

#include <string>

#if defined(_WIN32)
    #include <windows.h>
#else
    #include <linux/limits.h>
    #define MAX_PATH PATH_MAX
#endif

#define TRAP_NAME "trap_function"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

const string channelOrderToPrefix(cl_channel_order _co)
{
    std::string toReturn = channelOrderToString(_co);
    // Must match order in cl_api/cl.h and cl_api/cl_2_0.h
    // channel_order starts with CL_ that we need to cut
    toReturn = toReturn.substr(strlen("CL_"), toReturn.size());
    return toReturn;
}

const string samplerToAddrModePrefix(SamplerType _sampler)
{
  int addressMode = _sampler & __ADDRESS_MASK;
  switch (addressMode)
  {
    case CLK_ADDRESS_MIRRORED_REPEAT:
      return "MIRRORED_REPEAT";
      break;
    case CLK_ADDRESS_REPEAT:
      return "REPEAT";
    case CLK_ADDRESS_CLAMP:
      return "CLAMP_TO_EDGE";
    case CLK_ADDRESS_CLAMP_TO_EDGE:
      return "CLAMP_TO_EDGE";
    case CLK_ADDRESS_NONE:
      return "NONE";
    default:
      assert(0 && "Unkown addressing mode in sampler");
      return "Unknown";
  }
}

const string imgTypeToDimPrefix(cl_mem_object_type _type)
{
    switch(_type)
    {
        case CL_MEM_OBJECT_IMAGE1D:
        case CL_MEM_OBJECT_IMAGE1D_ARRAY:
        case CL_MEM_OBJECT_IMAGE1D_BUFFER:
            return "1D";
        case CL_MEM_OBJECT_IMAGE2D:
        case CL_MEM_OBJECT_IMAGE2D_ARRAY:
            return "2D";
        case CL_MEM_OBJECT_IMAGE3D:
            return "3D";
        default:
            throw Exceptions::DeviceBackendExceptionBase("Invalid type of image object");
    }
}

const string channelDataTypeToPrefix(cl_channel_type _ct)
{
    std::string toReturn = channelTypeToString(_ct);
    // channel_type starts with CL_ that we need to cut
    toReturn = toReturn.substr(strlen("CL_"), toReturn.size());
    return toReturn;
}

const string VecSizeToPrefix(VecSize _size)
{
    switch(_size)
    {
    case SCALAR:
        return "";
    case SOA4:
        return "soa4_";
    case SOA8:
        return "soa8_";
    case SOA16:
        return "soa16_";
    default:
        throw Exceptions::DeviceBackendExceptionBase(std::string("Internal error. Unsupported vector size"));
    }
}

const string FilterToPrefix(cl_filter_mode _filterMode)
{
    switch(_filterMode)
    {
    case CL_FILTER_LINEAR:
        return "LINEAR";
    case CL_FILTER_NEAREST:
        return "NEAREST";
    default:
        throw Exceptions::DeviceBackendExceptionBase(std::string("Internal error. Unsupported filter mode"));
    }
}

std::string ImageCallbackLibrary::getLibraryBasename()
{
    char szModuleName[MAX_PATH];
    std::string strErr;

    Utils::SystemInfo::GetModuleDirectory(szModuleName, MAX_PATH);

    //Klocwork warning - false alarm the Id is always in correct bounds
    const char* pCPUPrefix = m_CpuId.GetCPUPrefix(); 
    if(pCPUPrefix == NULL)
      throw Exceptions::DeviceBackendExceptionBase(std::string("Internal error. NULL CPU prefix"));


    std::string ret = std::string(szModuleName)
                    + "clbltfn"
                    + std::string(pCPUPrefix)
                    + "_img_cbk";

    assert(ret.length() <= MAX_PATH);
    return ret;
}

std::string ImageCallbackLibrary::getLibraryObjectName()
{
  return getLibraryBasename() + ".o";
}

std::string ImageCallbackLibrary::getLibraryRtlName()
{
  return getLibraryBasename() + ".rtl";
}
UndefCbkDesc::UndefCbkDesc(UndefCbkType _type, VecSize _vecSize):
Type(_type),
Size(_vecSize)
{}

std::string UndefCbkDesc::GetName() const
{
    std::string vecStr = VecSizeToPrefix(Size);
    switch(Type)
    {
    case READ_CBK_UNDEF_INT:
        return vecStr + "read_sample_UNDEFINED_QUAD_INT";
    case READ_CBK_UNDEF_FLOAT:
        return vecStr + "read_sample_UNDEFINED_QUAD_FLOAT";
    case TRANS_CBK_UNDEF_FLOAT:
        return vecStr + "trans_coord_float_UNDEFINED";
    case TRANS_CBK_UNDEF_FLOAT_FLOAT:
        return vecStr + "trans_coord_float_float_UNDEFINED";
    default:
        throw Exceptions::DeviceBackendExceptionBase("Type of undefined callback is invalid!");
    }
}

TransCbkDesc::TransCbkDesc(bool _isInt, SamplerType _sampler, VecSize _vectorSize):
    IsIntFormat(_isInt),
    Sampler(_sampler),
    VectorSize(_vectorSize)
{}

std::string TransCbkDesc::GetName() const
{
    std::stringstream ss;
    ss << VecSizeToPrefix(VectorSize);
    ss << "trans_coord_float_";
    if(!IsIntFormat)
        ss << "float_";

    ss << samplerToAddrModePrefix(Sampler) << "_";
    std::string isNormalizedStr = (Sampler & CLK_NORMALIZED_COORDS_TRUE) ?
      "TRUE":
      "FALSE";
    ss << isNormalizedStr << "_";

    if(Sampler & CLK_FILTER_LINEAR)
        ss << "LINEAR";
    else
        ss << "NEAREST";

    return ss.str();
}

ReadCbkDesc::ReadCbkDesc(bool _isClamp,
cl_channel_order _ch_order,
cl_channel_type _ch_type,
cl_filter_mode _filter,
cl_mem_object_type _imageType,
VecSize _vectorSize):
    IsClamp(_isClamp),
    Filter(_filter),
    ImageType(_imageType),
    VectorSize(_vectorSize)
{
    Format.image_channel_order = _ch_order;
    Format.image_channel_data_type = _ch_type;
}

std::string ReadCbkDesc::GetName() const
{
    std::stringstream ss ;
    ss << VecSizeToPrefix(VectorSize);
    ss << "read_sample_";
    if(Filter == CL_FILTER_NEAREST)
        ss << "NEAREST" << "_";
    else
        ss << "LINEAR" << imgTypeToDimPrefix(ImageType) << "_";
    std::string clampStr = IsClamp ? "CLAMP" : "NO_CLAMP";
    ss << clampStr <<  "_";
    ss << channelOrderToPrefix(Format.image_channel_order) << "_";
    ss << channelDataTypeToPrefix(Format.image_channel_data_type);
    return ss.str();
}

WriteCbkDesc::WriteCbkDesc(cl_channel_order _ch_order, cl_channel_type _ch_type, VecSize _vectorSize)
{
    Format.image_channel_data_type = _ch_type;
    Format.image_channel_order = _ch_order;
    VectorSize = _vectorSize;
}

std::string WriteCbkDesc::GetName() const
{
    std::stringstream ss;
    ss<<VecSizeToPrefix(VectorSize);
    ss<<"write_sample_";
    ss<<channelOrderToPrefix(Format.image_channel_order)<<"_";
    ss<<channelDataTypeToPrefix(Format.image_channel_data_type);
    return ss.str();
}

void ImageCallbackLibrary::Load()
{
    // Load built-ins module IR from an rtl file.
    llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> rtlBufferOrErr =
        llvm::MemoryBuffer::getFile(getLibraryRtlName().c_str());
    if( !rtlBufferOrErr )
    {
        throw Exceptions::DeviceBackendExceptionBase(
          std::string("Failed to load the image callback rtl library"));
    }

    // read IR into a Module
    m_pRtlBuffer.reset(rtlBufferOrErr.get().release());
    llvm::Module* M = m_Compiler->ParseModuleIR(m_pRtlBuffer.get());

    // create an execution engine (which assumes ownership of M)
    m_Compiler->CreateExecutionEngine(M);
    std::unique_ptr<llvm::ExecutionEngine> EE (
      static_cast<llvm::ExecutionEngine*>(m_Compiler->GetExecutionEngine()));

    // initialize the object cache with the path to the pre-compiled image
    // callback library object.
    m_pLoader->addLocation(M, getLibraryObjectName());
    EE->setObjectCache(m_pLoader.get());

    // put the module and the execution engine in a container
    m_pCompiledModule.reset(new CompiledModule(M, EE.release()));
}

void ImageCallbackLibrary::Build()
{
    m_ImageFunctions = new ImageCallbackFunctions(m_pCompiledModule.get());
}

// For CPU this should be left empty
// TODO: Add implementation for MIC support
bool ImageCallbackLibrary::LoadExecutable()
{
    return true;
}

ImageCallbackFunctions::ImageCallbackFunctions(CompiledModule* pCompiledModule)
    :m_pCompiledModule(pCompiledModule)
{
}

void* ImageCallbackFunctions::GetCbkPtr(const CbkDesc& _desc)
{
    std::string name = _desc.GetName();
    llvm::StringRef fName(name);
    llvm::Function* fncPtr = m_pCompiledModule->getFunction(fName);
    if(fncPtr == NULL)
    {
        std::stringstream ss;
        ss << "Image function " << _desc.GetName() << " wasn't found in the module. Make sure image libraries are valid";
        throw Exceptions::DeviceBackendExceptionBase(ss.str());
    }

    void* ptr = m_pCompiledModule->getPointerToFunction(fncPtr);
    if(ptr == NULL)
    {
        std::stringstream ss;
        ss << "Internal error. Failed to retreive pointer to function " << _desc.GetName();
        throw Exceptions::DeviceBackendExceptionBase(ss.str());
    }
    return ptr;
}

struct TrapDescriptor: public CbkDesc{
    std::string GetName() const { return TRAP_NAME; }
};

void* ImageCallbackFunctions::GetTrapCbk()
{
    TrapDescriptor Desc;
    return GetCbkPtr(Desc);
}

ImageCallbackLibrary::~ImageCallbackLibrary()
{
    if (m_ImageFunctions) 
        delete m_ImageFunctions;

    // Module must be freed before compiler.
    delete m_pCompiledModule.release();
    delete m_Compiler;
}

}}} // namespace
