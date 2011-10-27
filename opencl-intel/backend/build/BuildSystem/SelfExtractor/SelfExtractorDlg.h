// SelfExtractorDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "ExtractionProgressDialog.h"

class ExtractionProgressDialog;

// CSelfExtractorDlg dialog
class CSelfExtractorDlg : public CDialog
{
// Construction
public:
  CSelfExtractorDlg(CWnd* pParent = NULL); // standard constructor

// Dialog Data
  enum { IDD = IDD_SelfExtractor_DIALOG };

  protected:
  virtual void DoDataExchange(CDataExchange* pDX);  // DDX/DDV support


// Implementation
protected:
  HICON m_hIcon;
  ExtractionProgressDialog *m_pExtrDialog;

  // Generated message map functions
  virtual BOOL OnInitDialog();
  afx_msg void OnPaint();
  afx_msg HCURSOR OnQueryDragIcon();
  DECLARE_MESSAGE_MAP()

public:
  CFont m_fntWC;
  COLORREF m_BaseColor;
  //int m_wWidth;
  //int m_wHeight;
  int m_offset;

  afx_msg void OnBnClickedExtract();
  CString m_EditPath;
  afx_msg void OnBnClickedSelectDir();
  CEdit m_cEditPath;
  afx_msg void OnBnClickedCancel();
  CStatic m_WelcomRemTempFiles;
  CStatic m_WelcomCaption;
  CStatic m_WelcomDescr;
  CButton m_CheckRemTempFiles;
  CStatic m_Bitmap;
  CStatic m_DestDirGroupBox;
};
