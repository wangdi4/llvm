#include "stdafx.h"
#include "ExtractCabData.h"
#include "res/PackageStructure.h"
#include "res/CustomDefinitions.h"
#include <fdi.h>
#include <io.h>
#include <share.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <direct.h>
#include <vector>
#include <set>

using namespace std;

DWORD g_dwThreadExtractCabDataID = 0;
HANDLE g_hThreadExtractCabData = NULL;
bool g_stop_extracting = false;
int g_stop_updating = 0;
int g_rem_temp_files = TRUE;
HFDI g_hFDI = NULL;

CProgressCtrl *g_pProgressCtrl = NULL;
CProgressCtrl *g_pProgrFileCtrl = NULL;
CStatic *g_pstExtrFile = NULL;
CDialog *g_pDialog = NULL;
CString g_BaseExtrFileStr;

WCHAR g_dest_base_dir[MAX_PATH+1] = {0};
char g_CabFileName[] = "data.cab";
char g_CabFilePath[] = "C:\\DATA_CAB_FROM_RESOURCE\\";
char g_CabFileFullPath[] = "C:\\DATA_CAB_FROM_RESOURCE\\data.cab";
DWORD g_CabDataSize = 0;
LPVOID g_lpvCabData = NULL;
vector<DWORD> g_CabDataOffsets;
vector<INT_PTR> g_CabDataHandles;
vector<INT_PTR> g_FilesHandles;
vector<UINT> g_FilesExtractedBytes;
vector<CString> g_FilesNames;
vector<UINT> g_FilesSizes;
UINT g_ExtractedBytes;

// data required for removing files and folder after sfx installation
vector<CString> g_CreatedFiles;
set<CString> g_CreatedFolders;


FNALLOC(fn_alloc)
{
  return malloc(cb);
}

FNFREE(fn_free)
{
  free(pv);
}

FNOPEN(fn_open)
{
  INT_PTR res = 0, sub_res = 0;
  int i, j;
  bool bln;

  if (strcmp(pszFile, g_CabFileFullPath) == 0)
  {
    bln = false;
    do
    {
      sub_res = rand();

      for (i=0; i<(int)g_FilesHandles.size(); i++)
      {
        if (sub_res == g_FilesHandles[i])
        {
          bln = true;
          break;
        }
      }

      for (i=0; i<(int)g_CabDataHandles.size(); i++)
      {
        if (sub_res == g_CabDataHandles[i])
        {
          bln = true;
          break;
        }
      }
    }
    while(bln || (sub_res == 0));

    res = sub_res;
    g_CabDataHandles.push_back(res);
    g_CabDataOffsets.push_back(0);
  }
  else
  {    
    _wsopen_s(&res, (WCHAR*)pszFile, oflag, _SH_DENYNO, pmode);

    bln = false;
    for (i=0; i<(int)g_CabDataHandles.size(); i++)
    {
      if (res == g_CabDataHandles[i])
      {
        bln = true;
        break;
      }
    }
    j = i;

    if (bln)
    {
      bln = false;
      do
      {
        sub_res = rand();

        for (i=0; i<(int)g_FilesHandles.size(); i++)
        {
          if (sub_res == g_FilesHandles[i])
          {
            bln = true;
            break;
          }
        }

        for (i=0; i<(int)g_CabDataHandles.size(); i++)
        {
          if (sub_res == g_CabDataHandles[i])
          {
            bln = true;
            break;
          }
        }
      }
      while(bln || (sub_res == 0) || (sub_res == res));

      g_CabDataHandles[j] = sub_res;
    }
  }

  return res;
}

FNREAD(fn_read)
{
  bool bln = false;
  int i;

  for (i=0; i<(int)g_CabDataHandles.size(); i++)
  {
    if (hf == g_CabDataHandles[i])
    {
      bln = true;
      break;
    }
  }

  if (bln)
  {
    if (g_lpvCabData == NULL)
    {
      return 0;
    }
    if (g_CabDataSize == 0)
    {
      return 0;
    }
    if (pv == NULL)
    {
      return 0;
    }
    if (g_CabDataOffsets[i] == g_CabDataSize)
    {
      return 0;
    }

    UINT cn = g_CabDataSize - g_CabDataOffsets[i];

    if (cn >= cb) cn = cb;

    memcpy(pv, (byte*)g_lpvCabData+g_CabDataOffsets[i], cn);
    g_CabDataOffsets[i] += cn;

    return cn;
  }
  else
  {
    return _read(hf, pv, cb);
  }
}

FNWRITE(fn_write)
{
  bool bln;
  int i, progr;

  bln = false;
  for (i=0; i<(int)g_CabDataHandles.size(); i++)
  {
    if (hf == g_CabDataHandles[i])
    {
      bln = true;
      break;
    }
  }

  if (bln)
  {
    return -1;
  }
  else
  {
    if (!g_stop_extracting)
    {
      g_stop_updating = 0;

      g_ExtractedBytes += cb;
      progr = (int)(((double)g_ExtractedBytes/(double)FILES_SIZE)*(double)1000);
      if (progr > 1000)
      {
        progr = 1000;
      }

      if (g_pProgressCtrl) g_pProgressCtrl->SetPos(progr);

      bln = false;
      for (i=0; i<(int)g_FilesHandles.size(); i++)
      {
        if (hf == g_FilesHandles[i])
        {
          bln = true;
          break;
        }
      }

      if (bln)
      {
        g_FilesExtractedBytes[i] = g_FilesExtractedBytes[i] + cb;
        CString ExtrFileStr = g_BaseExtrFileStr + g_FilesNames[i];
        CString CurExtrFileStr;

        progr = (int)(((double)g_FilesExtractedBytes[i]/(double)g_FilesSizes[i])*(double)1000);
        if (progr > 1000)
        {
          progr = 1000;
        }

        if (g_pstExtrFile)
        {
          g_pstExtrFile->GetWindowTextW(CurExtrFileStr);

          if (CurExtrFileStr != ExtrFileStr)
          {
            g_pstExtrFile->SetWindowTextW(ExtrFileStr);
			g_pDialog->RedrawWindow();
          }
        }
        if (g_pProgrFileCtrl) g_pProgrFileCtrl->SetPos(progr);
      }
      else
      {
        bln = bln;
      }

      g_stop_updating = 1;
    }

    return _write(hf, pv, cb);
  }
}

FNCLOSE(fn_close)
{
  bool bln = false;
  int i;
  int res;

  for (i=0; i<(int)g_CabDataHandles.size(); i++)
  {
    if (hf == g_CabDataHandles[i])
    {
      bln = true;
      break;
    }
  }

  if (bln)
  {
    g_CabDataOffsets[i] = 0;
    res = 0;
  }
  else
  {
    res = _close(hf);

    bln = false;
    for (i=0; i<(int)g_FilesHandles.size(); i++)
    {
      if (hf == g_FilesHandles[i])
      {
        bln = true;
        break;
      }
    }

    if (bln)
    {
      int size = (int)g_FilesHandles.size();

      g_FilesHandles[i] = g_FilesHandles[size-1];
      g_FilesHandles.pop_back();

      g_FilesExtractedBytes[i] = g_FilesExtractedBytes[size-1];
      g_FilesExtractedBytes.pop_back();

      g_FilesNames[i] = g_FilesNames[size-1];
      g_FilesNames.pop_back();

      g_FilesSizes[i] = g_FilesSizes[size-1];
      g_FilesSizes.pop_back();
    }
  }

  return res;
}

FNSEEK(fn_seek)
{
  bool bln = false;
  int i;
  INT_PTR res;

  for (i=0; i<(int)g_CabDataHandles.size(); i++)
  {
    if (hf == g_CabDataHandles[i])
    {
      bln = true;
      break;
    }
  }

  if (bln)
  {
    DWORD new_pos = 0;

    if (seektype == SEEK_SET) //Beginning of the file.
    {
      new_pos = dist;
    }
    else if (seektype == SEEK_CUR) //Current position of the file pointer.
    {
      new_pos = g_CabDataOffsets[i] + dist;
    }
    else if (seektype == SEEK_END) //End of file.
    {
      new_pos = g_CabDataSize - dist;
    }
    else
    {
      return -1;
    }

    if ((new_pos>=0) && (new_pos<=g_CabDataSize))
    {
      g_CabDataOffsets[i] = new_pos;
      res = g_CabDataOffsets[i];
    }
    else
    {
      return -1;
    }
  }
  else
  {
    res = _lseek(hf, dist, seektype);
  }

  return res;
}

FNFDINOTIFY(fn_notification)
{
  switch (fdint)
  {
    case fdintCABINET_INFO: // general information about the cabinet
      return 0;

    case fdintPARTIAL_FILE: // first file in cabinet is continuation
      return 0;

    case fdintCOPY_FILE:  // file to be copied
    {
      WCHAR dest[MAX_PATH+1];
      WCHAR sub_dir[MAX_PATH+1];
      WCHAR dir[MAX_PATH+1];
      WCHAR cab_file_name[MAX_PATH+1]={0};
      bool bln = false;
      int i = 0;
      WCHAR *pwcb, *pwce;
      size_t nconv;
      INT_PTR hf;

      if (g_stop_extracting)
      {
        return 0;
      }

      mbstowcs_s(&nconv, cab_file_name, MAX_PATH+1, pfdin->psz1, MAX_PATH);

      for (i=0; i<FILES_NUMBER; i++)
      {
        if (wcscmp(cab_file_name, ppPackageFilesMap[i][0]) == 0)
        {
          bln = true;
          break;
        }
      }

      if (!bln)
      {
        return -1;
      }

      swprintf(dest, MAX_PATH, L"%s%s", g_dest_base_dir, ppPackageFilesMap[i][1]);

	  g_CreatedFiles.push_back(CString(dest));
	  g_CreatedFolders.insert(CString(g_dest_base_dir));

      pwcb = ppPackageFilesMap[i][1];

      while ((pwce = wcschr(pwcb, L'\\')) != NULL)
      {
        memcpy(sub_dir, ppPackageFilesMap[i][1], (pwce-ppPackageFilesMap[i][1])*sizeof(WCHAR));
        sub_dir[pwce-ppPackageFilesMap[i][1]] = '\0';
        swprintf(dir, MAX_PATH, L"%s%s", g_dest_base_dir, sub_dir);
        _wmkdir(dir);
		g_CreatedFolders.insert(CString(dir));
        pwcb = pwce + 1;
      }

      hf = fn_open((LPCH)dest, _O_BINARY | _O_CREAT | _O_WRONLY | _O_SEQUENTIAL, _S_IREAD | _S_IWRITE);

      g_FilesHandles.push_back(hf);
      g_FilesExtractedBytes.push_back(0);	  
      g_FilesNames.push_back(CString(ppPackageFilesMap[i][1]));
      g_FilesSizes.push_back(pfdin->cb);

      return hf;
    }

    case fdintCLOSE_FILE_INFO:  // close the file, set relevant info
        {
      fn_close(pfdin->hf);

      return TRUE;
        }

    case fdintNEXT_CABINET: // file continued to next cabinet
      return 0;
  }

  return 0;
}

void ExtractCabData()
{
  ERF err;

  g_CabDataOffsets.clear();
  g_CabDataHandles.clear();
  g_FilesHandles.clear();
  g_FilesExtractedBytes.clear();
  g_FilesNames.clear();
  g_FilesSizes.clear();
  g_CreatedFiles.clear();
  g_CreatedFolders.clear();
  g_ExtractedBytes = 0;

  g_hFDI = FDICreate(fn_alloc, fn_free, fn_open, fn_read, fn_write, fn_close, fn_seek, cpuUNKNOWN, &err);
  FDICopy(g_hFDI, g_CabFileName, g_CabFilePath, 0, fn_notification, NULL, NULL);
}

void replace(WCHAR *str, WCHAR *repl, WCHAR *dest, int max_size)
{
	WCHAR *tmp = new WCHAR[max_size];
	WCHAR *pwc;

	while (pwc = wcsstr(str, repl))
	{
		wcscpy_s(tmp, max_size, pwc+wcslen(repl));
		swprintf(pwc, max_size-(pwc-str), L"%s%s", dest, tmp);
	}

	delete[] tmp;
}

struct WinProc
{
	vector<HWND> m_vhwnds;
	DWORD m_hp;
};

BOOL CALLBACK MyEnumChildWindowProc( HWND hwnd, LPARAM lParam )
{
	WinProc *pData = (WinProc*)lParam;
	DWORD hp;

	if( hwnd )
	{
		GetWindowThreadProcessId( hwnd, &hp );

		if (pData->m_hp == hp)
		{
			pData->m_vhwnds.push_back(hwnd);			
		}		

		return TRUE;
	}

	return FALSE;
};

BOOL CALLBACK MyEnumWindowProc( HWND hwnd, LPARAM lParam )
{
	WinProc *pData = (WinProc*)lParam;
	DWORD hp;

	if( hwnd )
	{
		GetWindowThreadProcessId( hwnd, &hp );

		if (pData->m_hp == hp)
		{
			pData->m_vhwnds.push_back(hwnd);			
		}

		EnumChildWindows( hwnd, MyEnumChildWindowProc, lParam );

		return TRUE;
	}

	return FALSE;
}

DWORD WINAPI ThreadExtractCabData(PVOID pParam)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	WCHAR cmd[2*MAX_PATH+1];	

	ExtractCabData();

	if  (!g_stop_extracting)
	{
		#if defined(OPEN_EXTRACTION_DIRECTORY)		

		if (g_rem_temp_files != TRUE)
		{
			ZeroMemory( &si, sizeof(si) );
			si.cb = sizeof(si);
			ZeroMemory( &pi, sizeof(pi) );

			swprintf(cmd, 2*MAX_PATH, L"explorer.exe \"%s\"", g_dest_base_dir);

			// Start the child process.
			CreateProcess(  NULL, // No module name (use command line)
			cmd,  // Command line
			NULL,   // Process handle not inheritable
			NULL,   // Thread handle not inheritable
			FALSE,  // Set handle inheritance to FALSE
			0,      // No creation flags
			NULL,   // Use parent's environment block
			NULL,   // Use parent's starting directory
			&si,    // Pointer to STARTUPINFO structure
			&pi  ); // Pointer to PROCESS_INFORMATION structure

			WaitForSingleObject( pi.hProcess, 10000 );		
		}

		#endif

		#if defined(EXECUTE_CMD)

		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );

		wcscpy_s(cmd, 2*MAX_PATH, EXECUTE_CMD);		
		replace(cmd, L"[%extraction_directory]", g_dest_base_dir, 2*MAX_PATH);

		// Start the child process.
		CreateProcess(  NULL, // No module name (use command line)
		cmd,  // Command line
		NULL,   // Process handle not inheritable
		NULL,   // Thread handle not inheritable
		FALSE,  // Set handle inheritance to FALSE
		0,      // No creation flags
		NULL,   // Use parent's environment block
		g_dest_base_dir,
		&si,    // Pointer to STARTUPINFO structure
		&pi  ); // Pointer to PROCESS_INFORMATION structure

		WinProc wp;
		clock_t beg_time;

		beg_time = clock();

		while (clock() < beg_time + CLOCKS_PER_SEC)
		{
			wp.m_vhwnds.clear();
			wp.m_hp = pi.dwProcessId;
			EnumWindows( MyEnumWindowProc, (LPARAM)&wp );

			if (wp.m_vhwnds.size() > 0)
			{
				SwitchToThisWindow(wp.m_vhwnds[0], TRUE);				
			}

			Sleep(100);
		}

		if (g_rem_temp_files == TRUE)
		{
			g_pDialog->ShowWindow(SW_HIDE);
			WaitForSingleObject( pi.hProcess, INFINITE );		
		}

		#endif		
	}

	if (g_rem_temp_files == TRUE)
	{
		for (int i=0; i<(int)g_CreatedFiles.size(); i++)
		{			
			DeleteFile(g_CreatedFiles[i]);
		}

		set <CString>::iterator pCreatedFoldersIter;

		pCreatedFoldersIter = g_CreatedFolders.end();
		do
		{
			pCreatedFoldersIter--;
			_wrmdir(*pCreatedFoldersIter);			
		}
		while(pCreatedFoldersIter != g_CreatedFolders.begin());		
	}

	exit(0);

	return 0;
}

void StartExtractCabData(CProgressCtrl *pProgrCtrl, CProgressCtrl *pProgrFileCtrl, CStatic *pstExtrFile, CDialog *pDialg)
{
  g_pProgressCtrl = pProgrCtrl;
  g_pProgrFileCtrl = pProgrFileCtrl;
  g_pstExtrFile = pstExtrFile;
  g_pDialog = pDialg;

  if (g_pProgressCtrl) g_pProgressCtrl->SetRange(0,1000);
  if (g_pProgrFileCtrl) g_pProgrFileCtrl->SetRange(0,1000);
  if (g_pstExtrFile) g_pstExtrFile->GetWindowTextW(g_BaseExtrFileStr);

  g_dwThreadExtractCabDataID = 0;
  g_hThreadExtractCabData = NULL;
  g_stop_extracting = false;
  g_stop_updating = 0;

  g_hThreadExtractCabData = CreateThread(NULL, 0, ThreadExtractCabData, NULL, 0, &g_dwThreadExtractCabDataID);
}

void PauseExtractCabData()
{
  if (g_hThreadExtractCabData)
  {
    SuspendThread(g_hThreadExtractCabData);
  }
}

void ContinueExtractCabData()
{
  if (g_hThreadExtractCabData)
  {
    ResumeThread(g_hThreadExtractCabData);
  }
}

void StopExtractCabData()
{
  if (g_hThreadExtractCabData)
  {
    DWORD res;
    int retr;

    retr = 0;
    while ((g_stop_updating == 0) && (retr < 10))
    {
      ResumeThread(g_hThreadExtractCabData);
      WaitForSingleObject(g_hThreadExtractCabData, 100);
      SuspendThread(g_hThreadExtractCabData);
      retr++;
    }

    g_stop_extracting = true;
    ResumeThread(g_hThreadExtractCabData);

    res = WaitForSingleObject(g_hThreadExtractCabData, 10000);
  }
}

ULONGLONG GetFilesSize()
{
  return (ULONGLONG)FILES_SIZE;
}
