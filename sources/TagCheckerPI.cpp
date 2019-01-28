
#include "stdafx.h"

#ifndef MAC_PLATFORM
#include "PIHeaders.h"
#endif
#include "commctrl.h"
#include <functional>

typedef std::function< bool(bool perform_fix, 
                            PDSElement element,
                            ASAtom kid_type, 
                            CosObj kid, 
                            PDSMCInfo mcid_info,
                            PDSMC marked_content)> ProcessStructureElementFunction;

//*****************************************************************************
// callback to check if Dictionary is empty
static ACCB1 ASBool ACCB2 myCosDictEnumProc(CosObj obj, CosObj value, void* clientData) {
  return false;
}

//*****************************************************************************
bool DoStructureElement(PDDoc pd_doc, PDSElement element, bool perform_fix, ProcessStructureElementFunction callback) {

  // recursively going through StructElem kids
  ASInt32 num_kids = PDSElementGetNumKids(element);
  // -- debug
  //char buf[1024];
  //sprintf(buf, "SE: %s ", ASAtomGetString(PDSElementGetType(element)));

  bool to_ret = false;
  for (ASInt32 kid_index = 0; kid_index < num_kids; ++kid_index) {
    CosObj kid;
    PDSMCInfo mcid_info;
    PDSMC marked_content;

    //Get the kid info
    ASAtom kid_type = PDSElementGetKidWithMCInfo(element,  //The PDSElement containing the kid that is retrieved
      kid_index, //The index of the kid.
      &kid,     //The kid being accessed (depending on the kid's type) or NULL.
      &mcid_info,//The kid's information object or NULL.
      (void**)&marked_content,     //Pointer to the kid or NULL.
      NULL);    //The CosObj of the page containing the kid or NULL

    // calls callback that performs the check or the fix
    if (callback(perform_fix, element, kid_type, kid,mcid_info, marked_content))
      return true;

    if (kid_type == ASAtomFromString("StructElem")) {
      to_ret = to_ret || DoStructureElement(pd_doc, kid, perform_fix, callback);
    };
  }
  return to_ret;
}

//*****************************************************************************
// General function that recursively goes through all Structure elements and 
//on each SE invokes the callback. Returns true only if perform_fix is false and 
//it indicates that fix is possible
bool DoStructureElement(bool perform_fix, ProcessStructureElementFunction callback) {
  PDDoc pd_doc = AVDocGetPDDoc(AVAppGetActiveDoc());
  PDSTreeRoot pds_tree_root = CosNewNull();
  bool to_ret = false;
  if (PDDocGetStructTreeRoot(pd_doc, &pds_tree_root)) {
    ASInt32 num = PDSTreeRootGetNumKids(pds_tree_root);
    PDSElement elem;
    for (ASInt32 i = 0; i < num; i++) {
      PDSTreeRootGetKid(pds_tree_root, i, &elem);
      to_ret = to_ret || DoStructureElement(pd_doc, elem, perform_fix, callback);
    }
  }
  return to_ret;
}

//*****************************************************************************
PDPage PDDocAcquirePageFromCosObj(PDDoc pd_doc, CosObj page_obj) {
  for (auto i = 0; i < PDDocGetNumPages(pd_doc); i++)
    if (CosObjEqual(page_obj, PDDocGetPageObjByNum(pd_doc, i)))
      return PDDocAcquirePage(pd_doc, i);
  return NULL;
}

//*****************************************************************************
bool DoAllignSEWithMC(bool perform_fix) {
  ProcessStructureElementFunction sync_se_and_mc = [](bool perform_fix, PDSElement element,
    ASAtom kid_type, CosObj kid, PDSMCInfo mcid_info, PDSMC marked_content) {

    if (kid_type != ASAtomFromString("MC")) return false;
    if (perform_fix) {
      //need to acquire content to be able to change the MC container
      PDEContent pde_content = NULL;
      PDPage page = PDDocAcquirePageFromCosObj(AVDocGetPDDoc(AVAppGetActiveDoc()), mcid_info.page);
      if (page != NULL)
        pde_content = PDPageAcquirePDEContent(page, 0);
      else {
        AVAlertNote("Can't find page for mcid");
      }

      PDEContainer container = PDSMCGetPDEContainer(marked_content);
      ASAtom mc_tag = PDEContainerGetMCTag(container);
      if (mc_tag != PDSElementGetType(element)) {
        PDEContainerSetMCTag(container, PDSElementGetType(element));
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
      //debug
      //char buf[1024];
      //sprintf(buf, "Problem. SE: %s --> MC: %s ", ASAtomGetString(PDSElementGetType(element)), ASAtomGetString(mc_tag));
      if (mc_tag != PDSElementGetType(element))
        return true;
    }
    return false;
  };

  return DoStructureElement(perform_fix, sync_se_and_mc);
}

//*****************************************************************************
bool DoClassMap(bool perform_fix) {
  PDDoc pd_doc = AVDocGetPDDoc(AVAppGetActiveDoc());

  PDSTreeRoot pds_tree_root = CosNewNull();
  if (!PDDocGetStructTreeRoot(pd_doc, &pds_tree_root))
    return false;

  PDSClassMap class_map;
  if (PDSTreeRootGetClassMap(pds_tree_root, &class_map))
    if (!CosObjEqual(class_map, CosNewNull()))
      if (CosObjEnum(class_map, myCosDictEnumProc, NULL))
        if (!perform_fix) return true;
        else PDSTreeRootRemoveClassMap(pds_tree_root);
  return false;
}

//*****************************************************************************
bool DoRoleMap(bool perform_fix) {
  PDDoc pd_doc = AVDocGetPDDoc(AVAppGetActiveDoc());

  PDSTreeRoot pds_tree_root = CosNewNull();
  if (!PDDocGetStructTreeRoot(pd_doc, &pds_tree_root))  return false;

  PDSRoleMap role_map;
  if (PDSTreeRootGetRoleMap(pds_tree_root, &role_map))
    if (!CosObjEqual(role_map, CosNewNull()))
      if (CosObjEnum(role_map, myCosDictEnumProc, NULL))
        if (!perform_fix) return true;
        else PDSTreeRootRemoveRoleMap(pds_tree_root);
  return false;
}

//*****************************************************************************
bool DoIDTree(bool perform_fix) {
  return false;
}

//*****************************************************************************
bool DoAttributes(bool perform_fix) {
  ProcessStructureElementFunction attributes = [](bool perform_fix, PDSElement element, 
    ASAtom kid_type, CosObj kid, PDSMCInfo mcid_info, PDSMC marked_content) {
    if (kid_type == ASAtomFromString("StructElem")) {
      CosObj kid_obj = PDSElementGetCosObj(kid);
      if (CosObjGetType(kid_obj) == CosDict) {
        //remove empty Attributes
        CosObj attr_obj = CosDictGet(kid_obj, ASAtomFromString("A"));
        if (!CosObjEqual(attr_obj, CosNewNull()))
          if (CosObjEnum(attr_obj, myCosDictEnumProc, NULL))
            if (!perform_fix) return true;
            else CosDictRemove(kid_obj, ASAtomFromString("A"));
      }
    }
    return false;
  };
  return DoStructureElement(perform_fix, attributes);
}

//*****************************************************************************
bool DoTitleEntries(bool perform_fix) {
  ProcessStructureElementFunction titleEntries = [](bool perform_fix, PDSElement element,
    ASAtom kid_type, CosObj kid, PDSMCInfo mcid_info,PDSMC marked_content) {
    if (kid_type == ASAtomFromString("StructElem")) {
      CosObj kid_obj = PDSElementGetCosObj(kid);
      if (CosObjGetType(kid_obj) == CosDict) {
        //remove empty T key
        CosObj title_obj = CosDictGet(kid_obj, ASAtomFromString("T"));
        if (!CosObjEqual(title_obj, CosNewNull())) {
          ASTCount count;
          CosStringValue(title_obj, &count);
          if (count == 0)
            if (!perform_fix) return true;
            else CosDictRemove(kid_obj, ASAtomFromString("T"));
        }
      }
    }
    return false; 
  };

  return DoStructureElement(perform_fix, titleEntries);
}

//*****************************************************************************
bool DoIDEntries(bool perform_fix) {
  ProcessStructureElementFunction ids = [](bool perform_fix, PDSElement element, 
    ASAtom kid_type, CosObj kid, PDSMCInfo mcid_info, PDSMC marked_content) {
    if (kid_type == ASAtomFromString("StructElem")) {
      CosObj kid_obj = PDSElementGetCosObj(kid);
      if (CosObjGetType(kid_obj) == CosDict) {
        //remove empty ID
        CosObj id_obj = CosDictGet(kid_obj, ASAtomFromString("ID"));
        if (!CosObjEqual(id_obj, CosNewNull())) {
          ASTCount count;
          CosStringValue(id_obj, &count);
          if (count == 0)
            if (!perform_fix) return true;
            else CosDictRemove(kid_obj, ASAtomFromString("ID"));
        }
      }
    }
    return false;
  };
  return DoStructureElement(perform_fix, ids);
}

//*****************************************************************************
bool DoOutputIntents(bool perform_fix) {
//  *Remove Output Intents entry(OutputIntents dictionary from catalog is removed if exists)
  CosObj catalog = CosDocGetRoot(PDDocGetCosDoc(AVDocGetPDDoc(AVAppGetActiveDoc())));
  CosObj oi = CosDictGet(catalog, ASAtomFromString("OutputIntents"));
  if (!CosObjEqual(oi, CosNewNull()))
    if (!perform_fix) return true;
    else CosDictRemove(catalog, ASAtomFromString("OutputIntents"));
  return false;
}

//*****************************************************************************
bool DoAcroform(bool perform_fix) {
  //*Remove Acroform entry(if Fields entry in AcroFom is empty array, we remove whole AcroForm entry in catalog dictionary)
  CosObj catalog = CosDocGetRoot(PDDocGetCosDoc(AVDocGetPDDoc(AVAppGetActiveDoc())));

  CosObj acroform = CosDictGet(catalog, ASAtomFromString("AcroForm"));
  if (!CosObjEqual(acroform, CosNewNull())) {
    CosObj fields = CosDictGet(acroform, ASAtomFromString("Fields"));
    if ((CosObjGetType(fields) == CosArray) && (CosArrayLength(fields) == 0))
      if (!perform_fix) return true;
      else CosDictRemove(catalog, ASAtomFromString("AcroForm"));
  }
  return false;
}

//*****************************************************************************
bool DoRedundantLangAttribute(bool perform_fix) {
  return false;
}
