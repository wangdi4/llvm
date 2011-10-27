#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "SelfExtractorDlg.h"

class CSelfExtractorDlg;

// ExtractionProgressDialog dialog

class ExtractionProgressDialog : public CDialog
{
  DECLARE_DYNAMIC(ExtractionProgressDialog)

public:
  CSelfExtractorDlg *m_pParent;

  ExtractionProgressDialog(CWnd* pParent = NULL);   // standard constructor
  virtual ~ExtractionProgressDialog();

// Dialog Data
  enum { IDD = IDD_DIALOG_EXTRACTION_PROGRESS };

protected:
  HICON m_hIcon;
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnNMCustomdrawProgress1(NMHDR *pNMHDR, LRESULT *pResult);
  CProgressCtrl m_ProgressCtrl;
  virtual BOOL OnInitDialog();
  CStatic m_stExtrProgress;
  afx_msg void OnPaint();
  CStatic m_ExtrCap;
  CStatic m_ExtrDescr;
  CProgressCtrl m_ProgressFileCtrl;
  CStatic m_stExtrFile;
  afx_msg void OnBnClickedCancel();
  CStatic m_Bitmap;
  CStatic m_DestDirGroupBox;
};
