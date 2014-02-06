/*****************************************************************************\

Copyright 2000 - 2008 Intel Corporation All Rights Reserved.

    The source code contained or described herein and all documents related to
    the source code ("Material") are owned by Intel Corporation or its suppliers
    or licensors. Title to the Material remains with Intel Corporation or its
    suppliers and licensors. The Material contains trade secrets and proprietary
    and confidential information of Intel or its suppliers and licensors. The
    Material is protected by worldwide copyright and trade secret laws and
    treaty provisions. No part of the Material may be used, copied, reproduced,
    modified, published, uploaded, posted, transmitted, distributed, or
    disclosed in any way without Intel's prior express written permission.

    No license under any patent, copyright, trade secret or other intellectual
    property right is granted to or conferred upon you by disclosure or delivery
    of the Materials, either expressly, by implication, inducement, estoppel or
    otherwise. Any license under such intellectual property rights must be
    express and approved by Intel in writing.

File Name: translationblock.h
Abstract:

Notes:

\*****************************************************************************/
#pragma once

#include <stdint.h>
#include <stddef.h>

#if defined(_WIN32)
    #include <windows.h>

    // INSIDE_PLUGIN must be defined in the pre-processor definitions of the
    // CTranslationBlock DLL project
    #ifndef TRANSLATION_BLOCK_API
        #ifdef INSIDE_PLUGIN
            #define TRANSLATION_BLOCK_API __declspec(dllexport)
        #else
            #define TRANSLATION_BLOCK_API __declspec(dllimport)
        #endif
    #endif
#else
    #ifndef TRANSLATION_BLOCK_API
        #define TRANSLATION_BLOCK_API __attribute__((visibility("default")))
    #endif
#endif

namespace TC
{
static const uint32_t STB_VERSION = 1004UL;
static const uint32_t STB_MAX_ERROR_STRING_SIZE = 1024UL;

// Forward prototyping
struct STB_RegisterArgs;
struct STB_CreateArgs;
class  CTranslationBlock;
struct STB_GetKernelArgsInfoArgs;

#ifdef _MSC_VER
extern "C" TRANSLATION_BLOCK_API void __cdecl Register( STB_RegisterArgs* pRegisterArgs );
extern "C" TRANSLATION_BLOCK_API CTranslationBlock* __cdecl Create( STB_CreateArgs* pCreateArgs );
extern "C" TRANSLATION_BLOCK_API void __cdecl Delete( CTranslationBlock* pBlock );
extern "C" TRANSLATION_BLOCK_API void __cdecl GetKernelArgsInfo( const void *pBin, const char *szKernelName, STB_GetKernelArgsInfoArgs* pKernelArgsInfo );
extern "C" TRANSLATION_BLOCK_API void __cdecl ReleaseKernelArgsInfo( STB_GetKernelArgsInfoArgs* pKernelArgsInfo );
#else 
// Functions required to be defined by all translation blocks
extern "C" TRANSLATION_BLOCK_API void __attribute__((cdecl))  Register( STB_RegisterArgs* pRegisterArgs );
extern "C" TRANSLATION_BLOCK_API CTranslationBlock* __attribute__((cdecl))  Create( STB_CreateArgs* pCreateArgs );
extern "C" TRANSLATION_BLOCK_API void __attribute__((cdecl))  Delete( CTranslationBlock* pBlock );
extern "C" TRANSLATION_BLOCK_API void __attribute__((cdecl))  GetKernelArgsInfo( const void *pBin, const char *szKernelName, STB_GetKernelArgsInfoArgs* pKernelArgsInfo );
extern "C" TRANSLATION_BLOCK_API void __attribute__((cdecl))  ReleaseKernelArgsInfo( STB_GetKernelArgsInfoArgs* pKernelArgsInfo );
#endif

/******************************************************************************\
ENUMERATION: TB_DEVICE_TYPE
DESCRIPTION: Device type to identify the required operation path
\******************************************************************************/
enum TB_DEVICE_TYPE
{
    TB_DEVICE_UNKNOWN,
    TB_DEVICE_GPU,
    TB_DEVICE_CPU,
    TB_DEVICE_MIC,
    TB_NUM_DEVICE_TYPES,
};

/******************************************************************************\

Enumeration:
    TB_DATA_FORMAT

Description:
    Possible i/o formats for the translation classes

\******************************************************************************/
enum TB_DATA_FORMAT
{
    TB_DATA_FORMAT_UNKNOWN,
    TB_DATA_FORMAT_OCL_TEXT,
    TB_DATA_FORMAT_OCL_BINARY,
    TB_DATA_FORMAT_LLVM_TEXT,
    TB_DATA_FORMAT_LLVM_BINARY,
    TB_DATA_FORMAT_GHAL_TEXT,
    TB_DATA_FORMAT_GHAL_BINARY,
    TB_DATA_FORMAT_DEVICE_TEXT,
    TB_DATA_FORMAT_DEVICE_BINARY,
    TB_DATA_FORMAT_LLVM_ARCHIVE,
    TB_DATA_FORMAT_ELF,
    TB_DATA_FORMAT_RS_LLVM_BINARY,
    TB_DATA_FORMAT_RS_INFO,
    NUM_TB_DATA_FORMATS
};

/******************************************************************************\

Structure:
    STB_TranslationCode

Description:
    Structure used to describe the requested translation type

\******************************************************************************/
union STB_TranslationCode
{
    struct
    {
        TB_DATA_FORMAT InputType  : 16;
        TB_DATA_FORMAT OutputType : 16;
    };

    uint32_t Code;
};

/******************************************************************************\

Structure:
    STB_CreateArgs

Description:
    Structure used to store arguments used to pass data to the Create function

\******************************************************************************/
struct STB_CreateArgs
{
    STB_TranslationCode TranslationCode;
    TB_DEVICE_TYPE      deviceType;
    void*               pCreateData;

    STB_CreateArgs()
    {
        TranslationCode.Code = 0;
        deviceType = TB_DEVICE_UNKNOWN;
        pCreateData = NULL;
    }
};

/******************************************************************************\

Structure:
    STB_RegisterArgs

Description:
    Structure containing a pointer to an array of supported translation codes
    and a variable informing us of the size of the translation code array.

    The calling function is responsible for deleting the memory allocated
    for the translation code array.

Note:
    Version is contained in this header

\******************************************************************************/
struct STB_RegisterArgs
{
    uint32_t                 Version;
    uint32_t                 NumTranslationCodes;
    STB_TranslationCode*     pTranslationCodes;

    STB_RegisterArgs()
    {
        Version = STB_VERSION;
        NumTranslationCodes = 0;
        pTranslationCodes = NULL;
    }
};

/******************************************************************************\

Structure:
    STB_TranslateInputArgs

Description:
    Structure used to pass input variables to the translation block

\******************************************************************************/
struct STB_TranslateInputArgs
{
    char*       pInput;              // data to be translated
    uint32_t    InputSize;           // size of data to be translated
    const char* pOptions;            // list of build/compile options
    uint32_t    OptionsSize;         // size of options list
    const char* pInternalOptions;    // list of build/compile options
    uint32_t    InternalOptionsSize; // size of options list
    void*       pTracingOptions;     // instrumentation options
    uint32_t    TracingOptionsCount; // number of instrumentation options 

    STB_TranslateInputArgs()
    {
        pInput              = NULL;
        InputSize           = 0;
        pOptions            = NULL;
        OptionsSize         = 0;
        pInternalOptions    = NULL;
        InternalOptionsSize = 0;
        pTracingOptions     = NULL;
        TracingOptionsCount = 0;
    }
};

/******************************************************************************\

Structure:
    STB_TranslateOutputArgs

Description:
    Structure used to hold data returned from the translation block

\******************************************************************************/
struct STB_TranslateOutputArgs
{
    char*       pOutput;         // pointer to translated data buffer
    uint32_t    OutputSize;      // translated data buffer size (bytes)
    char*       pErrorString;    // string to print if translate fails
    uint32_t    ErrorStringSize; // size of error string

    STB_TranslateOutputArgs()
    {
        pOutput         = NULL;
        OutputSize      = 0;
        pErrorString    = NULL;
        ErrorStringSize = 0;
    }
};

/******************************************************************************\

Structure:
    STB_GetKernelArgsInfoArgs

Description:
    Structure used to hold data returned from the GetKernelArgeInfo

\******************************************************************************/
enum TB_KERNEL_ARG_ACC_QUAL
{
    TB_KERNEL_ARG_ACC_QUAL_READ_ONLY  = 0,
    TB_KERNEL_ARG_ACC_QUAL_WRITE_ONLY = 1,
    TB_KERNEL_ARG_ACC_QUAL_READ_WRITE = 2,
    TB_KERNEL_ARG_ACC_QUAL_NONE       = 3
};

enum TB_KERNEL_ARG_TYPE_QUAL
{
    TB_KERNEL_ARG_TYPE_QUAL_NONE     = 0,
    TB_KERNEL_ARG_TYPE_QUAL_CONST    = 1,
    TB_KERNEL_ARG_TYPE_QUAL_RESTRICT = 2,
    TB_KERNEL_ARG_TYPE_QUAL_VOLATILE = 4
};

struct ARG_INFO
{
    char* name;                  // pointer to name specified for the argument
    char* typeName;              // pointer to type name specified for the argument
    uint64_t adressQualifier;    // address qualifier specified for the argument
    uint64_t accessQualifier;    // access qualifier specified for the argument
    uint64_t typeQualifier;      // type qualifier specified for the argument
};

struct STB_GetKernelArgsInfoArgs
{
    uint32_t    m_numArgs;      // number of arguments to the kernel
    ARG_INFO*   m_argsInfo;     // pointer to argument info structures
	int32_t     m_retValue;     // return value

    STB_GetKernelArgsInfoArgs()
    {
        m_numArgs     = 0;
        m_argsInfo    = NULL;
        m_retValue    = 0;
    }
};

/******************************************************************************\
Class:
    CTranslationBlock

Description:
    Interface used to expose required functions to translation plug-ins
\******************************************************************************/
class CTranslationBlock
{
public:
    virtual bool Translate(
        const STB_TranslateInputArgs* pInput,
        STB_TranslateOutputArgs* pOutput ) = 0;

    virtual bool FreeAllocations( STB_TranslateOutputArgs* pOutput ) = 0;

    virtual bool GetOpcodes( void* pOpcodes ) { return false; }
    virtual bool GetOpcodesCount( uint32_t* pOpcodesCount, uint32_t* pOpcodeSize ){ return false; }

    CTranslationBlock() {}
    virtual ~CTranslationBlock() {}
};

} // namespace TC
