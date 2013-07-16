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

File Name: translation_chain.h

Abstract: 

Notes: 

\******************************************************************************/
#pragma once

#include "tc_common.h"
#include "translator.h"
#include "os_inc.h"

namespace TC
{
/******************************************************************************\

Structure: 
    SChainLink

\******************************************************************************/
struct SChainLink
{
    CTranslator*           pTranslator;
    STB_TranslationCode    Code;
};

/******************************************************************************\
Class:
    CTranslationChain

Description:

\******************************************************************************/
class CTranslationChain
{
public:
    static CTranslationChain* Create();
    static void Delete( CTranslationChain* pChain );

    TC_RETVAL Push( SChainLink* pLink );
    CTranslator* Pop();

    SChainLink* GetLink( UINT index );
    UINT GetNumLinks();

private:
    CTranslationChain();
    ~CTranslationChain();

    unsigned int m_numLinks;
    std::map<UINT, void*> m_linkMap;

    OCLRT::OS_CRITICAL_SECTION m_criticalSection;
};
} // namespace TC
