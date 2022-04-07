// INTEL CONFIDENTIAL
//
// Copyright 2016-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

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
    return LoadLibraryEx( dllPath, NULL, 0 );
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

