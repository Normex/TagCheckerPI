#ifndef _TagCheckerPI_H_
#define _TagCheckerPI_H_

#ifndef MAC_PLATFORM
#include "PIHeaders.h"
#endif

// functions do check as well as fix the issue
// if perform_fix == false then return value indicates if the function can/may/requires fix
//  otherwise it should return false (means if perform_fix == true always return false)
bool DoAllignSEWithMC(bool perform_fix = false);
bool DoClassMap(bool perform_fix = false);
bool DoRoleMap(bool perform_fix = false);
bool DoIDTree(bool perform_fix = false);
bool DoAttributes(bool perform_fix = false);
bool DoTitleEntries(bool perform_fix = false);
bool DoIDEntries(bool perform_fix = false);
bool DoOutputIntents(bool perform_fix = false);
bool DoAcroform(bool perform_fix = false);
bool DoOutlines(bool perform_fix = false);
bool DoExtensions(bool perform_fix = false);
bool DoPageLayout(bool perform_fix = false);
bool DoRedundantLangAttribute(bool perform_fix = false);
bool DoActualTextNullTerminator(bool perform_fix = false);

#endif