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

File Name:  ImageCallbackLibrary.h

\*****************************************************************************/
#pragma once

#include "BuiltinModules.h"
#include "CompiledModule.h"
#include "CPUDetect.h"
#include "CPUCompiler.h"
#include "StaticObjectLoader.h"
#include "llvm/ExecutionEngine/ObjectCache.h"
#include "cl_utils.h"

#include <memory>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

// TODO: ensure it is defined from cl_image_declaration.h
// indexes of translation callbacks for each type of sampler
// !!!IMPORTANT!!! These defines should be the same as in cl_image_declaration.h
enum SamplerType
{
    NONE_FALSE_NEAREST          =0x10,
    CLAMP_FALSE_NEAREST         =0x14,
    CLAMPTOEDGE_FALSE_NEAREST   =0x12,
    REPEAT_FALSE_NEAREST        =0x16,
    MIRRORED_FALSE_NEAREST      =0x18,
    
    NONE_TRUE_NEAREST           =0x11,
    CLAMP_TRUE_NEAREST          =0x15,
    CLAMPTOEDGE_TRUE_NEAREST    =0x13,
    REPEAT_TRUE_NEAREST         =0x17,
    MIRRORED_TRUE_NEAREST       =0x19,
    
    NONE_FALSE_LINEAR           =0x20,
    CLAMP_FALSE_LINEAR          =0x24,
    CLAMPTOEDGE_FALSE_LINEAR    =0x22,
    REPEAT_FALSE_LINEAR         =0x26,
    MIRRORED_FALSE_LINEAR       =0x28,
    
    NONE_TRUE_LINEAR            =0x21,
    CLAMP_TRUE_LINEAR           =0x25,
    CLAMPTOEDGE_TRUE_LINEAR     =0x23,
    REPEAT_TRUE_LINEAR          =0x27,
    MIRRORED_TRUE_LINEAR        =0x29,
    
    SAMPLER_UNDEFINED = 0xFF
};

// Describes callback type that is used if read_image is called
// with parameters that by spec produce undefined return value
enum UndefCbkType
{
    // Undefined reading callback with integer coordinates
    READ_CBK_UNDEF_INT,
    // Undefined reading callback with floating point coordinates
    READ_CBK_UNDEF_FLOAT,
    // Undefined translation callback for floating point coordinates but integer image
    TRANS_CBK_UNDEF_FLOAT,
    // Undefined translation callback for floating poitn coordinates and float image
    TRANS_CBK_UNDEF_FLOAT_FLOAT
};

// callback vector size
enum VecSize
{
    SCALAR = 1,
    SOA4 = 4,
    SOA8 = 8
};

// Auxiliary functions for image callback names mangling
const string channelOrderToPrefix(cl_channel_order _co);
const string samplerToAddrModePrefix(SamplerType _sampler);
const string imgTypeToDimPrefix(cl_mem_object_type _type);
const string channelDataTypeToPrefix(cl_channel_type _ct);
const string VecSizeToPrefix(VecSize _size);
const string FilterToPrefix(cl_filter_mode _filterMode);

class CbkDesc
{
public:
    virtual std::string GetName() const = 0;
};

class UndefCbkDesc : public CbkDesc
{
public:

    UndefCbkDesc(UndefCbkType _type, VecSize _vecSize = SCALAR);
    virtual std::string GetName() const;

private:
    UndefCbkType Type;
    VecSize Size;
};

class TransCbkDesc : public CbkDesc
{
public:

    TransCbkDesc(bool _isInt, SamplerType _sampler, VecSize _vectorSize = SCALAR);
    virtual std::string GetName() const;

private:
    bool IsIntFormat;
    SamplerType Sampler;
    VecSize VectorSize;
};

class ReadCbkDesc : public CbkDesc
{
public:
    ReadCbkDesc(bool _isClamp,
        cl_channel_order _ch_order,
        cl_channel_type _ch_type,
        cl_filter_mode _filter,
        // For regular 1.1 images reading callbacks are common for 2D and 3D
        // so set 2D here for generality
        cl_mem_object_type _imageType = CL_MEM_OBJECT_IMAGE2D,
        VecSize _vectorSize = SCALAR);

    virtual std::string GetName() const;

private:
    cl_image_format     Format;
    bool                IsClamp;
    cl_filter_mode      Filter;
    cl_mem_object_type  ImageType;
    VecSize             VectorSize;
};

class WriteCbkDesc : public CbkDesc
{
public:

    WriteCbkDesc(cl_channel_order _ch_order, cl_channel_type _ch_type, VecSize _vectorSize = SCALAR);
    // Returns llvm name of a function
    virtual std::string GetName() const;

private:
    VecSize VectorSize;
    cl_image_format Format;
};

const bool FLT_CBK = false;
const bool INT_CBK = !FLT_CBK;

const bool CLAMP_CBK = true;
const bool NO_CLAMP_CBK = !CLAMP_CBK;

// Returns size of array
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) \
  ((sizeof(a) / sizeof(*(a))) / \
  static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))
#endif

/**
 *  Holds the entire callback functions set for a specific compiled library (i.e., per architecture)
 */
class ImageCallbackFunctions{
public:

    ImageCallbackFunctions(CompiledModule* pCompiledModule);

private:

    // Used for function lookup. Not owned by this class
    CompiledModule* m_pCompiledModule;

    void* GetCbkPtr(const CbkDesc& _desc);

public:

    void* GetUndefinedCbk(UndefCbkType _type,
                   VecSize _vecSize = SCALAR)
    {
        UndefCbkDesc desc(_type, _vecSize);
        return GetCbkPtr(desc);
    }

    void* GetTranslationCbk(bool _isInt, SamplerType _sampler)
    {
        TransCbkDesc desc(_isInt, _sampler);
        return GetCbkPtr(desc);
    }

    void* GetReadingCbk(bool _isClamp,
                        cl_channel_order _ch_order,
                        cl_channel_type _ch_type,
                        cl_filter_mode _filter,
                        cl_mem_object_type _imageType = CL_MEM_OBJECT_IMAGE2D,
                        VecSize _vecSize = SCALAR)
    {
        ReadCbkDesc desc(_isClamp, _ch_order, _ch_type, _filter, _imageType, _vecSize);
        return GetCbkPtr(desc);
    }

    void* GetWritingCbk(cl_channel_order _ch_order, cl_channel_type _ch_type, VecSize _vectorSize = SCALAR)
    {
        WriteCbkDesc desc(_ch_order, _ch_type, _vectorSize);
        return GetCbkPtr(desc);
    }

    void* GetTrapCbk();

};

/**
 *  Provides loading and building image library for specified cpu. Owns compiler instance
 */
class ImageCallbackLibrary{
public:
    /**
    *  ctor
    */
    ImageCallbackLibrary(Intel::CPUId cpuId, CPUCompiler* compiler):
      m_CpuId(cpuId), m_ImageFunctions(NULL), m_Compiler(compiler),
      m_pRtlBuffer(nullptr), m_pLoader(new StaticObjectLoader)
    { }

    /**
    *  Loads image module from platform-specific rtl file
    */
    void Load();

    /**
    *  Populates m_ImageFunctions. Should only be called after Load call
    */
    void Build();
    /**
    *  MIC-specific: serialize library and load it to the device
    *  Then send address back to the host
    */
    bool LoadExecutable();
    /**
    *  Returns pointer to object with previously built image function pointers.
    *  library should already be loaded and built
    */
    ImageCallbackFunctions* getImageCallbackFunctions(){ return m_ImageFunctions; }

    ~ImageCallbackLibrary();

private:

    /// Get the path to the builtin library
    std::string getLibraryBasename();
    std::string getLibraryRtlName();
    std::string getLibraryObjectName();

    Intel::CPUId m_CpuId;
    // Instance with all function pointers. Owned by this class
    ImageCallbackFunctions* m_ImageFunctions;
    // Pointer to Compiler. Owned by this class.
    // TODO: this pointer should be changed to Compiler* instead of CPUCompiler
    // to enable MICSupport
    CPUCompiler* m_Compiler;
    // pointer to library Rtl Buffer. Owned by this class
    std::unique_ptr<llvm::MemoryBuffer> m_pRtlBuffer;

    // Pointer to OCL implementation of llvm::ObjectCache used to load from disk
    // object files which have been statically compiled (i.e. by MCJIT or llc)
    std::unique_ptr<StaticObjectLoader> m_pLoader;

    // Pointer to CompiledModule instance (contains ExecutionEngine)
    std::unique_ptr<CompiledModule> m_pCompiledModule;

private:
    // Disable copy ctor and assignment operator
    ImageCallbackLibrary( const ImageCallbackLibrary& );
    bool operator = (const ImageCallbackLibrary& );

};

}}} // namespace
