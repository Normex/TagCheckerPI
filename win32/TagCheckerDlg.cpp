// TagCheckerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TagCheckerPI.h"
#include "TagCheckerDlg.h"
#include "afxdialogex.h"


// CTagCheckerDlg dialog

IMPLEMENT_DYNAMIC(CTagCheckerDlg, CDialogEx)

CTagCheckerDlg::CTagCheckerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_TAGCHECKER_DLG, pParent)
{
}

CTagCheckerDlg::~CTagCheckerDlg()
{
}

void CTagCheckerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CTagCheckerDlg, CDialogEx)
  ON_WM_ACTIVATE()
  ON_BN_CLICKED(IDOK, &CTagCheckerDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CTagCheckerDlg message handlers
void CTagCheckerDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
  CDialogEx::OnActivate(nState, pWndOther, bMinimized);
}

void CTagCheckerDlg::OnBnClickedOk()
{
  DoAllignSEWithMC(IsDlgButtonChecked(IDC_CHECK_FIX_1_1) == BST_CHECKED);

  DoClassMap(IsDlgButtonChecked(IDC_CHECK_FIX_2_1) == BST_CHECKED);
  DoRoleMap(IsDlgButtonChecked(IDC_CHECK_FIX_2_2) == BST_CHECKED);
  DoIDTree(IsDlgButtonChecked(IDC_CHECK_FIX_2_3) == BST_CHECKED);
  DoAttributes(IsDlgButtonChecked(IDC_CHECK_FIX_2_4) == BST_CHECKED);
  DoTitleEntries(IsDlgButtonChecked(IDC_CHECK_FIX_2_5) == BST_CHECKED);
  DoIDEntries(IsDlgButtonChecked(IDC_CHECK_FIX_2_6) == BST_CHECKED);
  DoOutputIntents(IsDlgButtonChecked(IDC_CHECK_FIX_2_7) == BST_CHECKED);
  DoAcroform(IsDlgButtonChecked(IDC_CHECK_FIX_2_8) == BST_CHECKED);
  DoRedundantLangAttribute(IsDlgButtonChecked(IDC_CHECK_FIX_2_9) == BST_CHECKED);
  CDialogEx::OnOK();
}


BOOL CTagCheckerDlg::OnInitDialog()
{
  CDialogEx::OnInitDialog();

  CheckDlgButton(IDC_CHECK_FOUND_1_1, DoAllignSEWithMC());

  CheckDlgButton(IDC_CHECK_FOUND_2_1, DoClassMap());
  CheckDlgButton(IDC_CHECK_FOUND_2_2, DoRoleMap());
  CheckDlgButton(IDC_CHECK_FOUND_2_3, DoIDTree());
  CheckDlgButton(IDC_CHECK_FOUND_2_4, DoAttributes());
  CheckDlgButton(IDC_CHECK_FOUND_2_5, DoTitleEntries());
  CheckDlgButton(IDC_CHECK_FOUND_2_6, DoIDEntries());
  CheckDlgButton(IDC_CHECK_FOUND_2_7, DoOutputIntents());
  CheckDlgButton(IDC_CHECK_FOUND_2_8, DoAcroform());
  CheckDlgButton(IDC_CHECK_FOUND_2_9, DoRedundantLangAttribute());

  return TRUE;  // return TRUE unless you set the focus to a control
                // EXCEPTION: OCX Property Pages should return FALSE
}
