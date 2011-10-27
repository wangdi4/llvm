// ExtractionProgressDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SelfExtractor.h"
#include "ExtractionProgressDialog.h"
#include "ExtractCabData.h"
#include "res/CustomDefinitions.h"

IMPLEMENT_DYNAMIC(ExtractionProgressDialog, CDialog)

ExtractionProgressDialog::ExtractionProgressDialog(CWnd* pParent /*=NULL*/)
  : CDialog(ExtractionProgressDialog::IDD, pParent)
{
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

ExtractionProgressDialog::~ExtractionProgressDialog()
{
}

void ExtractionProgressDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_ProgressCtrl);
	DDX_Control(pDX, IDC_EXTR_PROGRESS, m_stExtrProgress);
	DDX_Control(pDX, IDC_EXTR_CAPTION, m_ExtrCap);
	DDX_Control(pDX, IDC_EXTR_DESCR, m_ExtrDescr);
	DDX_Control(pDX, IDC_PROGRESS2, m_ProgressFileCtrl);
	DDX_Control(pDX, IDC_EXTR_FILE, m_stExtrFile);
	DDX_Control(pDX, IDC_BITMAP, m_Bitmap);
	DDX_Control(pDX, IDC_GROUP_BOX, m_DestDirGroupBox);
}


BEGIN_MESSAGE_MAP(ExtractionProgressDialog, CDialog)
  ON_NOTIFY(NM_CUSTOMDRAW, IDC_PROGRESS1, &ExtractionProgressDialog::OnNMCustomdrawProgress1)
  ON_WM_PAINT()
  ON_BN_CLICKED(IDCANCEL, &ExtractionProgressDialog::OnBnClickedCancel)
END_MESSAGE_MAP()


// ExtractionProgressDialog message handlers

void ExtractionProgressDialog::OnNMCustomdrawProgress1(NMHDR *pNMHDR, LRESULT *pResult)
{
  LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
  // TODO: Add your control notification handler code here
  *pResult = 0;
}

BOOL ExtractionProgressDialog::OnInitDialog()
{
  CDialog::OnInitDialog();

  SetIcon(m_hIcon, TRUE);
  SetIcon(m_hIcon, FALSE);

  RECT wrc;
  m_pParent->GetWindowRect(&wrc);
  this->MoveWindow(&wrc, 0);

  StartExtractCabData(&m_ProgressCtrl, &m_ProgressFileCtrl, &m_stExtrFile, this);

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}


void ExtractionProgressDialog::OnPaint()
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

    if (m_pParent)
    {
		m_pParent->ShowWindow(SW_HIDE);

		CRect rect, clrc, ecrc, edrc, bmprc, ddgbrc, efrc, eprc;
		CBrush Brush;
		CFont* def_font;
		CPen p1(PS_SOLID, 1, RGB(172,168,153));
		CPen p2(PS_SOLID, 1, RGB(255,255,255));
		CPen* def_pen;
		CString ECStr, EDStr, EFStr, EPStr;

		dc.SetBkColor(m_pParent->m_BaseColor);
		dc.SetBkMode(TRANSPARENT);

		Brush.CreateSolidBrush(m_pParent->m_BaseColor);
		GetClientRect(&clrc);

		m_Bitmap.GetWindowRect(&bmprc);
		ScreenToClient(&bmprc);
		m_Bitmap.ShowWindow(SW_HIDE);

		m_ExtrCap.GetWindowRect(&ecrc);
		ScreenToClient(&ecrc);
		m_ExtrCap.GetWindowTextW(ECStr);
		m_ExtrCap.ShowWindow(SW_HIDE);

		m_ExtrDescr.GetWindowRect(&edrc);
		ScreenToClient(&edrc);
		m_ExtrDescr.GetWindowTextW(EDStr);
		m_ExtrDescr.ShowWindow(SW_HIDE);

		m_stExtrFile.GetWindowRect(&efrc);
		ScreenToClient(&efrc);
		m_stExtrFile.GetWindowTextW(EFStr);
		m_stExtrFile.ShowWindow(SW_HIDE);

		m_stExtrProgress.GetWindowRect(&eprc);
		ScreenToClient(&eprc);
		m_stExtrProgress.GetWindowTextW(EPStr);
		m_stExtrProgress.ShowWindow(SW_HIDE);

		m_DestDirGroupBox.GetWindowRect(&ddgbrc);
		ScreenToClient(&ddgbrc);

		HBITMAP hbitmap = m_Bitmap.GetBitmap();

		HDC dcBmp = CreateCompatibleDC(dc.m_hDC);
		HGDIOBJ tmpObj = SelectObject(dcBmp,hbitmap);
		BitBlt(dc.m_hDC,0,0,bmprc.Width(),bmprc.Height(),dcBmp,0,0,SRCCOPY);
		SelectObject(dc.m_hDC, tmpObj);
		DeleteDC(dcBmp);

		//rect = clrc;
		//rect.bottom = clrc.bottom - m_pParent->m_offset;
		//dc.FillRect(rect, &Brush);

		def_pen = dc.SelectObject(&p1);
		dc.MoveTo(0, clrc.bottom - m_pParent->m_offset+1);
		dc.LineTo(clrc.right, clrc.bottom - m_pParent->m_offset+1);
		dc.SelectObject(&p2);
		dc.MoveTo(0, clrc.bottom - m_pParent->m_offset+2);
		dc.LineTo(clrc.right, clrc.bottom - m_pParent->m_offset+2);

		def_font = dc.SelectObject(&(m_pParent->m_fntWC));
		dc.DrawTextW(ECStr, &ecrc, DT_LEFT | DT_WORDBREAK);

		dc.SelectObject(m_ExtrDescr.GetFont());
		dc.DrawTextW(EDStr, &edrc, DT_LEFT | DT_WORDBREAK);

		dc.SelectObject(m_stExtrFile.GetFont());
		dc.DrawTextW(EFStr, &efrc, DT_LEFT | DT_WORDBREAK);

		dc.SelectObject(m_stExtrProgress.GetFont());
		dc.DrawTextW(EPStr, &eprc, DT_LEFT | DT_WORDBREAK);		

		ddgbrc.top -= 15;
		ddgbrc.bottom = ddgbrc.top + 20;
		ddgbrc.left += 10;

		def_font = dc.SelectObject(&(m_pParent->m_fntWC));
		dc.DrawTextW(CString(L"Extraction Progress"), &ddgbrc, DT_LEFT | DT_WORDBREAK);	

		dc.SelectObject(def_font);
		dc.SelectObject(def_pen);
    }

    CDialog::OnPaint();
  }
}

void ExtractionProgressDialog::OnBnClickedCancel()
{
  PauseExtractCabData();

  int res = MessageBox(SFX_EXTR_PROGR_CANCEL_DIALOG_DESCR_TEXT, SFX_EXTR_PROGR_CANCEL_DIALOG_CAPTION, MB_YESNO);

  if (res == IDYES)
  {
    StopExtractCabData();
    OnCancel();
  }
  else
  {
    ContinueExtractCabData();
  }
}
