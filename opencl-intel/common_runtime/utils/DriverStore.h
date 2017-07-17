/*****************************************************************************\

INTEL CONFIDENTIAL
Copyright 2016
Intel Corporation All Rights Reserved.

The source code contained or described herein and all documents related to the
source code ("Material") are owned by Intel Corporation or its suppliers or
licensors. Title to the Material remains with Intel Corporation or its suppliers
and licensors. The Material contains trade secrets and proprietary and confidential
information of Intel or its suppliers and licensors. The Material is protected by
worldwide copyright and trade secret laws and treaty provisions. No part of the
Material may be used, copied, reproduced, modified, published, uploaded, posted
transmitted, distributed, or disclosed in any way without Intel’s prior express
written permission.

No license under any patent, copyright, trade secret or other intellectual
property right is granted to or conferred upon you by disclosure or delivery
of the Materials, either expressly, by implication, inducement, estoppel
or otherwise. Any license under such intellectual property rights must be
express and approved by Intel in writing.

File Name:  DriverStore.h

Abstract:

Notes:

\*****************************************************************************/
#pragma once

/* RedStone NTx86.10.0...14310 
The fuctionality is temporary disabled.
To eneble change this definition to
#define OS_WIN_RS 10014310
*/
#define OS_WIN_RS 99999999

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
__inline HINSTANCE GetModuleHINSTANCE() { return ( HINSTANCE )&__ImageBase; }

__inline char *getModulePath( char *dllPath, const char *FileName )
{
    /* GetModuleFileName retrieves the fully qualified path for the file that contains the specified module
     e.g. C:\Windows\System32\DriverStore\FileRepository\igdlh64.inf_amd64_808e12707a8c1658\igdrcl64.dll 
     We go back in the loop until we find a slash
     C:\Windows\System32\DriverStore\FileRepository\igdlh64.inf_amd64_808e12707a8c1658\
     Then add the name of the parameter passed */
    DWORD Length = GetModuleFileName( GetModuleHINSTANCE(), dllPath, MAX_PATH );
    for( DWORD l = Length; l > 0; l-- )
    {
        if( dllPath[ l - 1 ] == '\\' )
        {
            dllPath[ l ] = '\0';
            break;
        }
    }

    strcat_s( dllPath, MAX_PATH, FileName );

    return dllPath;
}

__inline HMODULE
LoadDependency(const char *dependencyFileName)
{
    char dllPath[MAX_PATH];
    getModulePath( dllPath, dependencyFileName );
    return LoadLibraryEx( dllPath, nullptr, 0 );
}

/******************************************************************************\
Function:    GetWinVer
Description: Determines the version of windows os
Returns:     UINSIGNED INT 99 9 99999 ( MajorVersion x 1000 000 MinorVersion x 100 000 OSBuildNumber)
\******************************************************************************/
__inline unsigned int GetWinVer( void )
{
    unsigned int Result = 0;
    LONG( WINAPI *pfnRtlGetVersion )( RTL_OSVERSIONINFOW* );
    ( FARPROC& )pfnRtlGetVersion = GetProcAddress( GetModuleHandleW( ( L"ntdll.dll" ) ), "RtlGetVersion" );
    if( pfnRtlGetVersion )
    {
        RTL_OSVERSIONINFOW RTLOSVersionInfo ={ 0 };
        RTLOSVersionInfo.dwOSVersionInfoSize = sizeof( RTLOSVersionInfo );
        // Get the OS version
        if( pfnRtlGetVersion( &RTLOSVersionInfo ) == 0 )
        {
            Result =  RTLOSVersionInfo.dwBuildNumber;
            Result += (RTLOSVersionInfo.dwMinorVersion * 100000);
            Result += (RTLOSVersionInfo.dwMajorVersion * 1000000);
        }
    }
    return Result;
}

