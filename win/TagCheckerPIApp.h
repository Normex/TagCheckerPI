#if !defined(AFX_CTagCheckerPIApp_H)
#define AFX_CTagCheckerPIApp_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CTagCheckerPIApp:
// See CTagCheckerPIApp.cpp for the implementation of this class
//

class CTagCheckerPIApp : public CWinApp
{
public:
  CTagCheckerPIApp();

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CTagCheckerPIApp)
  //}}AFX_VIRTUAL

  // Implementation
  //{{AFX_MSG(CTagCheckerPIApp)
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CTagCheckerPIApp_H)