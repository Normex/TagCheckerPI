
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

PDPage PDDocAcquirePageFromCosObj(PDDoc pd_doc, CosObj page_obj) {
  for (auto i = 0; i < PDDocGetNumPages(pd_doc); i++)
    if (CosObjEqual(page_obj, PDDocGetPageObjByNum(pd_doc, i)))
      return PDDocAcquirePage(pd_doc, i);
  return NULL;
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
        if (perform_fix) {
          //need to acquire content to be able to change the MC container
          PDEContent pde_content = NULL;
          PDPage page = PDDocAcquirePageFromCosObj(pd_doc, mcid_info.page);
          if (page != NULL)
            pde_content = PDPageAcquirePDEContent(page, 0);
          else {
            AVAlertNote("Can't find page for mcid");
          }

          PDEContainer container = PDSMCGetPDEContainer(marked_content);
          ASAtom mc_tag = PDEContainerGetMCTag(container);
          if (mc_tag != se_tag) {
            PDEContainerSetMCTag(container, se_tag);
            PDPageSetPDEContent(page, 0);
          }

          if (pde_content != NULL)
            PDPageReleasePDEContent(page, 0);
          if (page != NULL)
            PDPageRelease(page);
        }
        else {
          // just checking if the SE type matches the MC tag
          ASAtom mc_tag = PDEContainerGetMCTag(PDSMCGetPDEContainer(marked_content));
          if (mc_tag != se_tag) {
            char buf[1024];
            sprintf(buf, "Problem. SE: %s --> MC: %s \n Continue checking?", ASAtomGetString(se_tag), ASAtomGetString(mc_tag));
            if (AVAlert(ALERT_QUESTION, buf, "Yes", "No", NULL, true) == 2) {
              //cancel process
              E_RETURN(false);
            }
          }
        }
      HANDLER
      END_HANDLER;
    }

  }
  return true;
}

//*****************************************************************************
ASBool RemoveEmptyKeys(PDSElement element) {
  // recursively going through StructElem kids
  ASInt32 num_kids = PDSElementGetNumKids(element);

  for (ASInt32 kid_index = 0; kid_index < num_kids; ++kid_index) {
    PDSElement kid;

    //Get the kid info
    ASAtom kid_type = PDSElementGetKid(element,  //The PDSElement containing the kid that is retrieved
      kid_index, //The index of the kid.
      &kid,     //The kid being accessed (depending on the kid's type) or NULL.
      NULL,     //Pointer to the kid or NULL.
      NULL);    //The CosObj of the page containing the kid or NULL

    if (kid_type == ASAtomFromString("StructElem")) {
      DURING
        CosObj kid_obj = PDSElementGetCosObj(kid);
        if (CosObjGetType(kid_obj) == CosDict) {

          //remove empty T key
          CosObj title_obj = CosDictGet(kid_obj, ASAtomFromString("T"));
          if (!CosObjEqual(title_obj, CosNewNull())) {
            ASTCount count;
            CosStringValue(title_obj, &count);
            if (count == 0)
              CosDictRemove(kid_obj, ASAtomFromString("T"));
          }

          //remove empty A
          CosObj attr_obj = CosDictGet(kid_obj, ASAtomFromString("A"));
          if (!CosObjEqual(attr_obj, CosNewNull()))
            if (CosObjEnum(attr_obj, myCosDictEnumProc, NULL))
              CosDictRemove(kid_obj, ASAtomFromString("A"));

          //remove empty ID
          CosObj id_obj = CosDictGet(kid_obj, ASAtomFromString("ID"));
          if (!CosObjEqual(id_obj, CosNewNull())) {
            ASTCount count;
            CosStringValue(id_obj, &count);
            if (count == 0)
              CosDictRemove(kid_obj, ASAtomFromString("ID"));
          }
        }
      HANDLER
      END_HANDLER
      RemoveEmptyKeys(kid);
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

  ASInt32 num = PDSTreeRootGetNumKids(pds_tree_root);
  PDSElement elem;
  for (ASInt32 i = 0; i < num; i++) {
    PDSTreeRootGetKid(pds_tree_root, i, &elem);
    //Sync the Structure Element types with Marked content tags
    FixPDSElement(pd_doc, elem, true);
    //remove empty keys
    RemoveEmptyKeys(elem);
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
