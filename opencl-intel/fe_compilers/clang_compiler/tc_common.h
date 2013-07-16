/******************************************************************************\

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

File Name: tc_common.h

Abstract: 

Notes: 

\******************************************************************************/
#pragma once

#include <map>
#include <vector>

#include "os_inc.h"
#include "TranslationBlock.h"

namespace TC
{

/******************************************************************************\

Callback Function Pointer: 
    PFNNOTIFY

Description:
    Function pointer used to callback into the client and inform it that the
    translation is complete.

\******************************************************************************/
typedef void ( OSAPI *PFNNOTIFY )( void *pProgram, void* userData );

/******************************************************************************\
DEFINE: TC_INVALID_INDEX
\******************************************************************************/
#define TC_INVALID_INDEX ( 0xffff )

/******************************************************************************\
ENUMERATION: TC_RETVAL
DESCRIPTION: common return values for translation controller functions
\******************************************************************************/
enum TC_RETVAL
{
    TC_SUCCESS = 0,
    TC_ERROR,
};

/******************************************************************************\
ENUMERATION: TC_CHAIN_TYPE
DESCRIPTION: Compiler chains that are used to specify a build order
\******************************************************************************/
enum TC_CHAIN_TYPE
{
    TC_CHAIN_BUILD,
    TC_CHAIN_CPU_BUILD,
    TC_CHAIN_MIC_BUILD,
    TC_CHAIN_COMPILE,
    TC_CHAIN_LINK_LIB,
    TC_CHAIN_LINK_EXE,
    TC_CHAIN_CPU_LINK_EXE,
    TC_CHAIN_MIC_LINK_EXE,
    TC_CHAIN_BUILD_SPIR,
    TC_CHAIN_CPU_BUILD_SPIR,
    TC_CHAIN_MIC_BUILD_SPIR,
    TC_NUM_CHAIN_TYPES,
};

struct STC_ChainInfo
{
    TC_CHAIN_TYPE  ChainType;
    TB_DATA_FORMAT InputFormat;
    TB_DATA_FORMAT OutputFormat;
};

/******************************************************************************\
STRUCTURE:   STC_TranslateArgs
DESCRIPTION: Structure used to store arguments for the ProcessTranslation 
             function.
\******************************************************************************/
struct STC_TranslateArgs
{
    TC_CHAIN_TYPE             ChainType;
    STB_TranslationCode       Code;
    char                     *pInput;
    cl_uint                   InputSize;
    const char               *Options;
    void                     *pTask;
};

} // namespace TC
