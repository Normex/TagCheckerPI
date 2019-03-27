// TagCheckerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TagCheckerPI.h"
#include "TagCheckerDlg.h"
#include "afxdialogex.h"

const UINT fixIds[] =
{
  IDC_CHECK_FIX_1_1,  IDC_CHECK_FIX_2_1,  IDC_CHECK_FIX_2_2,
  IDC_CHECK_FIX_2_3,  IDC_CHECK_FIX_2_4,  IDC_CHECK_FIX_2_5,
  IDC_CHECK_FIX_2_6,  IDC_CHECK_FIX_2_7,  IDC_CHECK_FIX_2_8,
  IDC_CHECK_FIX_2_9,  IDC_CHECK_FIX_2_10, IDC_CHECK_FIX_2_11,
  IDC_CHECK_FIX_2_12, IDC_CHECK_FIX_1_2,  IDC_CHECK_FIX_1_3,
  0
};

const UINT wipeIds[] =
{
  IDC_CHECK_WIPE_1, IDC_CHECK_WIPE_2,
  0
};

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
  ON_BN_CLICKED(IDOK, &OnBnClickedOk)
  ON_BN_CLICKED(IDC_CHECK_FIX_ALL, &OnCheckFixAll)
  // these ranges cannot overlap in resource.h
  ON_CONTROL_RANGE(BN_CLICKED, IDC_CHECK_FIX_1_1, IDC_CHECK_FIX_1_3, &OnCheckFix)
  ON_CONTROL_RANGE(BN_CLICKED, IDC_CHECK_WIPE_1, IDC_CHECK_WIPE_2, &OnCheckWipe)
  // END these ranges cannot overlap
END_MESSAGE_MAP()


// CTagCheckerDlg message handlers
void CTagCheckerDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
  CDialogEx::OnActivate(nState, pWndOther, bMinimized);
}

void CTagCheckerDlg::OnCheckFixAll()
{
  UINT bFixAll = IsDlgButtonChecked(IDC_CHECK_FIX_ALL);
 
    for (const UINT* id = fixIds; *id; ++id) //Uncheck all
    {
      CWnd* idWnd = GetDlgItem(*id);
      if (!idWnd->IsWindowEnabled())
        continue;
      CheckDlgButton(*id, bFixAll);
    }
    SetOKBtnState();
}

void CTagCheckerDlg::OnCheckWipe(UINT nID)
{
  if (!IsInWipeIdsRange(nID))
    return;

  SetOKBtnState();
}

void CTagCheckerDlg::OnCheckFix(UINT nID)
{
  if (!IsInFixIdsRange(nID))
    return;

  SetFixAllCheckBtnState();
  SetOKBtnState();
}

void CTagCheckerDlg::OnBnClickedOk()
{
  DoAllignSEWithMC(IsDlgButtonChecked(IDC_CHECK_FIX_1_1) == BST_CHECKED);
  DoActualTextNullTerminator(IsDlgButtonChecked(IDC_CHECK_FIX_1_2) == BST_CHECKED);
  DoAlternateTextNullTerminator(IsDlgButtonChecked(IDC_CHECK_FIX_1_3) == BST_CHECKED);

  DoClassMap(IsDlgButtonChecked(IDC_CHECK_FIX_2_1) == BST_CHECKED);
  DoRoleMap(IsDlgButtonChecked(IDC_CHECK_FIX_2_2) == BST_CHECKED);
  DoIDTree(IsDlgButtonChecked(IDC_CHECK_FIX_2_3) == BST_CHECKED);
  DoAttributes(IsDlgButtonChecked(IDC_CHECK_FIX_2_4) == BST_CHECKED);
  DoTitleEntries(IsDlgButtonChecked(IDC_CHECK_FIX_2_5) == BST_CHECKED);
  DoIDEntries(IsDlgButtonChecked(IDC_CHECK_FIX_2_6) == BST_CHECKED);
  DoOutputIntents(IsDlgButtonChecked(IDC_CHECK_FIX_2_7) == BST_CHECKED);
  DoAcroform(IsDlgButtonChecked(IDC_CHECK_FIX_2_8) == BST_CHECKED);
  DoRedundantLangAttribute(IsDlgButtonChecked(IDC_CHECK_FIX_2_9) == BST_CHECKED);
  DoOutlines(IsDlgButtonChecked(IDC_CHECK_FIX_2_10) == BST_CHECKED);
  DoExtensions(IsDlgButtonChecked(IDC_CHECK_FIX_2_11) == BST_CHECKED);
  DoPageLayout(IsDlgButtonChecked(IDC_CHECK_FIX_2_12) == BST_CHECKED);
  
  if (IsDlgButtonChecked(IDC_CHECK_WIPE_1) == BST_CHECKED)
    CleanDocumentCatalog();
  if (IsDlgButtonChecked(IDC_CHECK_WIPE_2) == BST_CHECKED)
    CleanViewerPreferences();

  CDialogEx::OnOK();
}

BOOL CTagCheckerDlg::OnInitDialog()
{
  CDialogEx::OnInitDialog();

  CheckDlgButton(IDC_CHECK_FOUND_1_1, DoAllignSEWithMC());
  CheckDlgButton(IDC_CHECK_FOUND_1_2, DoActualTextNullTerminator());
  CheckDlgButton(IDC_CHECK_FOUND_1_3, DoAlternateTextNullTerminator());
  CheckDlgButton(IDC_CHECK_FOUND_2_1, DoClassMap());
  CheckDlgButton(IDC_CHECK_FOUND_2_2, DoRoleMap());
  CheckDlgButton(IDC_CHECK_FOUND_2_3, DoIDTree());
  CheckDlgButton(IDC_CHECK_FOUND_2_4, DoAttributes());
  CheckDlgButton(IDC_CHECK_FOUND_2_5, DoTitleEntries());
  CheckDlgButton(IDC_CHECK_FOUND_2_6, DoIDEntries());
  CheckDlgButton(IDC_CHECK_FOUND_2_7, DoOutputIntents());
  CheckDlgButton(IDC_CHECK_FOUND_2_8, DoAcroform());
  CheckDlgButton(IDC_CHECK_FOUND_2_9, DoRedundantLangAttribute());
  CheckDlgButton(IDC_CHECK_FOUND_2_10, DoOutlines());
  CheckDlgButton(IDC_CHECK_FOUND_2_11, DoExtensions());
  CheckDlgButton(IDC_CHECK_FOUND_2_12, DoPageLayout());

  CButton* pBtn = (CButton*)GetDlgItem(IDC_CHECK_FIX_1_1);
  pBtn->EnableWindow(IsDlgButtonChecked(IDC_CHECK_FOUND_1_1));
  pBtn = (CButton*)GetDlgItem(IDC_CHECK_FIX_1_2);
  pBtn->EnableWindow(IsDlgButtonChecked(IDC_CHECK_FOUND_1_2));
  pBtn = (CButton*)GetDlgItem(IDC_CHECK_FIX_1_3);
  pBtn->EnableWindow(IsDlgButtonChecked(IDC_CHECK_FOUND_1_3));
  pBtn = (CButton*)GetDlgItem(IDC_CHECK_FIX_2_1);
  pBtn->EnableWindow(IsDlgButtonChecked(IDC_CHECK_FOUND_2_1));
  pBtn = (CButton*)GetDlgItem(IDC_CHECK_FIX_2_2);
  pBtn->EnableWindow(IsDlgButtonChecked(IDC_CHECK_FOUND_2_2));
  pBtn = (CButton*)GetDlgItem(IDC_CHECK_FIX_2_3);
  pBtn->EnableWindow(IsDlgButtonChecked(IDC_CHECK_FOUND_2_3));
  pBtn = (CButton*)GetDlgItem(IDC_CHECK_FIX_2_4);
  pBtn->EnableWindow(IsDlgButtonChecked(IDC_CHECK_FOUND_2_4));
  pBtn = (CButton*)GetDlgItem(IDC_CHECK_FIX_2_5);
  pBtn->EnableWindow(IsDlgButtonChecked(IDC_CHECK_FOUND_2_5));
  pBtn = (CButton*)GetDlgItem(IDC_CHECK_FIX_2_6);
  pBtn->EnableWindow(IsDlgButtonChecked(IDC_CHECK_FOUND_2_6));
  pBtn = (CButton*)GetDlgItem(IDC_CHECK_FIX_2_7);
  pBtn->EnableWindow(IsDlgButtonChecked(IDC_CHECK_FOUND_2_7));
  pBtn = (CButton*)GetDlgItem(IDC_CHECK_FIX_2_8);
  pBtn->EnableWindow(IsDlgButtonChecked(IDC_CHECK_FOUND_2_8));
  pBtn = (CButton*)GetDlgItem(IDC_CHECK_FIX_2_9);
  pBtn->EnableWindow(IsDlgButtonChecked(IDC_CHECK_FOUND_2_9));
  pBtn = (CButton*)GetDlgItem(IDC_CHECK_FIX_2_10);
  pBtn->EnableWindow(IsDlgButtonChecked(IDC_CHECK_FOUND_2_10));
  pBtn = (CButton*)GetDlgItem(IDC_CHECK_FIX_2_11);
  pBtn->EnableWindow(IsDlgButtonChecked(IDC_CHECK_FOUND_2_11));
  pBtn = (CButton*)GetDlgItem(IDC_CHECK_FIX_2_12);
  pBtn->EnableWindow(IsDlgButtonChecked(IDC_CHECK_FOUND_2_12));

  SetOKBtnState();
  SetFixAllCheckBtnState();

  return TRUE;  // return TRUE unless you set the focus to a control
                // EXCEPTION: OCX Property Pages should return FALSE
}

void CTagCheckerDlg::SetOKBtnState()
{
  BOOL isFixChecked = IsAnyIdChecked(fixIds);
  BOOL isWipeChecked = IsAnyIdChecked(wipeIds);

  CButton* pOKBtn = (CButton*)GetDlgItem(IDOK);
  pOKBtn->EnableWindow(isFixChecked || isWipeChecked);
}

BOOL CTagCheckerDlg::IsAnyFixEnabled()
{
  BOOL isEnabled = FALSE;
  for (const UINT* fixId = fixIds; *fixId; ++fixId) {
    CWnd* idWnd = GetDlgItem(*fixId);
    if (idWnd->IsWindowEnabled()) {
      isEnabled = TRUE;
      break;
    }
  }
  return isEnabled;
}

BOOL CTagCheckerDlg::IsAnyIdChecked(const UINT ids[])
{
  BOOL isChecked = FALSE;
  for (const UINT* id = ids; *id; ++id) {
    if (IsDlgButtonChecked(*id)) {
      isChecked = TRUE;
      break;
    }
  }
  return isChecked;
}

BOOL CTagCheckerDlg::IsAllFixesChecked()
{
  BOOL isChecked = TRUE;
  for (const UINT* fixId = fixIds; *fixId; ++fixId) {
    CWnd* idWnd = GetDlgItem(*fixId);
    if (idWnd->IsWindowEnabled() && !IsDlgButtonChecked(*fixId)) {
      isChecked = FALSE;
      break;
    }
  }
  return isChecked;
}

void CTagCheckerDlg::SetFixAllCheckBtnState()
{
  CButton* pCheckFixAllBtn = (CButton*)GetDlgItem(IDC_CHECK_FIX_ALL);

  if (IsAllFixesChecked() && IsAnyFixEnabled())
    pCheckFixAllBtn->SetCheck(TRUE);
  else
    pCheckFixAllBtn->SetCheck(FALSE);

  if (IsAnyFixEnabled())
    pCheckFixAllBtn->EnableWindow(TRUE);
  else
    pCheckFixAllBtn->EnableWindow(FALSE);

}

bool CTagCheckerDlg::IsInWipeIdsRange(UINT nID)
{
  bool isWipeId = false;
  for (const UINT* id = wipeIds; *id; ++id) {
    if (nID == *id) {
      isWipeId = true;
      break;
    }
  }
  return isWipeId;
}

bool CTagCheckerDlg::IsInFixIdsRange(UINT nID)
{
  bool isFixId = false;
  for (const UINT* id = fixIds; *id; ++id) {
    if (nID == *id) {
      isFixId = true;
      break;
    }
  }
  return isFixId;
}
