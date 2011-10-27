#pragma once
#include "stdafx.h"

extern WCHAR g_dest_base_dir[MAX_PATH+1];
extern WCHAR BaseExtractionDirectory[];
extern DWORD g_CabDataSize;
extern LPVOID g_lpvCabData;
extern int g_rem_temp_files;
void StartExtractCabData(CProgressCtrl *pProgrCtrl, CProgressCtrl *pProgrFileCtrl, CStatic *pstExtrFile, CDialog *pDialg);
void PauseExtractCabData();
void ContinueExtractCabData();
void StopExtractCabData();
ULONGLONG GetFilesSize();

