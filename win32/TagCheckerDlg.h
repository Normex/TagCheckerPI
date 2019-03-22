#pragma once

#include "resource.h"

// CTagCheckerDlg dialog

class CTagCheckerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CTagCheckerDlg)

public:
	CTagCheckerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTagCheckerDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TAGCHECKER_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
  afx_msg void OnBnClickedOk();
  afx_msg void OnCheckFixAll();
  afx_msg void OnCheckFix(UINT nID);
  afx_msg void OnCheckWipe(UINT nID);
  virtual BOOL OnInitDialog();
private:
  void SetOKBtnState();
  BOOL IsAnyFixEnabled();
  BOOL IsAnyIdChecked(const UINT ids[]);
  BOOL IsAllFixesChecked();
  void SetFixAllCheckBtnState();
  bool IsInWipeIdsRange(UINT nID);
  bool IsInFixIdsRange(UINT nID);
};
