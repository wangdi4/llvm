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

#include "BuiltinModule.h"
#include "CPUDetect.h"
#include "CPUCompiler.h"
#include "llvm/Module.h"
#include "llvm/Support/MemoryBuffer.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

// number of reading callbacks = channel types count (15) * channel order count(10)
#define TYPES_COUNT 15
#define ORDERS_COUNT 10
#define MAX_NUM_FORMATS TYPES_COUNT * ORDERS_COUNT
// for given image channel type and channel order returns index in callback array
#define TYPE_ORDER_TO_INDEX(t,o) (t*ORDERS_COUNT+o);
// Number of traslate callbacks is equal to number of possible samplers.
// So number of callbacks is (addressing modes count (4)) *
// (normalization options count, normalized or not (2)) *
// (filtering mode count, linear or nearest (2))
#define ADDR_MODE_COUNT 4
#define NORM_OPTIONS 2
#define FILTER_OPTIONS 2
#define TRANS_NUM ADDR_MODE_COUNT * NORM_OPTIONS * FILTER_OPTIONS

// TODO: ensure it is defined from cl_image_declaration.h
// indexes of translation callbacks for each type of sampler
// !!!IMPORTANT!!! These defines should be the same as in cl_image_declaration.h
#define NONE_FALSE_NEAREST 0x00
#define CLAMP_FALSE_NEAREST 0x01
#define CLAMPTOEDGE_FALSE_NEAREST 0x02
#define REPEAT_FALSE_NEAREST 0x03
#define MIRRORED_FALSE_NEAREST 0x04

#define NONE_TRUE_NEAREST 0x08
#define CLAMP_TRUE_NEAREST 0x09
#define CLAMPTOEDGE_TRUE_NEAREST 0x0a
#define REPEAT_TRUE_NEAREST 0x0b
#define MIRRORED_TRUE_NEAREST 0x0c

#define NONE_FALSE_LINEAR 0x10
#define CLAMP_FALSE_LINEAR 0x11
#define CLAMPTOEDGE_FALSE_LINEAR 0x12
#define REPEAT_FALSE_LINEAR 0x13
#define MIRRORED_FALSE_LINEAR 0x14

#define NONE_TRUE_LINEAR 0x18
#define CLAMP_TRUE_LINEAR 0x19
#define CLAMPTOEDGE_TRUE_LINEAR 0x1a
#define REPEAT_TRUE_LINEAR 0x1b
#define MIRRORED_TRUE_LINEAR 0x1c

// Returns size of array
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) \
  ((sizeof(a) / sizeof(*(a))) / \
  static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))
#endif

// Auxiliary class for storing image function pointer and name
class FuncDesc
{
public:

    FuncDesc():
      m_fncPtr(NULL)
    {}

    void Init(llvm::Function* pFunc)
    {
        assert(pFunc != NULL);
        m_fncPtr = pFunc;
    }

    void* GetPtr(CPUCompiler* pCompiler)
    {
        assert(pCompiler != NULL);
        if(m_fncPtr == NULL)
            throw Exceptions::DeviceBackendExceptionBase(std::string("Images internal error. Uninitialized callback function requested"));
        // Request pointer to function. If it is the first request function is being JITted.
        void* ptr = pCompiler->GetPointerToFunction(m_fncPtr);
        if( ptr == NULL){
            throw Exceptions::DeviceBackendExceptionBase(std::string("Internal error occurred while jitting image functions"));
        }
        return ptr;
    }

private:
    /// Pointer to jitted funciton
    llvm::Function* m_fncPtr;
};

/**
 *  Holds the entire callback functions set for a specific compiled library (i.e., per architecture)
 */
class ImageCallbackFunctions{
public:

    ImageCallbackFunctions(llvm::Module* pImagesRTModule, CPUCompiler* pCompiler);

private:

#define DECLARE_IMAGE_CALLBACK(NAME)\
private:\
    FuncDesc m_fp##NAME;\
public:\
    void* Get##NAME()\
    {\
        return m_fp##NAME.GetPtr(m_pCompiler);\
    }

#define DECLARE_CALLBACK_ARRAY(NAME, COUNT)\
private:\
    FuncDesc m_fp##NAME[COUNT];\
public:\
    void* Get##NAME(int idx)\
    {\
        assert(idx < COUNT);\
        return m_fp##NAME[idx].GetPtr(m_pCompiler);\
    }

    //float coordinate translation callbacks
    DECLARE_IMAGE_CALLBACK(FNoneFalseNearest)
    DECLARE_IMAGE_CALLBACK(FClampToEdgeFalseNearest)
    DECLARE_IMAGE_CALLBACK(FUndefTrans)
    DECLARE_IMAGE_CALLBACK(FFUndefTrans)

    DECLARE_IMAGE_CALLBACK(FNoneTrueNearest)
    DECLARE_IMAGE_CALLBACK(FClampToEdgeTrueNearest)
    DECLARE_IMAGE_CALLBACK(FRepeatTrueNearest)
    DECLARE_IMAGE_CALLBACK(FMirroredTrueNearest)

    DECLARE_IMAGE_CALLBACK(FNoneFalseLinear)
    DECLARE_IMAGE_CALLBACK(FClampToEdgeFalseLinear)

    DECLARE_IMAGE_CALLBACK(FNoneTrueLinear)
    DECLARE_IMAGE_CALLBACK(FClampToEdgeTrueLinear)
    DECLARE_IMAGE_CALLBACK(FRepeatTrueLinear)
    DECLARE_IMAGE_CALLBACK(FMirroredTrueLinear)

    DECLARE_IMAGE_CALLBACK(FFNoneFalseNearest)
    DECLARE_IMAGE_CALLBACK(FFClampToEdgeFalseNearest)

    DECLARE_IMAGE_CALLBACK(FFNoneTrueNearest)
    DECLARE_IMAGE_CALLBACK(FFClampToEdgeTrueNearest)
    DECLARE_IMAGE_CALLBACK(FFRepeatTrueNearest)
    DECLARE_IMAGE_CALLBACK(FFMirroredTrueNearest)

    //image reading callbacks
    DECLARE_CALLBACK_ARRAY(NearestNoClamp, MAX_NUM_FORMATS)
    DECLARE_CALLBACK_ARRAY(NearestClamp, MAX_NUM_FORMATS)
    DECLARE_CALLBACK_ARRAY(LinearNoClamp2D, MAX_NUM_FORMATS)
    DECLARE_CALLBACK_ARRAY(LinearClamp2D, MAX_NUM_FORMATS)
    DECLARE_CALLBACK_ARRAY(LinearNoClamp3D, MAX_NUM_FORMATS)
    DECLARE_CALLBACK_ARRAY(LinearClamp3D, MAX_NUM_FORMATS)
    DECLARE_CALLBACK_ARRAY(LinearNoClamp1D, MAX_NUM_FORMATS)
    DECLARE_CALLBACK_ARRAY(LinearClamp1D, MAX_NUM_FORMATS)
    DECLARE_IMAGE_CALLBACK(UndefReadQuad)

    // image writing callbackS
    DECLARE_CALLBACK_ARRAY(WriteImage, MAX_NUM_FORMATS)

    // Used for Lazy compilation. Not owned by this class
    CPUCompiler* m_pCompiler;
};

/**
 *  Provides loading and building image library for specified cpu. Owns compiler instance
 */
class ImageCallbackLibrary{
public:
    /**
    *  ctor
    */
    ImageCallbackLibrary(Intel::ECPU cpuId, unsigned int cpuFeatures, CPUCompiler* compiler):
      m_CpuId(cpuId), m_CpuFeatures(cpuFeatures), m_ImageFunctions(NULL), m_Compiler(compiler),
      m_pRtlBuffer(NULL), m_pModule(NULL) { assert(m_CpuId>=CPU_PENTIUM && m_CpuId<CPU_LAST); }

    /**
    *  Loads image module from platform-specific rtl file
    */
    void Load();

    /**
    *  Builds preliminarily loaded image module. Should only be called after Load call
    */
    bool Build();
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

    ~ImageCallbackLibrary()
    {
        if (m_ImageFunctions) 
            delete m_ImageFunctions;
        if(m_Compiler)
            delete m_Compiler;
    }

private:
    Intel::ECPU m_CpuId;
    unsigned int m_CpuFeatures;
    // Instance with all function pointers. Owned by this class
    ImageCallbackFunctions* m_ImageFunctions;
    // Pointer to Compiler. Owned by this class.
    // TODO: this pointer should be changed to Compiler* instead of CPUCompiler
    // to enable MICSupport
    CPUCompiler* m_Compiler;
    // pointer to library Rtl Buffer. Owned by this class
    llvm::OwningPtr<llvm::MemoryBuffer> m_pRtlBuffer;
    // Pointer to built images module. Owned by m_Compiler
    llvm::Module* m_pModule;
};

}}} // namespace
