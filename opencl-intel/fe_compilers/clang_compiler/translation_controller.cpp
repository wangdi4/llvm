/*****************************************************************************\

Copyright 2000 - 2012 Intel Corporation All Rights Reserved.

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

File Name: translation_controller.cpp

Abstract: 

Notes: 

\*****************************************************************************/
#include "ElfReader.h"
#include "os_inc.h"
#include "translation_controller.h"
#include "translator.h"
#include "translation_chain.h"
#include "clang_driver.h"

namespace TC
{

HANDLE g_hThreadLimit;  // thread limiting semaphore
OCLRT::OS_CRITICAL_SECTION g_translationCriticalSection;

/******************************************************************************\

Global Function:
    ProcessTranslation

Description:
    Allows us to process a build request in a separate thread

\******************************************************************************/
unsigned int OSAPI CTranslationController::ProcessTranslation( void* pArgs )
{
#if TC_SERIALIZE_BUILDS
    OSEnterCriticalSection( &g_translationCriticalSection );
#endif

    DWORD retVal = CL_SUCCESS;
    STC_TranslateArgs* pTranslateArgs = (STC_TranslateArgs*)pArgs;

    if( pTranslateArgs )
    {
        assert( g_pTranslationController && "ERROR: Global translation controller does not exist!\n" );

        retVal = g_pTranslationController->Translate( pTranslateArgs );
    }

#if TC_SERIALIZE_BUILDS
    OSLeaveCriticalSection( &g_translationCriticalSection );
#endif

   
    return retVal;
}

/******************************************************************************\

Member Function:
    CTranslationController::Create

Output:
    Returns a pointer to a new, initialized CTranslationController class

\******************************************************************************/
CTranslationController* CTranslationController::Create()
{
    TC_RETVAL retVal = TC_ERROR;
    CTranslationController* pController = NULL;
    
    pController = new CTranslationController();

    if( pController )
    {
        retVal = pController->Initialize();

        if( retVal != TC_SUCCESS )
        {
            CTranslationController::Delete( pController );
            pController = NULL;
        }
    }

    return pController;
}

/******************************************************************************\

Member Function:
    CTranslationController::Delete

Output:
    Deletes an instance of the CTranslationController class

\******************************************************************************/
void CTranslationController::Delete( CTranslationController* &pController )
{
    if( pController )
    {
        delete ( pController );
        pController = NULL;
    }
}

/******************************************************************************\

Constructor: 
    CTranslationController

\******************************************************************************/
CTranslationController::CTranslationController()
{
    m_isInitialized = false;

    for( UINT i = 0; i < TC::TC_NUM_CHAIN_TYPES; i++ )
    {
        m_pChains[i] = NULL;
    }

    m_pFrontEnd = NULL;
    m_pBackend = NULL;
    m_pCLTxt2LLTxt = NULL;
    m_pElf2LLBin = NULL;
    m_pElf2LLLib = NULL;
}

/******************************************************************************\

Destructor: 
    ~CTranslationController

\******************************************************************************/
CTranslationController::~CTranslationController()
{
    OSEnterCriticalSection( &g_translationCriticalSection );
    CTranslator::Delete( m_pFrontEnd );
    CTranslator::Delete( m_pBackend );

    for( UINT i = 0; i < TC::TC_NUM_CHAIN_TYPES; i++ )
    {
        CTranslationChain::Delete( m_pChains[i] );
    }

    if( m_pCLTxt2LLTxt )
    {
        delete ( m_pCLTxt2LLTxt );
        m_pCLTxt2LLTxt = NULL;
    }
    if( m_pElf2LLBin )
    {
        delete ( m_pElf2LLBin );
        m_pElf2LLBin = NULL;
    }
    if( m_pElf2LLLib )
    {
        delete ( m_pElf2LLLib );
        m_pElf2LLLib = NULL;
    }

    OSLeaveCriticalSection( &g_translationCriticalSection );
    OSDeleteCriticalSection( &g_translationCriticalSection );
}

/******************************************************************************\

Member Function:
    CTranslationController::Initialize

Description:
    Returns a pointer to a new, initialized CTranslationController class

\******************************************************************************/
TC_RETVAL CTranslationController::Initialize()
{
    TC_RETVAL retVal = TC_SUCCESS;
    STB_TranslationCode code = { TB_DATA_FORMAT_UNKNOWN, TB_DATA_FORMAT_UNKNOWN };

    if( !m_isInitialized )
    {
        OSInitializeCriticalSection( &g_translationCriticalSection );
        OSEnterCriticalSection( &g_translationCriticalSection );
        m_isInitialized = true;

        // create array of links for GenX
        m_pFrontEnd = CTranslator::Create( name_libfcl );
        m_pBackend  = NULL;                                        //CTranslator::Create( name_libbcl );

        // fail if any of the translators were not created correctly
        if( m_pFrontEnd /*&& m_pBackend*/ )
        {
            //***************************************
            // create the chain links
            //***************************************
            // CL text to LLVM text
            //***************************************
            m_pCLTxt2LLTxt = new SChainLink;
            if( m_pCLTxt2LLTxt && ( retVal == TC_SUCCESS ) )
            {
                m_pCLTxt2LLTxt->Code.InputType  = TB_DATA_FORMAT_OCL_TEXT;
                m_pCLTxt2LLTxt->Code.OutputType = TB_DATA_FORMAT_LLVM_TEXT;
                m_pCLTxt2LLTxt->pTranslator     = m_pFrontEnd;
                retVal = TC_SUCCESS;
            }

            //***************************************
            // ELF to LLVM binary
            //***************************************
            m_pElf2LLBin = new SChainLink;
            if( m_pElf2LLBin && ( retVal == TC_SUCCESS ) )
            {
                m_pElf2LLBin->Code.InputType  = TB_DATA_FORMAT_ELF;
                m_pElf2LLBin->Code.OutputType = TB_DATA_FORMAT_LLVM_BINARY;
                m_pElf2LLBin->pTranslator     = m_pFrontEnd;
            }

            //***************************************
            // ELF to LLVM archive
            //***************************************
            m_pElf2LLLib = new SChainLink;
            if( m_pElf2LLLib && ( retVal == TC_SUCCESS ) )
            {
                m_pElf2LLLib->Code.InputType  = TB_DATA_FORMAT_ELF;
                m_pElf2LLLib->Code.OutputType = TB_DATA_FORMAT_LLVM_ARCHIVE;
                m_pElf2LLLib->pTranslator     = m_pFrontEnd;
            }

           //***************************************
            // Create the chains
            //***************************************
            // CPU Build Chain
            //***************************************
            if( ( m_pChains[TC_CHAIN_CPU_BUILD] = CTranslationChain::Create() ) == NULL )
            {
                retVal = TC_ERROR;
            }

            if( retVal == TC_SUCCESS )
            {
                retVal = m_pChains[TC_CHAIN_CPU_BUILD]->Push( m_pCLTxt2LLTxt );
            }

            //***************************************
            // MIC Build Chain
            //***************************************
            if( ( m_pChains[TC_CHAIN_MIC_BUILD] = CTranslationChain::Create() ) == NULL )
            {
                retVal = TC_ERROR;
            }

            if( retVal == TC_SUCCESS )
            {
                retVal = m_pChains[TC_CHAIN_MIC_BUILD]->Push( m_pCLTxt2LLTxt );
            }

            //***************************************
            // Compile Chain
            //***************************************
            if( ( m_pChains[TC_CHAIN_COMPILE] = CTranslationChain::Create() ) == NULL )
            {
                retVal = TC_ERROR;
            }

            if( retVal == TC_SUCCESS )
            {
                retVal = m_pChains[TC_CHAIN_COMPILE]->Push( m_pElf2LLBin );
            }

            //***************************************
            // Link Library Chain
            //***************************************
            if( ( m_pChains[TC_CHAIN_LINK_LIB] = CTranslationChain::Create() ) == NULL )
            {
                retVal = TC_ERROR;
            }

            if( retVal == TC_SUCCESS )
            {
                retVal = m_pChains[TC_CHAIN_LINK_LIB]->Push( m_pElf2LLLib );
            }

            //***************************************
            // CPU Link Executable Chain
            //***************************************
            if( ( m_pChains[TC_CHAIN_CPU_LINK_EXE] = CTranslationChain::Create() ) == NULL )
            {
                retVal = TC_ERROR;
            }

            if( retVal == TC_SUCCESS )
            {
                retVal = m_pChains[TC_CHAIN_CPU_LINK_EXE]->Push( m_pElf2LLBin );
            }

            //***************************************
            // MIC Link Executable Chain
            //***************************************
            if( ( m_pChains[TC_CHAIN_MIC_LINK_EXE] = CTranslationChain::Create() ) == NULL )
            {
                retVal = TC_ERROR;
            }

            if( retVal == TC_SUCCESS )
            {
                retVal = m_pChains[TC_CHAIN_MIC_LINK_EXE]->Push( m_pElf2LLBin );
            }

            //***************************************
            // CPU SPIR Build Chain
            //***************************************
            if( ( m_pChains[TC_CHAIN_CPU_BUILD_SPIR] = CTranslationChain::Create() ) == NULL )
            {
                retVal = TC_ERROR;
            }

            //***************************************
            // MIC SPIR Build Chain
            //***************************************
            if( ( m_pChains[TC_CHAIN_MIC_BUILD_SPIR] = CTranslationChain::Create() ) == NULL )
            {
                retVal = TC_ERROR;
            }
        }
        else
        {
            retVal = TC_ERROR;
        }

        OSLeaveCriticalSection( &g_translationCriticalSection );
    }

    return retVal;
}

/******************************************************************************\

Member Function:
    CTranslationController::GetStartIndex

\******************************************************************************/
TC_RETVAL CTranslationController::GetStartIndex( 
    TC_CHAIN_TYPE chainType, 
    TB_DATA_FORMAT inputType, 
    UINT& startIdx  )
{
    TC_RETVAL retVal = TC_ERROR;
    startIdx = (UINT)TC_INVALID_INDEX;

    if( m_pChains && m_pChains[chainType] )
    {
        for( UINT i = 0; i < m_pChains[chainType]->GetNumLinks(); i++ )
        {
            SChainLink* pLink = m_pChains[chainType]->GetLink( i );
            if( pLink && ( pLink->Code.InputType == inputType ) )
            {
                startIdx = i;
                retVal = TC_SUCCESS;
            }
        }
    }

    return retVal;
}

/******************************************************************************\

Member Function:
    CTranslationController::GetStopIndex

\******************************************************************************/
TC_RETVAL CTranslationController::GetStopIndex( 
    TC_CHAIN_TYPE chainType, 
    TB_DATA_FORMAT outputType, 
    UINT& stopIdx )
{
    TC_RETVAL retVal = TC_ERROR;
    stopIdx = (UINT)TC_INVALID_INDEX;

    if( m_pChains && m_pChains[chainType] )
    {
        for( UINT i = 0; i < m_pChains[chainType]->GetNumLinks(); i++ )
        {
            SChainLink* pLink = m_pChains[chainType]->GetLink( i );
            if( pLink && ( pLink->Code.OutputType == outputType ) )
            {
                stopIdx = i;
                retVal = TC_SUCCESS;
            }
        }
    }

    return retVal;
}

/******************************************************************************\

Member Function:
    CTranslationController::InitializeTranslation

Description:
    Does error checking across a number of parameters and sets the start and
    stop indices for the translation.

\******************************************************************************/
TC_RETVAL CTranslationController::InitializeTranslation( 
    STC_TranslateArgs* pTranslateArgs, 
    UINT& startIdx, 
    UINT& stopIdx )
{
    TC_RETVAL retVal = TC_ERROR;
    CTranslationChain* pTranslationChain = NULL;

    if( pTranslateArgs && pTranslateArgs->pInput && pTranslateArgs->InputSize )
    {
        pTranslationChain = m_pChains[pTranslateArgs->ChainType];

        if( pTranslationChain )
        {
            retVal = GetStartIndex( 
                pTranslateArgs->ChainType,
                pTranslateArgs->Code.InputType,
                startIdx );

            if( retVal == CL_SUCCESS )
            {
                retVal = GetStopIndex( 
                    pTranslateArgs->ChainType,
                    pTranslateArgs->Code.OutputType,
                    stopIdx );
            }
        }
    }

    return retVal;
}

/******************************************************************************\

Member Function:
    CTranslationController::Translate

Description:
    Function called to request translation.  Translate determines whether the
    requested translation is supported.  If supported a request to process
    the translation is made and TC_SUCCESS is returned.  If not supported,
    the translation is not processed and TC_ERROR is returned.

Input:
    STranslateArgs* pArgs - translation arguments

Output:
    TC_ERROR_CODE - informs whether translation was processed or not

\******************************************************************************/
int CTranslationController::Translate( STC_TranslateArgs* pTranslateArgs )
{
    int retVal = CL_SUCCESS;
    cl_build_status buildStatus = CL_BUILD_ERROR;
    CTranslator* pTranslator = NULL;
    CTranslator* pPrevTranslator = NULL;
    CTranslationChain* pTranslationChain = NULL;
    SChainLink* pChainLink = NULL;
    STB_TranslateInputArgs inputArgs;
    STB_TranslateInputArgs* pInputArgs = &inputArgs;
    STB_TranslateOutputArgs* pOutputArgs = NULL;
    STB_TranslateOutputArgs* pPrevOutputArgs = NULL;
    UINT startIdx = TC_INVALID_INDEX;
    UINT stopIdx = TC_INVALID_INDEX;
    size_t strLength = 0;
    char* pTmpOptions = NULL;
//    const char* pKernelArgInfoOption = " -cl-kernel-arg-info"; // Note the space in the front

    // validate translation arguments
    retVal = InitializeTranslation( pTranslateArgs, startIdx, stopIdx );

    // set chain parameters
    if( retVal == CL_SUCCESS )
    {
        // get the translation chain
        pTranslationChain = m_pChains[pTranslateArgs->ChainType];

        // set the initial inputs
        pInputArgs->pInput = pTranslateArgs->pInput;
        pInputArgs->InputSize = pTranslateArgs->InputSize;

        assert( pTranslateArgs->Options && "Oprions bufferr should be allocation (even as an 'empty' string)" );

        // append -cl-kernel-arg-info, if necessary
        if( pTranslateArgs->Options )
        {
            strLength = strlen( pTranslateArgs->Options ) + 1;

//            // if the string does not contain the -cl-kernel-arg-info option...
//            if( strstr( pTranslateArgs->Options, "-cl-kernel-arg-info" ) == NULL )
//            {
//                // ...then add it to the existing options...
//                strLength += strlen( pKernelArgInfoOption );
//
//                pTmpOptions = new char[ strLength ];
//                if( pTmpOptions )
//                {
//                    sprintf_s( pTmpOptions, strLength, "%s%s", pTranslateArgs->Options, pKernelArgInfoOption );
//
//                    pInputArgs->pOptions = pTmpOptions;
//                    pInputArgs->OptionsSize = (DWORD)strLength;
//                }
//                else
//                {
//                    retVal = CL_OUT_OF_HOST_MEMORY;
//                }
//            }
//            // ...if the options already include -cl-get-kernel-arg-info...
//            else
            {
                // no new allocation is needed - we can just assign to the passed in translation args.
                pInputArgs->pOptions = pTranslateArgs->Options;
                pInputArgs->OptionsSize = (DWORD)strLength;
            }
        }
    }

    // for each link in the chain...
    for( UINT  i = startIdx; i <= stopIdx; i++ )
    {
        if( retVal == CL_SUCCESS )
        {
            // store the previous translator and previous output
            pPrevTranslator = pTranslator;
            pPrevOutputArgs = pOutputArgs;
            pOutputArgs = new STB_TranslateOutputArgs();

            if( !pOutputArgs )
            {
                retVal = CL_OUT_OF_HOST_MEMORY;
            }

            if( retVal == CL_SUCCESS )
            {
                // get the link/translator
                pChainLink = pTranslationChain->GetLink( i );

                if( pChainLink && pChainLink->pTranslator )
                {
                    pTranslator = pChainLink->pTranslator;
                }
                else
                {
                    retVal = CL_BUILD_PROGRAM_FAILURE;
                }
            }

            if( retVal == CL_SUCCESS )
            {
                // call translator->translate
                if( pTranslator->Translate( pInputArgs, pOutputArgs, pChainLink->Code ) != TC_SUCCESS )
                {
                    retVal = CL_BUILD_PROGRAM_FAILURE;
                }

                // free up previous allocations
                if( ( pPrevOutputArgs ) && 
                    ( pPrevOutputArgs->pOutput || pPrevOutputArgs->pErrorString ) &&
                    ( pPrevTranslator->FreeAllocations( pPrevOutputArgs ) != TC_SUCCESS ) )
                {
                    assert( 0 && "WARNING: failed to free allocations" );
                }
            }

            if( retVal == CL_SUCCESS )
            {
                switch( pChainLink->Code.OutputType )
                {
                case TB_DATA_FORMAT_LLVM_ARCHIVE:
                case TB_DATA_FORMAT_LLVM_BINARY:
                case TB_DATA_FORMAT_LLVM_TEXT:
                    {
                        // store the LLVM data in the program object
                        switch (pTranslateArgs->ChainType)
                        {
                        case TC_CHAIN_COMPILE:
                            {
                                Intel::OpenCL::ClangFE::ClangFECompilerCompileTask* pCompileTask = (Intel::OpenCL::ClangFE::ClangFECompilerCompileTask *)pTranslateArgs->pTask;
                                retVal = pCompileTask->StoreOutput( pOutputArgs, pChainLink->Code.OutputType ); 
                            }
                            break;
                        case TC_CHAIN_CPU_LINK_EXE:
                        case TC_CHAIN_LINK_LIB:
                            {
                                Intel::OpenCL::ClangFE::ClangFECompilerLinkTask* pLinkTask = (Intel::OpenCL::ClangFE::ClangFECompilerLinkTask *)pTranslateArgs->pTask;
                                retVal = pLinkTask->StoreOutput( pOutputArgs, pChainLink->Code.OutputType ); 
                            }
                            break;
                        default:
                            assert( 0 && "Invalid chain type");
                        }

#if TC_UNLOAD_FRONTEND_PER_USE
                        // make the new input arguments = to stored LLVM binary
                        if( retVal == CL_SUCCESS )
                        {
                            pProgObj->GetLLVMBinary( pInputArgs->pInput, pInputArgs->InputSize );
                            if( !pInputArgs->pInput || !pInputArgs->InputSize )
                            {
                                retVal = CL_BUILD_PROGRAM_FAILURE;
                            }
                        }

                        if( m_pFrontEnd && ( i != stopIdx ) )
                        {
                            pOutputArgs = NULL;

                            // unload the DLLs if needed
                            // Right now, it appears that only the front end needs to be unloaded
                            m_pFrontEnd->Unload();
                        }
#else
                        // new input is our current output
                        pInputArgs->pInput = pOutputArgs->pOutput;
                        pInputArgs->InputSize = pOutputArgs->OutputSize;
#endif
                    }
                    break;

                default:
                    retVal = CL_BUILD_PROGRAM_FAILURE;
                }
            }
            else
            {
                // store the LLVM data in the program object
                switch ( pTranslateArgs->ChainType )
                {
                case TC_CHAIN_COMPILE:
                    {
                        Intel::OpenCL::ClangFE::ClangFECompilerCompileTask* pCompileTask = (Intel::OpenCL::ClangFE::ClangFECompilerCompileTask *)pTranslateArgs->pTask;
                        pCompileTask->ClearOutput( pOutputArgs ); 
                    }
                    break;
                
                case TC_CHAIN_CPU_LINK_EXE:
                    {
                        Intel::OpenCL::ClangFE::ClangFECompilerLinkTask* pLinkTask = (Intel::OpenCL::ClangFE::ClangFECompilerLinkTask *)pTranslateArgs->pTask;
                        pLinkTask->ClearOutput( pOutputArgs ); 
                    }
                    break;
                default:
                    assert( 0 && "Invalid chain type");
                }
                break;
            }
        }
    }

    if( retVal == CL_SUCCESS )
    {
        buildStatus = CL_BUILD_SUCCESS;
    }
    else
    {
        buildStatus = CL_BUILD_ERROR;
    }

    // free up the pInputArgs->pOptions
    if( pTmpOptions )
    {
        delete[] pTmpOptions;
        pTmpOptions = NULL;
    }

    // free up any outstanding allocations
    if( ( pOutputArgs ) && ( pTranslator ) &&
        ( pOutputArgs->pOutput || pOutputArgs->pErrorString ) &&
        ( pTranslator->FreeAllocations( pOutputArgs ) != TC_SUCCESS ) )
    {
        assert( 0 && "WARNING: failed to free allocations" );
    }

#if TC_UNLOAD_FRONTEND_PER_USE
    if( m_pFrontEnd )
    {
        // unload the DLLs if needed
        // Right now, it appears that only the front end needs to be unloaded
        m_pFrontEnd->Unload();
    }
#endif

    return retVal;
}

/******************************************************************************\

Member Function:
    CTranslationController::DumpOutputData

\******************************************************************************/
void CTranslationController::DumpProgramData( 
    char* pBuffer, 
    UINT  bufferSize,
    char* pFileRoot )
{
    OSEnterCriticalSection( &g_translationCriticalSection );
  
    FILE* pFile = NULL;
    UINT  dumpType = 0;
    char  pFileName[MAX_PATH];
    char  pModuleName[MAX_PATH];
    char  pCoreName[TC_SUFFIX_LENGTH];
    char  pSuffix[TC_SUFFIX_LENGTH];
    char  pRevIDString[TC_SUFFIX_LENGTH];
    char* pModuleCurPos = pModuleName;
    char* pFileCurPos   = pFileName;
    bool  printElfText  = false;

    static UINT cl_idx  = 0;
    static UINT elf_idx = 0;
    static UINT ll_idx  = 0;
    static UINT g3d_idx = 0;
    static UINT bin_idx = 0;

    strcat_s( pCoreName, TC_SUFFIX_LENGTH, "_" );
    strcat_s( pCoreName, TC_SUFFIX_LENGTH, pRevIDString );

    if( pBuffer && bufferSize )
    {
        // figure out what kind of file we're trying to print
        if( *(DWORD*)pBuffer == 0x464c457f /* 'ELF' */ )
        {
            // dumpType ELF;
            printElfText = true;
            sprintf_s( pSuffix, TC_SUFFIX_LENGTH, "%s_%d.elf\0", pCoreName, elf_idx++ );
        }
        else if( ( strstr( (char*)pBuffer, "IGHAL-pc-win32" ) != NULL ) ||
                 ( strstr( (char*)pBuffer, "target triple = \"GHAL3D" ) != NULL ) ||
                 ( strstr( (char*)pBuffer, "target triple = \"IGIL" ) != NULL ) )
        {
            // dumpType LLVM;
            sprintf_s( pSuffix, TC_SUFFIX_LENGTH, "%s_%d.ll\0", pCoreName, ll_idx++ );
        }
        else if( ( strstr( (char*)pBuffer, "VERSION_2_" ) != NULL ) ||
                 ( strstr( (char*)pBuffer, "VERSION_3_" ) != NULL ) )
        {
            // dumpType GHAL3D;
            sprintf_s( pSuffix, TC_SUFFIX_LENGTH, "%s_%d.ghal3d\0", pCoreName, g3d_idx++ );
        }
        else if( *(DWORD*)pBuffer == 0x494E5443  /*'I', 'N', 'T', 'C'*/ )
        {
            // dumpType GEN;
            sprintf_s( pSuffix, TC_SUFFIX_LENGTH, "%s_%d.bin\0", pCoreName, bin_idx++ );

        }
        else
        {
            // dumpType OCL_TEXT;
            sprintf_s( pSuffix, TC_SUFFIX_LENGTH, "%s_%d.cl\0", pCoreName, cl_idx++ );
        }
    }

    OSLeaveCriticalSection( &g_translationCriticalSection );
    return;
}

/******************************************************************************\

Member Function:
    CTranslationController::PrintELF

\******************************************************************************/
void CTranslationController::PrintELF( char* pElfBinary, size_t elfBinarySize, char* pFilename )
{
    FILE* pFile = NULL;
    fopen_s( &pFile, pFilename, "wb" );

    if( pElfBinary && pFile )
    {
        char strHeaderType[MAX_PATH];
        char strSectionType[MAX_PATH];
        CLElfLib::CElfReader* pElfReader = CLElfLib::CElfReader::Create( pElfBinary, elfBinarySize );

        if( pElfReader )
        {
            const CLElfLib::SElf64Header* pElfReaderHeader = pElfReader->GetElfHeader();

            if( pElfReaderHeader )
            {
                UINT numSections = pElfReaderHeader->NumSectionHeaderEntries;
                UINT type = pElfReaderHeader->Type;
                memset( strHeaderType, 0x00, MAX_PATH );

                switch( type )
                {
                case CLElfLib::EH_TYPE_OPENCL_SOURCE:
                    strcpy_s( strHeaderType, MAX_PATH-1, "OpenCL Source [CL Source/Headers]\0" );
                    break;

                case CLElfLib::EH_TYPE_OPENCL_OBJECTS:
                    strcpy_s( strHeaderType, MAX_PATH-1, "OpenCL Objects [LLVM binaries/archives]\0" );
                    break;

                case CLElfLib::EH_TYPE_OPENCL_LIBRARY:
                    strcpy_s( strHeaderType, MAX_PATH-1, "OpenCL Library\0" );
                    break;

                case CLElfLib::EH_TYPE_OPENCL_EXECUTABLE:
                    strcpy_s( strHeaderType, MAX_PATH-1, "OpenCL Executable\0" );
                    break;
                default:
                    strcpy_s( strHeaderType, MAX_PATH-1, "Unexpected ELF Type\0" );
                    break;
                }

                fprintf_s( pFile, "*** TEXT REPRESENTATION OF ELF BINARY ***\n" );
                fprintf_s( pFile, "ELF Header Type: %s\n", strHeaderType );
                fprintf_s( pFile, "Number of sections: %d\n", numSections );

                for( UINT i = 0; i < numSections; i++ )
                {
                    type = pElfReader->GetSectionHeader( i )->Type;
                    switch( type )
                    {
                    case CLElfLib::SH_TYPE_OPENCL_SOURCE:
                        strcpy_s( strSectionType, MAX_PATH-1, "OpenCL Text - Source\0" );
                        break;

                    case CLElfLib::SH_TYPE_OPENCL_HEADER:
                        strcpy_s( strSectionType, MAX_PATH-1, "OpenCL Text - Header\0" );
                        break;

                    case CLElfLib::SH_TYPE_OPENCL_LLVM_ARCHIVE:
                        strcpy_s( strSectionType, MAX_PATH-1, "LLVM Archive\0" );
                        break;

                    case CLElfLib::SH_TYPE_OPENCL_LLVM_BINARY:
                        strcpy_s( strSectionType, MAX_PATH-1, "LLVM Binary\0" );
                        break;

                    case CLElfLib::SH_TYPE_OPENCL_DEV_BINARY:
                        strcpy_s( strSectionType, MAX_PATH-1, "Gen Device Binary\0" );
                        break;

                    case CLElfLib::SH_TYPE_STR_TBL:
                        strcpy_s( strSectionType, MAX_PATH-1, "String Table\0" );
                        break;

                    case CLElfLib::SH_TYPE_NULL:
                        strcpy_s( strSectionType, MAX_PATH-1, "Empty\0" );
                        break;
                    }

                    fprintf_s( pFile, "\n/---------------------------------------\n" );
                    fprintf_s( pFile, "Section Name: %s\n", pElfReader->GetSectionName( i ) );
                    fprintf_s( pFile, "Section Type: %s\n", strSectionType );
                    fprintf_s( pFile, "Section Data:\n" );

                    char* pSectionData = NULL;
                    size_t sectionSize = 0;

                    pElfReader->GetSectionData( i, pSectionData, sectionSize );
                    size_t myCount = fwrite( pSectionData, 1, sectionSize, pFile );
                    fprintf_s( pFile, "\n\\---------------------------------------\n" );
                }
            }
        }

        fclose( pFile );
    }
}

} // namespace TC
