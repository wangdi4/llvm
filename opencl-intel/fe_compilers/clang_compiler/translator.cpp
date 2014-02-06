/******************************************************************************\

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

File Name: TranslationChainLink.h

Abstract: 

Notes: 

\******************************************************************************/


#include "translator.h"

namespace TC
{
    PFNGETKERNELARGSINFOPLUGIN GetKernelArgsInfoPlugin;     // this global is not a member of CTranslator. It is here for convinence only
    PFNRELEASEKERNELARGSINFOPLUGIN ReleaseKernelArgsInfoPlugin;     // this global is not a member of CTranslator. It is here for convinence only

/******************************************************************************\

Destructor:
    CTranslator::Create

\******************************************************************************/
CTranslator* CTranslator::Create( const char* dllName )
{
    CTranslator* pTranslator = NULL;   
    pTranslator = new CTranslator( dllName );
    
    if( pTranslator )
    {
        if( pTranslator->Initialize() != TC_SUCCESS )
        {
            delete ( pTranslator );
            pTranslator = NULL;
        }
    }

    return pTranslator;
}

/******************************************************************************\

Destructor:
    CTranslator::Delete

\******************************************************************************/
void CTranslator::Delete( CTranslator* &pTranslator )
{
    if( pTranslator )
    {
        delete ( pTranslator );
        pTranslator = NULL;
    }
}

/******************************************************************************\

Constructor:
    CTranslator::CTranslator

\******************************************************************************/
CTranslator::CTranslator( const char* dllName )
{
    RegisterPlugin  = NULL;
    CreatePlugin    = NULL;
    DeletePlugin    = NULL;
    GetKernelArgsInfoPlugin = NULL;
    ReleaseKernelArgsInfoPlugin = NULL;

    m_name     = NULL;
    m_hDll     = (OS_HINSTANCE) OCLRT::OS_HMNULL; // NOTE: C-Style cast is intentional. On Linux we need static_cast<> here, on Windows - reinterpret_cast<>.
    m_isLoaded = false;

    if( dllName )
    {
        UINT size = (UINT)( strlen( dllName ) + 1 );
        m_name = new char[ size ];

        if( m_name )
        {
            strcpy_s( m_name, size, dllName );
        }
    }

    m_registerArgs.Version             = -1;
    m_registerArgs.pTranslationCodes   = NULL;
    m_registerArgs.NumTranslationCodes = 0;
}

/******************************************************************************\

Destructor:
    CTranslator::~CTranslator

\******************************************************************************/
CTranslator::~CTranslator()
{
    Unload();
 
    Intel::OpenCL::Utils::OclAutoMutex lock( &m_criticalSection );
    if( m_name )
    {
        delete[] m_name;
        m_name = NULL;
    }
}

/******************************************************************************\

Member Function:
    CTranslator::Initialize

\******************************************************************************/
TC_RETVAL CTranslator::Initialize()
{
    TC_RETVAL retVal = TC_ERROR;
    
    if( m_name )
    {
        retVal = Load();

#if TC_UNLOAD_AFTER_INITIALIZATION
        if( retVal == TC_SUCCESS )
        {
            retVal = Unload();
        }
#endif
    }

    return retVal;
}

/******************************************************************************\

Member Function:
    CTranslator::Load

\******************************************************************************/
TC_RETVAL CTranslator::Load()
{
    Intel::OpenCL::Utils::OclAutoMutex lock( &m_criticalSection );
    TC_RETVAL retVal = TC_ERROR;

    if( m_isLoaded == true )
    {
        // Compiler is already loaded
        retVal = TC_SUCCESS;
    }
    else
    {
        // load the DLL associated with the link name
        m_hDll = OSLoadLibrary( m_name );
        assert (m_hDll && "ERROR: failed to load fe dll" );

        cl_uint TranslationCode = 0;

        // get translation capabilities
        RegisterPlugin = (PFNREGISTERTRANSLATIONPLUGIN) OSGetProcAddress( m_hDll, "Register" );
        CreatePlugin   = (PFNCREATETRANSLATIONPLUGIN)OSGetProcAddress( m_hDll, "Create" );
        DeletePlugin   = (PFNDELETETRANSLATIONPLUGIN)OSGetProcAddress( m_hDll, "Delete" );
        GetKernelArgsInfoPlugin = (PFNGETKERNELARGSINFOPLUGIN)OSGetProcAddress( m_hDll, "GetKernelArgsInfo" );
        ReleaseKernelArgsInfoPlugin = (PFNRELEASEKERNELARGSINFOPLUGIN)OSGetProcAddress( m_hDll, "ReleaseKernelArgsInfo" );

        // ensure all functions exist
        assert( RegisterPlugin && "ERROR: Failed to register fe dll" );
        assert( CreatePlugin && "ERROR: Failed to register fe dll" );
        assert( DeletePlugin && "ERROR: Failed to register fe dll" );
        assert( GetKernelArgsInfoPlugin && "ERROR: Failed to register fe dll" );
        assert( ReleaseKernelArgsInfoPlugin && "ERROR: Failed to register fe dll" );

        // First call is made with pTranslationCodes = NULL so we get the number of codes we
        // need to allocate room for...
        RegisterPlugin( &m_registerArgs );

        // Ensure that we're using the same block version
        assert(( m_registerArgs.Version == STB_VERSION ) && "ERROR: fe dll is not the correct version" );

        // Allocate room for the translation block's codes
        m_registerArgs.pTranslationCodes = new TC::STB_TranslationCode[ m_registerArgs.NumTranslationCodes ];

        assert( m_registerArgs.pTranslationCodes && "ERROR: Failed to allocate room for translation code(s)" );

        // translation codes found - now go get the codes
        RegisterPlugin( &m_registerArgs );
        retVal = TC_SUCCESS;
    }

    if( retVal == TC_SUCCESS )
    {
        m_isLoaded = true;
    }

    return retVal;
}

/******************************************************************************\

Member Function:
    CTranslator::Unload

\******************************************************************************/
TC_RETVAL CTranslator::Unload()
{
    Intel::OpenCL::Utils::OclAutoMutex lock( &m_criticalSection );
    TC_RETVAL retVal = TC_SUCCESS;
    CTranslationBlock* pBlock = NULL;
    STB_TranslateOutputArgs* pArgs = NULL;

    // unload the DLL
    if( m_isLoaded && m_hDll )
    {
        while( !m_allocMap.empty( ) )
        {
            std::map<void*, void*>::iterator allocIter;
            allocIter = m_allocMap.begin( );

            pBlock = (CTranslationBlock*)allocIter->second;
            pArgs = (STB_TranslateOutputArgs*)allocIter->first;

            if( pBlock && pBlock->FreeAllocations( pArgs ) )
            {
                if( pArgs )
                {
                    delete pArgs;
                    pArgs = NULL;
                }
                m_allocMap.erase( allocIter );
                retVal = TC_SUCCESS;
            }
        }

        while( !m_blockMap.empty( ) )
        {
            std::map<UINT, void*>::iterator blockIter;
            blockIter = m_blockMap.begin( );
            m_blockMap.erase( blockIter );
        }

        if( OSFreeLibrary( m_hDll ) == TRUE )
        {
            // delete all associated codes
            if( m_registerArgs.pTranslationCodes )
            {
                delete[] m_registerArgs.pTranslationCodes;
                m_registerArgs.pTranslationCodes = NULL;
            }
            m_hDll = (OS_HINSTANCE) OCLRT::OS_HMNULL; // NOTE: C-Style cast is intentional. On Linux we need static_cast<> here, on Windows - reinterpret_cast<>.
            m_isLoaded = false;
        }
        else
        {
            assert( 0 );
            retVal = TC_ERROR;
        }
    }

    return retVal;
}

/******************************************************************************\

Member Function:
    CTranslator::ValidateCode

\******************************************************************************/
TC_RETVAL CTranslator::ValidateCode( STB_TranslationCode code )
{
    TC_RETVAL retVal = TC_ERROR;

    // ensure that one of the translation codes matches expectations
    if( m_registerArgs.pTranslationCodes )
    {
        for( UINT i = 0; i < m_registerArgs.NumTranslationCodes; i++ )
        {
            if( (DWORD)m_registerArgs.pTranslationCodes[i].Code ==
                (DWORD)code.Code )
            {
                retVal = TC_SUCCESS;
                break;
            }
        }
    }

    return retVal;
}

/******************************************************************************\

Member Function:
    CTranslator::GetBlock

\******************************************************************************/
CTranslationBlock* CTranslator::GetBlock( STB_TranslationCode code )
{
    Intel::OpenCL::Utils::OclAutoMutex lock( &m_criticalSection );
    TC_RETVAL retVal = TC_SUCCESS;
    CTranslationBlock* pBlock = NULL;
    std::map<UINT, void*>::iterator blockIter = m_blockMap.find( code.Code );

    if( blockIter != m_blockMap.end() )
    {
        pBlock = (CTranslationBlock*)blockIter->second;
    }

    // create the translation block if needed
    if( !pBlock )
    {
        // load the DLL if necessary
        if( !m_isLoaded )
        {
            retVal = Load();
        }

        if( ( retVal == TC_SUCCESS ) && CreatePlugin )
        {
            STB_CreateArgs createArgs;
            createArgs.TranslationCode = code;
            createArgs.deviceType = TB_DEVICE_CPU;
            
            pBlock = CreatePlugin( &createArgs );

            if( pBlock )
            {
                m_blockMap.insert( std::pair<UINT, void*>( code.Code, pBlock ) );
            }
        }
    }

    assert ( pBlock && "ERROR: Could not create block -- Translator" );

    return (CTranslationBlock*)pBlock;
}

/******************************************************************************\

Member Function:
    CTranslator::FreeAllocations

\******************************************************************************/
TC_RETVAL CTranslator::FreeAllocations( STB_TranslateOutputArgs* pOutputArgs )
{
    Intel::OpenCL::Utils::OclAutoMutex lock( &m_criticalSection );
    TC_RETVAL retVal = TC_ERROR;
    CTranslationBlock* pBlock = NULL;
    STB_TranslateOutputArgs* pArgs = NULL;

    std::map<void*, void*>::iterator Iter;
    Iter = m_allocMap.find( pOutputArgs );

    if( Iter != m_allocMap.end() )
    {
        pBlock = (CTranslationBlock*)Iter->second;
        pArgs = (STB_TranslateOutputArgs*)Iter->first;

        if( pBlock && pBlock->FreeAllocations( pArgs ) )
        {
            if( pArgs )
            {
                delete pArgs;
                pArgs = NULL;
            }
            m_allocMap.erase( Iter );
            pOutputArgs = NULL;
            retVal = TC_SUCCESS;
        }
    }

    return retVal;
}

/******************************************************************************\

Member Function:
    CTranslator::Translate

\******************************************************************************/
TC_RETVAL CTranslator::Translate( 
    STB_TranslateInputArgs* pInputArgs, 
    STB_TranslateOutputArgs* pOutputArgs, 
    STB_TranslationCode code )
{
    TC_RETVAL retVal = TC_ERROR;
    CTranslationBlock* pBlock = GetBlock( code );

    if( pBlock )
    {
        if( pBlock->Translate( pInputArgs, pOutputArgs) == true )
        {
            retVal = TC_SUCCESS;
        }

        Intel::OpenCL::Utils::OclAutoMutex lock( &m_criticalSection );
        m_allocMap.insert( std::pair<void*, void*>( pOutputArgs, pBlock ) );
    }

    return retVal;    
}

} // namespace TC
