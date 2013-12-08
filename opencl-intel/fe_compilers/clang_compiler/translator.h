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

File Name: translator.h

Abstract: 

Notes: 

\******************************************************************************/
#pragma once

#include "tc_common.h"
#include "cl_synch_objects.h"
#include <map>

namespace TC
{
/******************************************************************************\

Translation Block Plugin Function Pointer: 
    PFNREGISTERTRANSLATIONPLUGIN
    PFNCREATETRANSLATIONPLUGIN
    PFNDELETETRANSLATIONPLUGIN
    PFNGETKENELINFOPLUGIN

Description:
    Imported translation block plugin function pointers
        + Register - provides the formats supported by the plugin
        + Create - creates a CTranslationBlock class
        + Delete - delete the CTranslationBlock class and any internally defined 
                   resources
		+ GetKernelArgsInfo - return the kernel info structures


\******************************************************************************/
typedef void (__cdecl  *PFNREGISTERTRANSLATIONPLUGIN )( STB_RegisterArgs* pRegisterArgs );
typedef CTranslationBlock* (__cdecl  *PFNCREATETRANSLATIONPLUGIN )( STB_CreateArgs* pCreateArgs );
typedef void (__cdecl  *PFNDELETETRANSLATIONPLUGIN )( CTranslationBlock* pBlock );
typedef void (__cdecl  *PFNGETKERNELARGSINFOPLUGIN )( const void *pBin, const char *szKernelName, STB_GetKernelArgsInfoArgs* pKernelArgsInfo );
typedef void (__cdecl  *PFNRELEASEKERNELARGSINFOPLUGIN )( STB_GetKernelArgsInfoArgs* pKernelArgsInfo );

extern PFNGETKERNELARGSINFOPLUGIN GetKernelArgsInfoPlugin;     // this global is not a member of CTranslator. It is here for convinence only
extern PFNRELEASEKERNELARGSINFOPLUGIN ReleaseKernelArgsInfoPlugin;     // this global is not a member of CTranslator. It is here for convinence only

/******************************************************************************\

Class: CTranslator

Description:

\******************************************************************************/
class CTranslator
{
public:
    static CTranslator* Create( const char* linkName );
    static void Delete( CTranslator* &pLink );

    CTranslationBlock* GetBlock( STB_TranslationCode code );
    const char* GetName();

    bool IsLoaded();
    bool IsNamed( const char* linkName );

    TC_RETVAL Translate( 
        STB_TranslateInputArgs* pInputArgs, 
        STB_TranslateOutputArgs* pOutputArgs, 
        STB_TranslationCode code );

    TC_RETVAL FreeAllocations( STB_TranslateOutputArgs* pOutputArgs );
    TC_RETVAL Unload();

private:
    //Copying and Assignment is NOT allowed.
    CTranslator( const CTranslator &a );
    CTranslator operator = ( CTranslator a );

protected:
    CTranslator( const char* linkName );
    ~CTranslator();

    TC_RETVAL Initialize();
    TC_RETVAL Load();
    TC_RETVAL ValidateCode( STB_TranslationCode );

    PFNREGISTERTRANSLATIONPLUGIN RegisterPlugin;
    PFNCREATETRANSLATIONPLUGIN   CreatePlugin;
    PFNDELETETRANSLATIONPLUGIN   DeletePlugin;

    char*                    m_name;
    OS_HINSTANCE             m_hDll;
    STB_RegisterArgs         m_registerArgs;
    bool                     m_isLoaded;
    std::map<UINT, void*>    m_blockMap;
    std::map<void*, void*>   m_allocMap;
    Intel::OpenCL::Utils::OclMutex m_criticalSection;
};

__inline const char* CTranslator::GetName()
{
    return m_name;
}

__inline bool CTranslator::IsLoaded()
{
    return m_isLoaded;
}

__inline bool CTranslator::IsNamed( const char* linkName )
{
    return (bool)!strcmp( linkName, m_name );
}

} // namespace TC
