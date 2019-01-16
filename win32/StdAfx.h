/*********************************************************************

 ADOBE SYSTEMS INCORPORATED
 Copyright (C) 1999-2003 Adobe Systems Incorporated
 All rights reserved.

 NOTICE: Adobe permits you to use, modify, and distribute this file
 in accordance with the terms of the Adobe license agreement
 accompanying it. If you have received this file from a source other
 than Adobe, then your use, modification, or distribution of it
 requires the prior written permission of Adobe.

 ---------------------------------------------------------------------

 StdAfx.h

 - Include file for standard system include files, or project specific
   include files that are used frequently, but are changed infrequently.

*********************************************************************/

#include <afxwin.h>			// MFC base classes
#include <afxext.h>         // MFC extensions
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC OLE automation classes
#include <afxdb.h>          // MFC database classes
#include <afxcontrolbars.h>

// Standard C++ headers
#include <memory>

// Acrobat Headers.
#ifdef DEBUG
#undef DEBUG
#endif
#ifdef DODEBUG
#undef DODEBUG
#endif

#ifndef MAC_PLATFORM
#include "PIHeaders.h"
#endif
