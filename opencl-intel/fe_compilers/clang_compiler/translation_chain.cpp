/*****************************************************************************\

Copyright 2010 Intel Corporation All Rights Reserved.

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

File Name: translation_chain.cpp

Abstract: 

Notes: 

\*****************************************************************************/
#include "translation_chain.h"

namespace TC
{
CTranslationChain::CTranslationChain()
{
    OSInitializeCriticalSection( &m_criticalSection );
    m_numLinks = 0;
}

CTranslationChain::~CTranslationChain()
{
    OSEnterCriticalSection( &m_criticalSection );

    for( unsigned int i = 0; i < m_numLinks; i++ )
    {
        // The actual link allocations are being cleared in the translation controller
        // as that is where they are created...

        // clear the map entry
        m_linkMap.erase( i );
    }

    m_numLinks = 0;
    OSLeaveCriticalSection( &m_criticalSection );
    OSDeleteCriticalSection( &m_criticalSection );
}

CTranslationChain* CTranslationChain::Create()
{
    CTranslationChain* pChain = NULL;
    pChain = new CTranslationChain();
    return pChain;
}

void CTranslationChain::Delete( CTranslationChain* pChain )
{
    if( pChain )
    {
        delete ( pChain );
        pChain = NULL;
    }
}

TC_RETVAL CTranslationChain::Push( SChainLink* pLink )
{
    OSEnterCriticalSection( &m_criticalSection );
    
    TC_RETVAL retVal = TC_SUCCESS;
    if( pLink )
    {
        std::pair<UINT, void*> newElement;
        newElement.first = m_numLinks++;
        newElement.second = pLink;
        m_linkMap.insert( newElement );
    }

    OSLeaveCriticalSection( &m_criticalSection );
    return retVal;
}

SChainLink* CTranslationChain::GetLink( UINT index )
{
    OSEnterCriticalSection( &m_criticalSection );
    SChainLink* pLink = NULL;

    if( index < m_numLinks )
    {
        std::map<UINT,void*>::iterator iter = m_linkMap.find( index );
        pLink = (SChainLink*)iter->second;
    }

    OSLeaveCriticalSection( &m_criticalSection );
    return pLink;
}

UINT CTranslationChain::GetNumLinks()
{
    return m_numLinks;
}
} // namespace TC
