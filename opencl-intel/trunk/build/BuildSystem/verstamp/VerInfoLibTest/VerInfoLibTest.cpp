// VerInfoLibTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "VerInfoLibTest.h"
#include "..\VerInfoLib\All.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;

void ShowUsage()
{
  _tprintf(_T("\verstamp.exe -update <FileName> <Major Version> <Minor Version> <Build Number>\n"));
}

void ShowSystemFilesVersions()
{
  WIN32_FIND_DATA FindFileData;
  HANDLE hFind;

  TCHAR szBuf[MAX_PATH];
  ::GetSystemDirectory(szBuf, MAX_PATH);
  CString strSystemFolder = szBuf;

  hFind = FindFirstFile(strSystemFolder + "\\*.dll", &FindFileData);

  if (hFind == INVALID_HANDLE_VALUE)
  {
    printf ("Invalid File Handle. Get Last Error reports %d\n", GetLastError ());
  }
  else
  {
    do
    {
      CString strModulePath = strSystemFolder + "\\" + FindFileData.cFileName;

      const CVersionInfo vi(strModulePath);

      if (vi.IsValid())
      {
        _tprintf(_T("%s\t%s\n"), FindFileData.cFileName, vi["FileVersion"]);
      }

    }
    while (FindNextFile(hFind, &FindFileData));

    FindClose(hFind);
  }
}

void TestVersionInfoLib(int argc, TCHAR* argv[], TCHAR* envp[])
{
  if (2 == argc)
  {
    if (!_tcscmp(argv[1], _T("-system")))
    {
      ShowSystemFilesVersions();
    }
    else
      ShowUsage();
  }
  else if (3 == argc)
  {
    if (!_tcscmp(argv[1], _T("-f")))
    {
      // 2nd argument is the file path
      CString strFilePath(argv[2]);

      const CVersionInfo vi(strFilePath);
      if (vi.IsValid())
      {
        POSITION posTable = vi.GetStringFileInfo().GetFirstStringTablePosition();
        while (posTable)
        {
          const CStringTable &st = *vi.GetStringFileInfo().GetNextStringTable(posTable);

          _tprintf(_T("String table %s\n------------------------------\n"), st.GetKey());

          POSITION posString = st.GetFirstStringPosition();

          while (posString)
          {
            const CVersionInfoString &vistr = *st.GetNextString(posString);
            _tprintf(_T("%s=%s\n"), vistr.GetKey(), vistr.GetValue());
          }
          _tprintf(_T("------------------------------\n"));
        }
      }
      else
      {
        _tprintf(_T("Failed to get module version information for %s\n"), strFilePath);
      }
    }
    else
      ShowUsage();

  }
  else if (argc > 4)
  {
    if (!_tcscmp(argv[1], _T("-u")))
    {
      // 2nd argument is the file path
      CString strFilePath(argv[2]);
      CVersionInfo vi(strFilePath);

      vi[argv[3]] = argv[4];
      vi.Save();
    }
    else
      ShowUsage();

  }
  else
    ShowUsage();


}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
  int nRetCode = 0;

  // initialize MFC and print and error on failure
  if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
  {
    // TODO: change error code to suit your needs
    cerr << _T("Fatal Error: MFC initialization failed") << endl;
    nRetCode = 1;
  }
  else
  {
    if ((6 == argc) && (0 == _tcscmp(argv[1], _T("-update"))))
    {
      // 2nd argument is the file path
      CString strFilePath(argv[2]);
      CVersionInfo vi(strFilePath);

      if( !vi.IsValid())
      {
        cerr << _T("Error: can't open file") << argv[2] << endl;
        return -1;
      }

      vi["ProductName"] = "Intel(R) OpenCL SDK 1.5";
      vi["LegalCopyright"] = "© 2007 - 2011, Intel Corporation.  All rights reserved.";

      int major = atoi( argv[3] );
      int minor = atoi( argv[4] );
      int build = atoi( argv[5] );

      WORD hi = HIWORD( build );
      WORD lo = LOWORD( build );

      vi.SetFileVersion( major, minor, hi, lo, TRUE, ", " );
      vi.SetProductVersion( major, minor, hi, lo, TRUE, ", " );

      vi["Comments"] = "OpenCL";

      char buffer[128];
      sprintf_s( buffer, 128, "%d", build );
      vi["PrivateBuild"] = buffer;

      vi.Save();
    }
    else
    {
      TestVersionInfoLib(argc, argv, envp);
    }
  }
  return nRetCode;
}
