
#ifndef MAC_PLATFORM
#include "stdafx.h"
#include "commctrl.h"
#include "PIHeaders.h"
#endif

#include <algorithm>
#include <functional>
#include <vector>

struct dictionary_keys {
  std::vector<ASAtom> to_keep;
  std::vector<ASAtom> to_remove;
};

typedef std::function<bool(
  bool perform_fix,
  PDSElement element,
  ASAtom kid_type,
  int kid_index,
  CosObj kid,
  PDSMCInfo mcid_info/*,
  PDSMC marked_content*/)> ProcessStructureElementFunction;

//*****************************************************************************
// callback to check if Dictionary is empty
static ACCB1 ASBool ACCB2 MyCosDictEnumProc(CosObj obj, CosObj value, void* clientData) {
  return false;
}

//*****************************************************************************
// callback to get keys which should be removed 
static ACCB1 ASBool ACCB2 GetKeysToRemoveEnumProc(CosObj key, CosObj value, void* clientData) {
  if (!clientData)
    return false;

  dictionary_keys* keys = reinterpret_cast<dictionary_keys*>(clientData);
  auto it = std::find(keys->to_keep.begin(), keys->to_keep.end(), CosNameValue(key));

  //found, therefor return and do not add to to_remove keys
  if (it != keys->to_keep.end())
    return true;

  //not found in to_keep keys therefor push to to_remove keys, which are to be deleted
  keys->to_remove.push_back(CosNameValue(key));
  return true;
}

static ACCB1 ASBool ACCB2 GetKeysEnumProc(CosObj key, CosObj value, void* clientData) {
  if (!clientData)
    return false;

  std::vector<ASAtom>* keys = reinterpret_cast<std::vector<ASAtom>*>(clientData);
  keys->push_back(CosNameValue(key));
  return true;
}

//*****************************************************************************
bool DoStructureElement(PDDoc pd_doc, PDSElement element, bool perform_fix, ProcessStructureElementFunction callback) {

  // recursively going through StructElem kids
  ASInt32 num_kids = PDSElementGetNumKids(element);
  // -- debug
  char buf[1024];
  char buf2[1024];
  sprintf(buf, "SE: %s ", ASAtomGetString(PDSElementGetType(element)));
  PDSElementGetTitle(element, (ASUns8*)buf2);

  bool to_ret = false;
  for (ASInt32 kid_index = 0; kid_index < num_kids; ++kid_index) {
    CosObj kid;
    PDSMCInfo mcid_info;
    //PDSMC marked_content;

    //Get the kid info
    ASAtom kid_type = PDSElementGetKidWithMCInfo(element,  //The PDSElement containing the kid that is retrieved
      kid_index, //The index of the kid.
      &kid,     //The kid being accessed (depending on the kid's type) or NULL.
      &mcid_info,//The kid's information object or NULL.
      NULL, //(void**)&marked_content,     //Pointer to the kid or NULL.
      NULL);    //The CosObj of the page containing the kid or NULL
    
    // calls callback that performs the check or the fix
    if (callback(perform_fix, element, kid_type, kid_index, kid, mcid_info/*, marked_content*/))
      return true;

    if (kid_type == ASAtomFromString("StructElem")) {
      to_ret = to_ret || DoStructureElement(pd_doc, kid, perform_fix, callback);
    }
  }
  return to_ret;
}

//*****************************************************************************
// General function that recursively goes through all Structure elements and 
//on each SE invokes the callback. Returns true only if perform_fix is false and 
//it indicates that fix is possible
bool DoStructureTreeRoot(bool perform_fix, ProcessStructureElementFunction callback) {
  PDDoc pd_doc = AVDocGetPDDoc(AVAppGetActiveDoc());
  PDSTreeRoot pds_tree_root = CosNewNull();
  bool to_ret = false;
  if (PDDocGetStructTreeRoot(pd_doc, &pds_tree_root)) {
    ASInt32 num = PDSTreeRootGetNumKids(pds_tree_root);
    PDSElement elem;
    for (ASInt32 i = 0; i < num; i++) {
      PDSTreeRootGetKid(pds_tree_root, i, &elem);
      if (callback(perform_fix, CosNewNull(), ASAtomFromString("StructElem"), -1, elem, PDSMCInfo()/*, NULL*/))
        return true;
      to_ret = to_ret || DoStructureElement(pd_doc, elem, perform_fix, callback);
    }
  }
  return to_ret;
}

bool HasArtifactNestedContainer(PDEElement elem, PDEContent artifacted_content, bool perform_fix) {
  bool hasNestedContainer = false;
  if (PDEObjectGetType((PDEObject)elem) == kPDEContainer) {
    ASAtom elem_atom = PDEContainerGetMCTag((PDEContainer)elem);
    auto elem_atom_cstr = ASAtomGetString(elem_atom);
    bool is_artifact = ASAtomFromString("Artifact") == elem_atom;
    if (!artifacted_content && is_artifact) {
      artifacted_content = PDEContentCreate();
    }
    else if (artifacted_content) {
      hasNestedContainer = true;
      if (!perform_fix)
        return true; //stop, first occurrence of nested container under artifact found,no need to continue searching when not fixing
    }

    PDEContent elem_content = PDEContainerGetContent((PDEContainer)elem);
    int num_elem_content = PDEContentGetNumElems(elem_content);
    for (int i = 0; i < num_elem_content; i++) {
      PDEElement kid_elem = PDEContentGetElem(elem_content, i);
      hasNestedContainer |= HasArtifactNestedContainer(kid_elem, artifacted_content, perform_fix);
      if (hasNestedContainer && !perform_fix)
        return true;
    }

    if (is_artifact) {
      PDEContainerSetContent((PDEContainer)elem, artifacted_content);
      //PDERelease((PDEObject)artifacted_content); //crashed
      artifacted_content = NULL;
    }
  }
  else if (artifacted_content)
    PDEContentAddElem(artifacted_content, kPDEAfterLast, elem);

  return hasNestedContainer;
}

bool HasArtifactNestedContainer(bool perform_fix) {
  bool hasNestedContainer = false;
  PDDoc doc = AVDocGetPDDoc(AVAppGetActiveDoc());
  ASInt32 num_pages = PDDocGetNumPages(doc);
  for (int i = 0; i < num_pages; i++) {
    PDPage page = PDDocAcquirePage(doc, i);
    PDEContent content = PDPageAcquirePDEContent(page, 0);
    DURING
      ASInt32 num_elems = PDEContentGetNumElems(content);
      for (int i = 0; i < num_elems; i++) {
        PDEElement elem = PDEContentGetElem(content, i);
        hasNestedContainer |= HasArtifactNestedContainer(elem, NULL, perform_fix);
        if (hasNestedContainer && !perform_fix) {
          PDPageReleasePDEContent(page, 0);
          content = NULL;
          PDPageRelease(page);
          page = NULL;
          break;
        }
      }

      if (hasNestedContainer && perform_fix) {
        PDPageSetPDEContent(page, gExtensionID);
        PDPageNotifyContentsDidChange(page);
      }

      PDPageReleasePDEContent(page, 0);
      content = NULL;
      PDPageRelease(page);
      page = NULL;
    HANDLER
      if (page && content) {
        PDPageReleasePDEContent(page, 0);
        content = NULL;
      }
      if (page) {
        PDPageRelease(page);
        page = NULL;
      }
#ifndef MAC_PLATFORM
#ifdef _DEBUG
      AVAlertNote("HasArtifactNestedContainer Exception");
#endif // _DEBUG
#endif
    END_HANDLER
  }
  return hasNestedContainer;
}

bool HasContainerOtherThanMCIDEntry(PDEElement elem, bool perform_fix) {
  bool hasOtherThanMCID = false;
  if (PDEObjectGetType((PDEObject)elem) == kPDEContainer) {
    ASAtom elem_tag = PDEContainerGetMCTag((PDEContainer)elem);
    auto elem_tag_cstr = ASAtomGetString(elem_tag);
    if (elem_tag == ASAtomFromString("Artifact")) {
      return false;
    }

    CosObj dict;
    ASBool is_in_line;
    if (PDEContainerGetDict((PDEContainer)elem, &dict, &is_in_line)) {
      std::vector<ASAtom> keys;
      CosObjEnum(dict, GetKeysEnumProc, &keys);
      for (auto key : keys)
      {
        if (key != ASAtomFromString("MCID"))
        {
          hasOtherThanMCID = true;
          if (!perform_fix)
            return true;
          CosDictRemove(dict, key);
#ifndef MAC_PLATFORM
#ifdef _DEBUG
          char msgbuf[256];
          sprintf_s(msgbuf, sizeof(msgbuf), "Removed from Catalog: %s\n", ASAtomGetString(key));
          OutputDebugString(msgbuf);
#endif // _DEBUG
#endif
        }
      }
    }

    PDEContent elem_content = PDEContainerGetContent((PDEContainer)elem);
    int num_elem_content = PDEContentGetNumElems(elem_content);
    for (int i = 0; i < num_elem_content; i++) {
      PDEElement kid_elem = PDEContentGetElem(elem_content, i);
      hasOtherThanMCID |= HasContainerOtherThanMCIDEntry(kid_elem, perform_fix);
      if (hasOtherThanMCID && !perform_fix)
        break;
    }
  }

  return hasOtherThanMCID;
}

bool HasContainerOtherThanMCIDEntry(bool perform_fix) {
  bool hasPropertyDict = false;
  PDDoc doc = AVDocGetPDDoc(AVAppGetActiveDoc());
  ASInt32 num_pages = PDDocGetNumPages(doc);
  for (int i = 0; i < num_pages; i++) {
    PDPage page = PDDocAcquirePage(doc, i);
    PDEContent content = PDPageAcquirePDEContent(page, 0);
    DURING
      ASInt32 num_elems = PDEContentGetNumElems(content);
      for (int i = 0; i < num_elems; i++) {
        PDEElement elem = PDEContentGetElem(content, i);
        hasPropertyDict |= HasContainerOtherThanMCIDEntry(elem, perform_fix);
        if (hasPropertyDict && !perform_fix) {
          PDPageReleasePDEContent(page, 0);
          content = NULL;
          PDPageRelease(page);
          page = NULL;
          break;
        }
      }

      if (hasPropertyDict && perform_fix) {
        PDPageSetPDEContent(page, gExtensionID);
        PDPageNotifyContentsDidChange(page);
      }

      PDPageReleasePDEContent(page, 0);
      content = NULL;
      PDPageRelease(page);
      page = NULL;
    HANDLER
      if (page && content) {
        PDPageReleasePDEContent(page, 0);
        content = NULL;
      }
      if (page) {
        PDPageRelease(page);
        page = NULL;
      }
#ifndef MAC_PLATFORM
#ifdef _DEBUG
      AVAlertNote("HasContainerOtherThanMCIDValue Exception");
#endif // _DEBUG
#endif
    END_HANDLER
  }
  return hasPropertyDict;
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
    ASAtom kid_type, int kid_index, CosObj kid, PDSMCInfo mcid_info/*, PDSMC marked_content*/) {

    if (kid_type != ASAtomFromString("MC")) return false;

    bool to_ret = false;
    //need to acquire content to be able to change the MC container
    PDEContent pde_content = NULL;
    PDPage page = PDDocAcquirePageFromCosObj(AVDocGetPDDoc(AVAppGetActiveDoc()), mcid_info.page);
    if (page != NULL)
      pde_content = PDPageAcquirePDEContent(page, 0);
    else
      AVAlertNote("Can't find page for mcid");

    PDSMC marked_content=NULL;
    //CosObj kid2;
    //Get the kid info
    DURING
      ASAtom kid_type_new = PDSElementGetKidWithMCInfo(element,  //The PDSElement containing the kid that is retrieved
        kid_index, //The index of the kid.
        NULL,     //The kid being accessed (depending on the kid's type) or NULL.
        NULL,     //The kid's information object or NULL.
        (void**)&marked_content,     //Pointer to the kid or NULL.
        NULL);    //The CosObj of the page containing the kid or NULL
    HANDLER
      marked_content = NULL;
    END_HANDLER

    if (marked_content!=NULL)
      if (perform_fix) {
        PDEContainer container = PDSMCGetPDEContainer(marked_content);
        ASAtom mc_tag = PDEContainerGetMCTag(container);
        if (mc_tag != PDSElementGetType(element)) {
          PDEContainerSetMCTag(container, PDSElementGetType(element));
          PDPageSetPDEContent(page, 0);
        }
      }
      else {
        // just checking if the SE type matches the MC tag
        ASAtom mc_tag = PDEContainerGetMCTag(PDSMCGetPDEContainer(marked_content));
        //debug
        //char buf[1024];
        //sprintf(buf, "Problem. SE: %s --> MC: %s ", ASAtomGetString(PDSElementGetType(element)), ASAtomGetString(mc_tag));
        if (mc_tag != PDSElementGetType(element))
          to_ret = true; //stop processing the tree
      }

    if (pde_content != NULL)
      PDPageReleasePDEContent(page, 0);
    if (page != NULL)
      PDPageRelease(page);

    return to_ret;
  };

  return DoStructureTreeRoot(perform_fix, sync_se_and_mc);
}

//*****************************************************************************
bool DoClassMap(bool perform_fix) {
  PDDoc pd_doc = AVDocGetPDDoc(AVAppGetActiveDoc());

  PDSTreeRoot pds_tree_root = CosNewNull();
  if (!PDDocGetStructTreeRoot(pd_doc, &pds_tree_root))
    return false;
  PDSClassMap class_map;
  if (!PDSTreeRootGetClassMap(pds_tree_root, &class_map))
    return false;

  if (CosObjEnum(class_map, MyCosDictEnumProc, NULL))
    if (!perform_fix) return true; //stop processing the tree
    else PDSTreeRootRemoveClassMap(pds_tree_root);

  return false;
}

//*****************************************************************************
bool HasRoleMap() {
  PDDoc pd_doc = AVDocGetPDDoc(AVAppGetActiveDoc());
  PDSTreeRoot pds_tree_root;
  if (!PDDocGetStructTreeRoot(pd_doc, &pds_tree_root))
    return false;
  PDSRoleMap role_map;
  if (PDSTreeRootGetRoleMap(pds_tree_root, &role_map))
    return true;
  return false;
}

//*****************************************************************************
bool DoEmptyRoleMap(bool perform_fix) {
  PDDoc pd_doc = AVDocGetPDDoc(AVAppGetActiveDoc());
  PDSTreeRoot pds_tree_root = CosNewNull();
  if (!PDDocGetStructTreeRoot(pd_doc, &pds_tree_root))
    return false;
  PDSRoleMap role_map;
  if (!PDSTreeRootGetRoleMap(pds_tree_root, &role_map))
    return false;

  if (CosObjEnum(role_map, MyCosDictEnumProc, NULL))
    if (!perform_fix) return true; //stop processing the tree
    else PDSTreeRootRemoveRoleMap(pds_tree_root);

  return false;
}

//*****************************************************************************
bool DoUsedRoleMap(bool perform_fix) {
  ProcessStructureElementFunction roleMapUsage = [](bool perform_fix, PDSElement element,
    ASAtom kid_type, int kid_index, CosObj kid, PDSMCInfo mcid_info/*, PDSMC marked_content*/)
  {
    if (kid_type != ASAtomFromString("StructElem"))
      return false;

    PDDoc pd_doc = AVDocGetPDDoc(AVAppGetActiveDoc());
    PDSTreeRoot pds_tree_root = CosNewNull();
    if (!PDDocGetStructTreeRoot(pd_doc, &pds_tree_root))
      return false;
    PDSRoleMap role_map;
    if (!PDSTreeRootGetRoleMap(pds_tree_root, &role_map))
      return false;

    if (CosObjEnum(role_map, MyCosDictEnumProc, NULL)) //is empty
      return false;

    ASAtom standart_type = PDSRoleMapGetDirectMap(role_map, PDSElementGetType(kid));
    if (standart_type != ASAtomNull) //found in RoleMap
      return true;
    else
      if (perform_fix)
        PDSTreeRootRemoveRoleMap(pds_tree_root);

    return false;
  };
  return DoStructureTreeRoot(perform_fix, roleMapUsage);
}

//*****************************************************************************
bool DoIDTree(bool perform_fix) {
  PDDoc pd_doc = AVDocGetPDDoc(AVAppGetActiveDoc());

  PDSTreeRoot pds_tree_root = CosNewNull();
  if (!PDDocGetStructTreeRoot(pd_doc, &pds_tree_root))
    return false;
  CosObj idtree_obj = CosDictGet(pds_tree_root, ASAtomFromString("IDTree"));
  if (CosObjEqual(idtree_obj, CosNewNull()))
    return false;

  CosObj names = CosDictGet(idtree_obj, ASAtomFromString("Names"));
  if ((CosObjEnum(idtree_obj, MyCosDictEnumProc, NULL)) ||
    (CosObjGetType(names) == CosArray) && (CosArrayLength(names) == 0))
    if (!perform_fix) return true; //stop processing the tree
    else CosDictRemove(pds_tree_root, ASAtomFromString("IDTree"));

  return false;
}

//*****************************************************************************
bool DoAttributes(bool perform_fix) {
  ProcessStructureElementFunction attributes = [](bool perform_fix, PDSElement element,
    ASAtom kid_type, int kid_index, CosObj kid, PDSMCInfo mcid_info/*, PDSMC marked_content*/)
  {
    if (kid_type != ASAtomFromString("StructElem"))
      return false;

    CosObj kid_obj = PDSElementGetCosObj(kid);
    CosObj attr_obj = CosDictGet(kid_obj, ASAtomFromString("A"));
    if (CosObjEqual(attr_obj, CosNewNull()))
      return false;

    if (CosObjEnum(attr_obj, MyCosDictEnumProc, NULL))
      if (!perform_fix) return true; //stop processing the tree
      else CosDictRemove(kid_obj, ASAtomFromString("A"));

    return false;
  };
  return DoStructureTreeRoot(perform_fix, attributes);
}

//*****************************************************************************
bool DoTitleEntries(bool perform_fix) {
  ProcessStructureElementFunction titleEntries = [](bool perform_fix, PDSElement element,
    ASAtom kid_type, int kid_index, CosObj kid, PDSMCInfo mcid_info/*, PDSMC marked_content*/)
  {
    if (kid_type != ASAtomFromString("StructElem"))
      return false;

    CosObj kid_obj = PDSElementGetCosObj(kid);
    CosObj title_obj = CosDictGet(kid_obj, ASAtomFromString("T"));
    if (CosObjEqual(title_obj, CosNewNull()))
      return false;

    ASTCount count;
    CosStringValue(title_obj, &count);
    if (count == 0)
      if (!perform_fix) return true; //stop processing the tree
      else CosDictRemove(kid_obj, ASAtomFromString("T"));

    return false;
  };
  return DoStructureTreeRoot(perform_fix, titleEntries);
}

//*****************************************************************************
bool DoIDEntries(bool perform_fix) {
  ProcessStructureElementFunction ids = [](bool perform_fix, PDSElement element,
    ASAtom kid_type, int kid_index, CosObj kid, PDSMCInfo mcid_info/*, PDSMC marked_content*/)
  {
    if (kid_type != ASAtomFromString("StructElem"))
      return false;

    CosObj kid_obj = PDSElementGetCosObj(kid);
    CosObj id_obj = CosDictGet(kid_obj, ASAtomFromString("ID"));
    if (CosObjEqual(id_obj, CosNewNull()))
      return false;

    ASTCount count;
    CosStringValue(id_obj, &count);
    if (count == 0)
      if (!perform_fix) return true; //stop processing the tree
      else CosDictRemove(kid_obj, ASAtomFromString("ID"));

    return false;
  };
  return DoStructureTreeRoot(perform_fix, ids);
}

//*****************************************************************************
bool DoOutputIntents(bool perform_fix) {
  //*Remove Output Intents entry (OutputIntents dictionary from catalog is removed if exists)
  CosObj catalog = CosDocGetRoot(PDDocGetCosDoc(AVDocGetPDDoc(AVAppGetActiveDoc())));
  CosObj oi = CosDictGet(catalog, ASAtomFromString("OutputIntents"));
  if (CosObjEqual(oi, CosNewNull()))
    return false;

  if (!perform_fix) return true; //stop processing the tree
  else CosDictRemove(catalog, ASAtomFromString("OutputIntents"));

  return false;
}

//*****************************************************************************
bool DoAcroform(bool perform_fix) {
  //*Remove Acroform entry(if Fields entry in AcroFom is empty array, we remove whole AcroForm entry in catalog dictionary)
  CosObj catalog = CosDocGetRoot(PDDocGetCosDoc(AVDocGetPDDoc(AVAppGetActiveDoc())));
  CosObj acroform = CosDictGet(catalog, ASAtomFromString("AcroForm"));
  if (CosObjEqual(acroform, CosNewNull()))
    return false;

  CosObj fields = CosDictGet(acroform, ASAtomFromString("Fields"));
  if ((CosObjEnum(acroform, MyCosDictEnumProc, NULL)) ||
    (CosObjGetType(fields) == CosArray) && (CosArrayLength(fields) == 0))
    if (!perform_fix) return true; //stop processing the tree
    else CosDictRemove(catalog, ASAtomFromString("AcroForm"));

  return false;
}

//*****************************************************************************
bool DoOutlines(bool perform_fix) {
  //*Remove Outlines entry(if Count entry in Outlines is 0, we remove whole Outlines entry in catalog dictionary)
  CosObj catalog = CosDocGetRoot(PDDocGetCosDoc(AVDocGetPDDoc(AVAppGetActiveDoc())));
  CosObj outlines = CosDictGet(catalog, ASAtomFromString("Outlines"));
  if (CosObjEqual(outlines, CosNewNull()))
    return false;

  CosObj count = CosDictGet(outlines, ASAtomFromString("Count"));
  if ((CosObjEnum(outlines, MyCosDictEnumProc, NULL)) ||
    (CosObjGetType(count) == CosInteger) && (CosIntegerValue(count) == 0))
    if (!perform_fix) return true; //stop processing the tree
    else CosDictRemove(catalog, ASAtomFromString("Outlines"));

  return false;
}

//*****************************************************************************
bool DoExtensions(bool perform_fix) {
  //*Remove Extensions entry if empty
  CosObj catalog = CosDocGetRoot(PDDocGetCosDoc(AVDocGetPDDoc(AVAppGetActiveDoc())));
  CosObj extensions = CosDictGet(catalog, ASAtomFromString("Extensions"));
  if (CosObjEqual(extensions, CosNewNull()))
    return false;

  if (CosObjEnum(extensions, MyCosDictEnumProc, NULL))
    if (!perform_fix) return true; //stop processing the tree
    else CosDictRemove(catalog, ASAtomFromString("Extensions"));

  return false;
}

//*****************************************************************************
bool DoPageLayout(bool perform_fix) {
  //*Remove PageLayout entry if empty
  CosObj catalog = CosDocGetRoot(PDDocGetCosDoc(AVDocGetPDDoc(AVAppGetActiveDoc())));
  CosObj pageLayout = CosDictGet(catalog, ASAtomFromString("PageLayout"));
  if (CosObjEqual(pageLayout, CosNewNull()))
    return false;

  ASAtom layout_name = CosNameValue(pageLayout);
  if (ASAtomFromString("") == layout_name)
    if (!perform_fix) return true; //stop processing the tree
    else CosDictRemove(catalog, ASAtomFromString("PageLayout"));

  return false;
}

//*****************************************************************************
bool DoRedundantLangAttribute(bool perform_fix) {
  ProcessStructureElementFunction ids = [](bool perform_fix, PDSElement element,
    ASAtom kid_type, int kid_index, CosObj kid, PDSMCInfo mcid_info/*, PDSMC marked_content*/)
  {
    if (kid_type != ASAtomFromString("StructElem"))
      return false;

    CosObj catalog = CosDocGetRoot(PDDocGetCosDoc(AVDocGetPDDoc(AVAppGetActiveDoc())));
    CosObj catalog_lang = CosDictGet(catalog, ASAtomFromString("Lang"));

    if (CosObjEqual(catalog_lang, CosNewNull()))
      return false;

    ASTCount catalog_lang_len = 0;
    char* catalog_lang_str = CosStringValue(catalog_lang, &catalog_lang_len);
    if (catalog_lang_len <= 0)
      return false;

    CosObj kid_obj = PDSElementGetCosObj(kid);
    CosObj kid_lang = CosDictGet(kid_obj, ASAtomFromString("Lang"));
    if (CosObjEqual(kid_lang, CosNewNull()))
      return false;

    ASTCount kid_lang_len = 0;
    char* kid_lang_str = CosStringValue(kid_lang, &kid_lang_len);
    if (kid_lang_len <= 0)
      return false;

    //if (_stricmp(kid_lang_str, catalog_lang_str) == 0)
    if (strcmp(kid_lang_str, catalog_lang_str) == 0)
      if (!perform_fix) return true; //stop processing the tree
      else CosDictRemove(kid_obj, ASAtomFromString("Lang"));

    return false;
  };
  return DoStructureTreeRoot(perform_fix, ids);
}

bool DoActualTextNullTerminator(bool perform_fix) {
  ProcessStructureElementFunction actuals = [](bool perform_fix, PDSElement element,
    ASAtom kid_type, int kid_index, CosObj kid, PDSMCInfo mcid_info/*, PDSMC marked_content*/)
  {
    bool ret_val = false;

    if (kid_type != ASAtomFromString("StructElem"))
      return false;

    ASInt32 len = PDSElementGetActualText(kid, NULL);
    if (len <= 0)
      return false;

    ASUns8* actual_text = (ASUns8*)ASmalloc(len + 1);
    PDSElementGetActualText(kid, actual_text);
    if (len >= 2 && (actual_text[0] == 0xFF && actual_text[1] == 0xFE) ||
      (actual_text[0] == 0xFE && actual_text[1] == 0xFF))
    {
      if (actual_text[len - 1] == '\0' && actual_text[len - 2] == '\0')
        if (!perform_fix) ret_val = true; //stop processing the tree
        else if (len == 2)
        {
          CosObj kid_obj = PDSElementGetCosObj(kid);
          CosDictRemove(kid_obj, ASAtomFromString("ActualText"));
        }
        else PDSElementSetActualText(kid, actual_text, len - 2);
    }
    else if (actual_text[len - 1] == '\0')
    {
      if (!perform_fix) ret_val = true; //stop processing the tree
      else if (len == 1)
      {
        CosObj kid_obj = PDSElementGetCosObj(kid);
        CosDictRemove(kid_obj, ASAtomFromString("ActualText"));
      }
      else PDSElementSetActualText(kid, actual_text, len - 1);
    }
    ASfree(actual_text);
    return ret_val;
  };
  return DoStructureTreeRoot(perform_fix, actuals);
}

bool DoAlternateTextNullTerminator(bool perform_fix) {
  ProcessStructureElementFunction alternates = [](bool perform_fix, PDSElement element,
    ASAtom kid_type, int kid_index, CosObj kid, PDSMCInfo mcid_info/*, PDSMC marked_content*/)
  {
    bool ret_val = false;

    if (kid_type != ASAtomFromString("StructElem"))
      return false;

    ASInt32 len = PDSElementGetAlt(kid, NULL);
    if (len <= 0)
      return false;

    ASUns8* actual_text = (ASUns8*)ASmalloc(len + 1);
    PDSElementGetAlt(kid, actual_text);
    if (len >= 2 && (actual_text[0] == 0xFF && actual_text[1] == 0xFE) ||
      (actual_text[0] == 0xFE && actual_text[1] == 0xFF))
    {
      if (actual_text[len - 1] == '\0' && actual_text[len - 2] == '\0')
        if (!perform_fix) ret_val = true; //stop processing the tree
        else if (len == 2)
        {
          CosObj kid_obj = PDSElementGetCosObj(kid);
          CosDictRemove(kid_obj, ASAtomFromString("Alt"));
        }
        else PDSElementSetAlt(kid, actual_text, len - 2);
    }
    else if (actual_text[len - 1] == '\0')
    {
      if (!perform_fix) ret_val = true; //stop processing the tree
      else if (len == 1)
      {
        CosObj kid_obj = PDSElementGetCosObj(kid);
        CosDictRemove(kid_obj, ASAtomFromString("Alt"));
      }
      else PDSElementSetAlt(kid, actual_text, len - 1);
    }
    ASfree(actual_text);
    return ret_val;
  };
  return DoStructureTreeRoot(perform_fix, alternates);
}

//*****************************************************************************
void CleanViewerPreferences() {
  CosObj catalog = CosDocGetRoot(PDDocGetCosDoc(AVDocGetPDDoc(AVAppGetActiveDoc())));
  CosObj vp = CosDictGet(catalog, ASAtomFromString("ViewerPreferences"));

  if (CosObjEqual(vp, CosNewNull()))
    return;

  dictionary_keys viewer_preferences_keys;
  viewer_preferences_keys.to_keep = {
    ASAtomFromString("DisplayDocTitle")
  };

  CosObjEnum(vp, GetKeysToRemoveEnumProc, &viewer_preferences_keys);

  for (ASAtom key_to_remove : viewer_preferences_keys.to_remove)
  {
    CosDictRemove(vp, key_to_remove);
#ifndef MAC_PLATFORM
#ifdef DEBUG
    char msgbuf[256];
    sprintf_s(msgbuf, sizeof(msgbuf), "Removed from ViewerPreferences: %s\n", ASAtomGetString(key_to_remove));
    OutputDebugString(msgbuf);
#endif // DEBUG
#endif
  }
}

//*****************************************************************************
void CleanDocumentCatalog() {
  CosObj catalog = CosDocGetRoot(PDDocGetCosDoc(AVDocGetPDDoc(AVAppGetActiveDoc())));
  if (CosObjEqual(catalog, CosNewNull()))
    return;

  dictionary_keys catalog_keys;
  catalog_keys.to_keep = {
    ASAtomFromString("Lang"),
    ASAtomFromString("MarkInfo"),
    ASAtomFromString("Metadata"),
    ASAtomFromString("Pages"),
    ASAtomFromString("StructTreeRoot"),
    ASAtomFromString("Type"),
    ASAtomFromString("ViewerPreferences"),
  };

  CosObjEnum(catalog, GetKeysToRemoveEnumProc, &catalog_keys);

  for (ASAtom key_to_remove : catalog_keys.to_remove)
  {
    CosDictRemove(catalog, key_to_remove);
#ifndef MAC_PLATFORM
#ifdef _DEBUG
    char msgbuf[256];
    sprintf_s(msgbuf, sizeof(msgbuf), "Removed from Catalog: %s\n", ASAtomGetString(key_to_remove));
    OutputDebugString(msgbuf);
#endif // _DEBUG
#endif
  }
}