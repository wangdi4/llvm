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

File Name: base_translation_controller.h

Abstract:

Notes:

\******************************************************************************/
#pragma once

#include "tc_common.h"

#define TC_SERIALIZE_BUILDS        ( 1 )  // 1 = disable re-entry to compiler DLLs
#define TC_SUFFIX_LENGTH           ( 32 )

namespace TC
{
// forward declarations
class CTranslationController;
class CTranslator;
class CTranslationChain;
struct SChainLink;

// global variables
extern CTranslationController* g_pTranslationController;

/******************************************************************************\
Class:
    CTranslationController

Description:
    The translation controller is responsible for:
    + initializing all plug-ins
    + interfacing with the client
    + validating translation data

\******************************************************************************/
class CTranslationController
{
public:
    // static member functions
    static unsigned int OSAPI ProcessTranslation( void* pArgs );
    static CTranslationController* Create();
    static void Delete( CTranslationController* &pController );

    // public member functions
    int Translate( STC_TranslateArgs* pTranslateArgs );

private:
    CTranslationController();
    ~CTranslationController();

    TC_RETVAL Initialize();

    TC_RETVAL InitializeTranslation(
        STC_TranslateArgs* pTranslateArgs,
        UINT& startIdx,
        UINT& stopIdx );

    TC_RETVAL GetStartIndex( TC_CHAIN_TYPE chainType, TB_DATA_FORMAT inputType, UINT& startIdx );
    TC_RETVAL GetStopIndex( TC_CHAIN_TYPE chainType, TB_DATA_FORMAT outputType, UINT& stopIdx );

    //Copying and Assignment is NOT allowed.
    CTranslationController( const CTranslationController &a );
    CTranslationController operator = ( CTranslationController a );

    bool  m_isInitialized;

    // Translators
    CTranslator* m_pFrontEnd;

    // Compiler links
    SChainLink* m_pElf2LLBin;

    // Compiler chains
    CTranslationChain* m_pChains[TC_NUM_CHAIN_TYPES];
};
} // namespace TC
