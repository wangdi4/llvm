#include "stdafx.h"
#include "SelfExtractor.h"
#include "SelfExtractorDlg.h"
#include "ExtractCabData.h"
#include "res/CustomDefinitions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSelfExtractorDlg::CSelfExtractorDlg(CWnd* pParent /*=NULL*/)
  : CDialog(CSelfExtractorDlg::IDD, pParent)
  , m_EditPath(_T(""))
{
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSelfExtractorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PATH, m_EditPath);
	DDX_Control(pDX, IDC_EDIT_PATH, m_cEditPath);
	DDX_Control(pDX, IDC_WELCOM_CAPTION, m_WelcomCaption);
	DDX_Control(pDX, IDC_WELCOM_DESCRIPTION, m_WelcomDescr);
	DDX_Control(pDX, IDC_WELCOM_REM_TEMP_FILES, m_WelcomRemTempFiles);	
	DDX_Control(pDX, IDC_BITMAP, m_Bitmap);
	DDX_Control(pDX, IDC_ST_DEST_FOLDER, m_DestDirGroupBox);
	DDX_Control(pDX, IDC_CHECK_REM_TEMP_FILES, m_CheckRemTempFiles);
}

BEGIN_MESSAGE_MAP(CSelfExtractorDlg, CDialog)
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  //}}AFX_MSG_MAP
  ON_BN_CLICKED(IDEXTRACT, &CSelfExtractorDlg::OnBnClickedExtract)
  ON_BN_CLICKED(IDC_SELECT_DIR, &CSelfExtractorDlg::OnBnClickedSelectDir)
  ON_BN_CLICKED(IDCANCEL, &CSelfExtractorDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

BOOL CSelfExtractorDlg::OnInitDialog()
{
  CDialog::OnInitDialog();

  BOOL res;
  CRect wrc;
  STARTUPINFO StartupInfo;

  m_pExtrDialog = NULL;

  //m_wWidth = 502;
  //m_wHeight = 390;
  //m_offset = 164;

  m_BaseColor = RGB(BACKGROUND_COLOR_R,BACKGROUND_COLOR_G,BACKGROUND_COLOR_B);

  //m_WelcomDescr.Set

  res = m_fntWC.CreateFont(
  16,                        // nHeight
  0,                         // nWidth
  0,                         // nEscapement
  0,                         // nOrientation
  FW_BOLD,           // nWeight
  FALSE,                     // bItalic
  FALSE,                     // bUnderline
  0,                         // cStrikeOut
  ANSI_CHARSET,              // nCharSet
  OUT_DEFAULT_PRECIS,        // nOutPrecision
  CLIP_DEFAULT_PRECIS,       // nClipPrecision
  DEFAULT_QUALITY,           // nQuality
  DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
  _T("Arial"));             // lpszFacename
  //_T("Microsoft Sans Serif"));

  m_WelcomCaption.SetFont(&m_fntWC, 0);

  SetIcon(m_hIcon, TRUE);
  SetIcon(m_hIcon, FALSE);

  GetWindowRect(&wrc);

  m_offset = (int)((double)wrc.Height()*(double)0.45);

  //wrc.right = wrc.left+m_wWidth-1;
  //wrc.bottom = wrc.top+m_wHeight-1;
  //MoveWindow(&wrc, 0);

  GetStartupInfo(&StartupInfo);

  wcscpy_s(g_dest_base_dir, MAX_PATH+1, StartupInfo.lpTitle);
  if (wcsrchr(g_dest_base_dir, L'\\')) *(wcsrchr(g_dest_base_dir, L'\\')) = L'\0';
  if (g_dest_base_dir[wcslen(g_dest_base_dir)-1] != L'\\') wcscat_s(g_dest_base_dir, MAX_PATH+1, L"\\");
  wcscat_s(g_dest_base_dir, MAX_PATH+1, BaseExtractionDirectory);
  wcscat_s(g_dest_base_dir, MAX_PATH+1, L"\\");

  m_cEditPath.SetWindowTextW(g_dest_base_dir);
  m_cEditPath.SetSel(-1, 0);

  m_CheckRemTempFiles.SetCheck(TRUE);  

  return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSelfExtractorDlg::OnPaint()
{
  if (IsIconic())
  {
    CPaintDC dc(this); // device context for painting

    SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

    // Center icon in client rectangle
    int cxIcon = GetSystemMetrics(SM_CXICON);
    int cyIcon = GetSystemMetrics(SM_CYICON);
    CRect rect;
    GetClientRect(&rect);
    int x = (rect.Width() - cxIcon + 1) / 2;
    int y = (rect.Height() - cyIcon + 1) / 2;

    // Draw the icon
    dc.DrawIcon(x, y, m_hIcon);
  }
  else
  {
    CPaintDC dc(this); // device context for painting
    CRect rect, clrc, wcrc, wdrc, bmprc, ddgbrc, wrtfrc;
    CBrush Brush;
    CFont* def_font;
    CString WCStr, WDStr, WRTFStr;
    CPen p1(PS_SOLID, 1, RGB(172,168,153));
    CPen p2(PS_SOLID, 1, RGB(255,255,255));
    CPen* def_pen;

    dc.SetBkColor(m_BaseColor);
	dc.SetBkMode(TRANSPARENT);

    Brush.CreateSolidBrush(m_BaseColor);
    GetClientRect(&clrc);

	m_Bitmap.GetWindowRect(&bmprc);
	ScreenToClient(&bmprc);
	m_Bitmap.ShowWindow(SW_HIDE);

    m_WelcomCaption.GetWindowRect(&wcrc);
    ScreenToClient(&wcrc);
    m_WelcomCaption.GetWindowTextW(WCStr);
    m_WelcomCaption.ShowWindow(SW_HIDE);

    m_WelcomDescr.GetWindowRect(&wdrc);
    ScreenToClient(&wdrc);
    m_WelcomDescr.GetWindowTextW(WDStr);
    m_WelcomDescr.ShowWindow(SW_HIDE);	

	m_WelcomRemTempFiles.GetWindowRect(&wrtfrc);
    ScreenToClient(&wrtfrc);
    m_WelcomRemTempFiles.GetWindowTextW(WRTFStr);
    m_WelcomRemTempFiles.ShowWindow(SW_HIDE);		

	m_DestDirGroupBox.GetWindowRect(&ddgbrc);
    ScreenToClient(&ddgbrc);

	HBITMAP hbitmap = m_Bitmap.GetBitmap();

	HDC dcBmp = CreateCompatibleDC(dc.m_hDC);
	HGDIOBJ tmpObj = SelectObject(dcBmp,hbitmap);
	BitBlt(dc.m_hDC,0,0,bmprc.Width(),bmprc.Height(),dcBmp,0,0,SRCCOPY);
	SelectObject(dc.m_hDC, tmpObj);
	DeleteDC(dcBmp);

    //rect = clrc;
    //rect.bottom = clrc.bottom - m_offset;
    //dc.FillRect(rect, &Brush);

    def_pen = dc.SelectObject(&p1);
    dc.MoveTo(0, clrc.bottom - m_offset+1);
    dc.LineTo(clrc.right, clrc.bottom - m_offset+1);
    dc.SelectObject(&p2);
    dc.MoveTo(0, clrc.bottom - m_offset+2);
    dc.LineTo(clrc.right, clrc.bottom - m_offset+2);

    def_font = dc.SelectObject(&m_fntWC);
    dc.DrawTextW(WCStr, &wcrc, DT_LEFT | DT_WORDBREAK);	
		
    dc.SelectObject(m_WelcomDescr.GetFont());

    dc.DrawTextW(WDStr, &wdrc, DT_LEFT | DT_WORDBREAK);	

	dc.DrawTextW(WRTFStr, &wrtfrc, DT_LEFT | DT_WORDBREAK);	

	ddgbrc.top -= 15;
	ddgbrc.bottom = ddgbrc.top + 20;
	ddgbrc.left += 10;

	def_font = dc.SelectObject(&m_fntWC);
	dc.DrawTextW(CString(L"Destination Folder"), &ddgbrc, DT_LEFT | DT_WORDBREAK);	

	m_cEditPath.SetSel(-1, 0);

    dc.SelectObject(def_font);
    dc.SelectObject(def_pen);

    CDialog::OnPaint();
  }
}

HCURSOR CSelfExtractorDlg::OnQueryDragIcon()
{
  return static_cast<HCURSOR>(m_hIcon);
}

void CSelfExtractorDlg::OnBnClickedExtract()
{
  ULARGE_INTEGER FreeBytesAvailable, TotalNumberOfBytes, TotalNumberOfFreeBytes;
  HINSTANCE hModule;
  HRSRC hRsrc;
  HGLOBAL hCabDataRsrc;
  WCHAR sub_dir[MAX_PATH+1];
  WCHAR msg[MAX_PATH*2+1];
  int req_mb, avail_mb;
  WCHAR *pwcb, *pwce;
  DWORD dw_res;
  int i_res;

  srand(GetTickCount());

  g_CabDataSize = 0;
  g_lpvCabData = NULL;

  m_cEditPath.GetWindowTextW(g_dest_base_dir, MAX_PATH);

  wcscpy_s(sub_dir, MAX_PATH+1, g_dest_base_dir);
  dw_res = GetFullPathName(sub_dir, MAX_PATH+1, g_dest_base_dir, NULL);

  if (dw_res == 0)
  {
    i_res = MessageBox(SFX_MAIN_DIALOG_EXTR_PRESS_FAIL_DESCR, SFX_MAIN_DIALOG_EXTR_PRESS_FAIL_CAPTION, MB_ICONERROR);
    return;
  }

  if (g_dest_base_dir[wcslen(g_dest_base_dir)-1] != L'\\') wcscat_s(g_dest_base_dir, MAX_PATH+1, L"\\");

  if ((wcslen(g_dest_base_dir) > 2) && (g_dest_base_dir[1] == L':'))
  {
    pwcb = g_dest_base_dir;

    while ((pwce = wcschr(pwcb, L'\\')) != NULL)
    {
      memcpy(sub_dir, g_dest_base_dir, (pwce-g_dest_base_dir)*sizeof(WCHAR));
      sub_dir[pwce-g_dest_base_dir] = L'\0';

      if (sub_dir[pwce-g_dest_base_dir-1] != L':')
      {
        _wmkdir(sub_dir);
      }

      pwcb = pwce + 1;
    }
  }
  else if ((wcslen(g_dest_base_dir) > 2) && (g_dest_base_dir[0] == L'\\') && (g_dest_base_dir[1] == L'\\') && (g_dest_base_dir[2] != L'\\'))
  {
    pwcb = g_dest_base_dir+2;
    if ((pwce = wcschr(pwcb, L'\\')) != NULL)
    {
      pwcb = pwce + 1;

      if ((pwce = wcschr(pwcb, L'\\')) != NULL)
      {
        pwcb = pwce + 1;

        while ((pwce = wcschr(pwcb, L'\\')) != NULL)
        {
          memcpy(sub_dir, g_dest_base_dir, (pwce-g_dest_base_dir)*sizeof(WCHAR));
          sub_dir[pwce-g_dest_base_dir] = L'\0';
          _wmkdir(sub_dir);
          pwcb = pwce + 1;
        }
      }
    }
  }

  _wmkdir(g_dest_base_dir);
  i_res = _wmkdir(g_dest_base_dir);

  if (i_res != 0)
  {
    _get_errno(&i_res);

    if (i_res != EEXIST)
    {
      int res = MessageBox(SFX_MAIN_DIALOG_INCORRECT_EXTR_DIR_DESCR, SFX_MAIN_DIALOG_INCORRECT_EXTR_DIR_CAPTION, MB_ICONERROR);
      return;
    }
  }

  GetDiskFreeSpaceEx(g_dest_base_dir, &FreeBytesAvailable, &TotalNumberOfBytes, &TotalNumberOfFreeBytes);

  if (GetFilesSize() > FreeBytesAvailable.QuadPart)
  {
    req_mb = (int)(GetFilesSize()/(ULONGLONG)0x100000);
    avail_mb = (int)(FreeBytesAvailable.QuadPart/(ULONGLONG)0x100000);

    swprintf_s(msg, MAX_PATH*2, L"There is no enough disc space. Required %dmb. Available %dmb.", req_mb, avail_mb);
    int res = MessageBox(msg, SFX_MAIN_DIALOG_NO_DISC_SPACE_CAPTION, MB_ICONERROR);
    return;
  }

  hModule = AfxGetInstanceHandle();
  hRsrc = FindResource(hModule, MAKEINTRESOURCE(IDR_CAB_DATA),_T("Binary"));
  hCabDataRsrc = LoadResource(hModule, hRsrc);
  g_CabDataSize = SizeofResource(hModule, hRsrc);
  g_lpvCabData = LockResource(hCabDataRsrc);

  g_rem_temp_files = m_CheckRemTempFiles.GetCheck();

  m_pExtrDialog = new ExtractionProgressDialog(this);
  m_pExtrDialog->m_pParent = this;

  if (m_pExtrDialog) m_pExtrDialog->DoModal();

  if (m_pExtrDialog) delete m_pExtrDialog;
}

void CSelfExtractorDlg::OnBnClickedSelectDir()
{
  BROWSEINFO  bi;
  TCHAR   szPath[MAX_PATH];

  memset( &bi, 0, sizeof(bi) );
  bi.hwndOwner = this->m_hWnd;
  bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_NEWDIALOGSTYLE;

  LPITEMIDLIST pidl = SHBrowseForFolder( &bi );

  if( pidl != NULL )
  {
    szPath[0] = L'\0';

    if( SHGetPathFromIDList( pidl, szPath ) != FALSE )
    {
      wcscpy_s(g_dest_base_dir, MAX_PATH+1, szPath);
      if (g_dest_base_dir[wcslen(g_dest_base_dir)-1] != L'\\') wcscat_s(g_dest_base_dir, MAX_PATH+1, L"\\");
    }
  }

  m_cEditPath.SetWindowTextW(g_dest_base_dir);
}

void CSelfExtractorDlg::OnBnClickedCancel()
{
  int res = MessageBox(SFX_MAIN_DIALOG_CANCEL_DESCR, SFX_MAIN_DIALOG_CANCEL_CAPTION, MB_YESNO);

  if (res == IDYES)
  {
    OnCancel();
  }
}
