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


/**
 *  Holds the entire callback functions set for a specific compiled library (i.e., per architecture)
 */
class ImageCallbackFunctions{
public:

    ImageCallbackFunctions(llvm::Module* pImagesRTModule, CPUCompiler* pCompiler);

    //integer coordinate translate callbacks
    void* m_fpINoneFalseNearest; 
    void* m_fpIClampToEdgeFalseNearest;
    void* m_fpIUndefTrans;

    //float coordinate translation callbacks
    void *m_fpFNoneFalseNearest;
    void *m_fpFClampToEdgeFalseNearest;
    void *m_fpFUndefTrans;
    void *m_fpFFUndefTrans;
    
    void *m_fpFNoneTrueNearest;
    void *m_fpFClampToEdgeTrueNearest;
    void *m_fpFRepeatTrueNearest;
    void *m_fpFMirroredTrueNearest;

    void *m_fpFNoneFalseLinear;
    void *m_fpFClampToEdgeFalseLinear;
    
    void *m_fpFNoneTrueLinear;
    void *m_fpFClampToEdgeTrueLinear;
    void *m_fpFRepeatTrueLinear;
    void *m_fpFMirroredTrueLinear;

    void *m_fpFFNoneFalseNearest;
    void *m_fpFFClampToEdgeFalseNearest;

    void *m_fpFFNoneTrueNearest;
    void *m_fpFFClampToEdgeTrueNearest;
    void *m_fpFFRepeatTrueNearest;
    void *m_fpFFMirroredTrueNearest;

    //image reading callbacks
    void *m_fpNearestNoClamp[MAX_NUM_FORMATS];
    void *m_fpNearestClamp[MAX_NUM_FORMATS];
    void *m_fpLinearNoClamp2D[MAX_NUM_FORMATS];
    void *m_fpLinearClamp2D[MAX_NUM_FORMATS];
    void *m_fpLinearNoClamp3D[MAX_NUM_FORMATS];
    void *m_fpLinearClamp3D[MAX_NUM_FORMATS];
    void *m_fpUndefReadQuad;

    // image writing callbackS
    void* m_fpWriteImage[MAX_NUM_FORMATS];
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
        if(m_pRtlBuffer)
            delete m_pRtlBuffer;
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
    llvm::MemoryBuffer* m_pRtlBuffer;
    // Pointer to built images module. Owned by m_Compiler
    llvm::Module* m_pModule;
};

}}} // namespace
