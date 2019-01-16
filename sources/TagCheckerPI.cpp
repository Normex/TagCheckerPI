
#include "stdafx.h"

#ifndef MAC_PLATFORM
#include "PIHeaders.h"
#endif
#include "commctrl.h"

static AVMenuItem menu_item_fix = NULL;
static AVMenuItem menu_item_check_se = NULL;
static AVMenuItem menu_item_check_classmap = NULL;
static AVMenuItem menu_item_check_rolemap = NULL;

ACCB1 ASBool ACCB2 IsFileOpen(void *clientData);

ACCB1 void ACCB2 FixCommand(void *clientData);
ACCB1 void ACCB2 CheckSE(void *clientData);
ACCB1 void ACCB2 CheckClassMap(void *clientData);
ACCB1 void ACCB2 CheckRoleMap(void *clientData);

//*****************************************************************************
extern ACCB1 ASBool ACCB2 MyPluginUnload() {
  if (menu_item_fix)
    AVMenuItemRemove(menu_item_fix);
  if (menu_item_check_se)
    AVMenuItemRemove(menu_item_check_se);
  if (menu_item_check_classmap)
    AVMenuItemRemove(menu_item_check_classmap);
  if (menu_item_check_rolemap)
    AVMenuItemRemove(menu_item_check_rolemap);

  return true;
}

//*****************************************************************************
ACCB1 ASBool ACCB2 MyPluginSetmenu() {
  AVMenubar menubar = AVAppGetMenubar();
  AVMenu volatile main_menu = NULL;
  if (!menubar)
    return false;

  DURING
    // Create our menuitem
    menu_item_fix = AVMenuItemNew("Fix all", "NORM:TagChecker_AccessibilityLWG:Fix", NULL, true, NO_SHORTCUT, 0, NULL, gExtensionID);
    AVMenuItemSetExecuteProc(menu_item_fix, ASCallbackCreateProto(AVExecuteProc, FixCommand), NULL);
    AVMenuItemSetComputeEnabledProc(menu_item_fix, ASCallbackCreateProto(AVComputeEnabledProc, IsFileOpen), (void *)pdPermEdit);

    menu_item_check_se = AVMenuItemNew("Check SE vs. MC", "NORM:TagChecker_AccessibilityLWG:CheckSE", NULL, true, NO_SHORTCUT, 0, NULL, gExtensionID);
    AVMenuItemSetExecuteProc(menu_item_check_se, ASCallbackCreateProto(AVExecuteProc, CheckSE), NULL);
    AVMenuItemSetComputeEnabledProc(menu_item_check_se, ASCallbackCreateProto(AVComputeEnabledProc, IsFileOpen), NULL);

    menu_item_check_classmap = AVMenuItemNew("Check ClassMap", "NORM:TagChecker_AccessibilityLWG:CheckClassMap", NULL, true, NO_SHORTCUT, 0, NULL, gExtensionID);
    AVMenuItemSetExecuteProc(menu_item_check_classmap, ASCallbackCreateProto(AVExecuteProc, CheckClassMap), NULL);
    AVMenuItemSetComputeEnabledProc(menu_item_check_classmap, ASCallbackCreateProto(AVComputeEnabledProc, IsFileOpen), NULL);

    menu_item_check_rolemap = AVMenuItemNew("Check RoleMap", "NORM:TagChecker_AccessibilityLWG:CheckRoleMap", NULL, true, NO_SHORTCUT, 0, NULL, gExtensionID);
    AVMenuItemSetExecuteProc(menu_item_check_rolemap, ASCallbackCreateProto(AVExecuteProc, CheckRoleMap), NULL);
    AVMenuItemSetComputeEnabledProc(menu_item_check_rolemap, ASCallbackCreateProto(AVComputeEnabledProc, IsFileOpen), NULL);

    main_menu = AVMenubarAcquireMenuByName(menubar, "NORM:TagChecker_AccessibilityLWG:Main");
    if (!main_menu) {
      main_menu = AVMenuNew("Tag Checker", "NORM:TagChecker_AccessibilityLWG:Main", gExtensionID);
      AVMenubarAddMenu(menubar, main_menu, APPEND_MENU);
    }

    AVMenuAddMenuItem(main_menu, menu_item_fix, APPEND_MENUITEM);
    AVMenuAddMenuItem(main_menu, menu_item_check_se, APPEND_MENUITEM);
    AVMenuAddMenuItem(main_menu, menu_item_check_classmap, APPEND_MENUITEM);
    AVMenuAddMenuItem(main_menu, menu_item_check_rolemap, APPEND_MENUITEM);
    AVMenuRelease(main_menu);
  HANDLER
    if (main_menu)
      AVMenuRelease(main_menu);
    return false;
  END_HANDLER

  return true;
}

//*****************************************************************************
ACCB1 ASBool ACCB2 IsFileOpen(void *clientData) {
  return (AVAppGetActiveDoc() != NULL);
}

//*****************************************************************************
// callback to check if Dictionary is empty
static ACCB1 ASBool ACCB2 myCosDictEnumProc(CosObj obj, CosObj value, void* clientData) {
  return false;
}

//*****************************************************************************
ASBool FixPDSElement(PDDoc pd_doc, PDSElement element, ASBool perform_fix) {

  // recursively going through StructElem kids
  ASInt32 num_kids = PDSElementGetNumKids(element);
  ASAtom se_tag = PDSElementGetType(element);

  for (ASInt32 kid_index = 0; kid_index < num_kids; ++kid_index) {
    PDSElement kid;
    PDSMCInfo mcid_info;
    PDSMC marked_content;

    //Get the kid info
    ASAtom kid_type = PDSElementGetKidWithMCInfo(element,  //The PDSElement containing the kid that is retrieved
      kid_index, //The index of the kid.
      &kid,     //The kid being accessed (depending on the kid's type) or NULL.
      &mcid_info,//The kid's information object or NULL.
      (void**)&marked_content,     //Pointer to the kid or NULL.
      NULL );    //The CosObj of the page containing the kid or NULL


    if (kid_type == ASAtomFromString("StructElem")) {
      if (!FixPDSElement(pd_doc, kid, perform_fix))
        return false; //canceled
    }
    else if (kid_type == ASAtomFromString("OBJR")) {
    }
    else if (kid_type == ASAtomFromString("MC")) {
      DURING
        PDEContent pde_content = NULL;
        PDPage page = NULL;
        if (perform_fix) {
          page = PDDocAcquirePage(pd_doc, 0); //rt - need to acquire the right page #
          pde_content = PDPageAcquirePDEContent(page, 0);
        }

        PDEContainer container = PDSMCGetPDEContainer(marked_content);
        //PDEContainerGetDict
        ASAtom mc_tag = PDEContainerGetMCTag(container);

        if (mc_tag != se_tag) {
          if (perform_fix) {
            PDEContainerSetMCTag(container, se_tag);
            PDPageSetPDEContent(page, 0);
          }
          else {
            char buf[1024];
            sprintf(buf, "Problem. SE: %s --> MC: %s \n Continue checking?", ASAtomGetString(se_tag), ASAtomGetString(mc_tag));
            if (AVAlert(ALERT_QUESTION, buf, "Yes", "No", NULL, true) == 2) {
              //cancel process
              E_RETURN(false);
            }
          }
        }

        if (pde_content != NULL)
          PDPageReleasePDEContent(page, 0);
        if (page != NULL)
          PDPageRelease(page);

      HANDLER
      END_HANDLER;
    }

  }
  return true;
}

//*****************************************************************************
ACCB1 void ACCB2 FixCommand(void *clientData) {
  PDDoc pd_doc = AVDocGetPDDoc(AVAppGetActiveDoc());

  PDSTreeRoot pds_tree_root = CosNewNull();
  if (!PDDocGetStructTreeRoot(pd_doc, &pds_tree_root))
    return;

  //Sync the Structure Element types with Marked content tags
  ASInt32 num = PDSTreeRootGetNumKids(pds_tree_root);
  PDSElement elem;
  for (ASInt32 i = 0; i < num; i++) {
    PDSTreeRootGetKid(pds_tree_root, i, &elem);
    FixPDSElement(pd_doc, elem, true);
  }

  //remove ClassMap only if empty
  PDSClassMap class_map;
  if (PDSTreeRootGetClassMap(pds_tree_root, &class_map))
    if (!CosObjEqual(class_map, CosNewNull()))
      if (CosObjEnum(class_map, myCosDictEnumProc, NULL))
        PDSTreeRootRemoveClassMap(pds_tree_root);

  //remove RoleMap only if empty
  PDSRoleMap role_map;
  if (PDSTreeRootGetRoleMap(pds_tree_root, &role_map))
    if (!CosObjEqual(role_map, CosNewNull()))
      if (CosObjEnum(role_map, myCosDictEnumProc, NULL))
        PDSTreeRootRemoveRoleMap(pds_tree_root);
}

//*****************************************************************************
ACCB1 void ACCB2 CheckSE(void *clientData) {
  PDDoc pd_doc = AVDocGetPDDoc(AVAppGetActiveDoc());
  PDSTreeRoot pds_tree_root = CosNewNull();

  if (PDDocGetStructTreeRoot(pd_doc, &pds_tree_root)) {
    ASInt32 num = PDSTreeRootGetNumKids(pds_tree_root);
    PDSElement elem;
    for (ASInt32 i = 0; i < num; i++) {
      PDSTreeRootGetKid(pds_tree_root, i, &elem);
      FixPDSElement(pd_doc, elem, false);
    }
  }
}

//*****************************************************************************
ACCB1 void ACCB2 CheckClassMap(void *clientData) {
  PDDoc pd_doc = AVDocGetPDDoc(AVAppGetActiveDoc());

  PDSTreeRoot pds_tree_root = CosNewNull();
  if (!PDDocGetStructTreeRoot(pd_doc, &pds_tree_root))
    return;

  PDSClassMap class_map;
  if (PDSTreeRootGetClassMap(pds_tree_root, &class_map))
    if (!CosObjEqual(class_map, CosNewNull()))
      if (CosObjEnum(class_map, myCosDictEnumProc, NULL))
        AVAlertNote("ClassMap exists and is empty - can remove it");
      else AVAlertNote("ClassMap exists and not empty - won't remove");
}

//*****************************************************************************
ACCB1 void ACCB2 CheckRoleMap(void *clientData) {
  PDDoc pd_doc = AVDocGetPDDoc(AVAppGetActiveDoc());

  PDSTreeRoot pds_tree_root = CosNewNull();
  if (!PDDocGetStructTreeRoot(pd_doc, &pds_tree_root))  return;

  PDSRoleMap role_map;
  if (PDSTreeRootGetRoleMap(pds_tree_root, &role_map))
    if (!CosObjEqual(role_map, CosNewNull()))
      if (CosObjEnum(role_map, myCosDictEnumProc, NULL))
        AVAlertNote("RoleMap exists and is empty - can remove it");
      else AVAlertNote("RoleMap exists and not empty - won't remove");
}
