/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_xfiles_H
#define __olx_xl_xfiles_H
#include "xbase.h"
#include "symmlib.h"
#include "refmodel.h"
#include "lattice.h"
BeginXlibNamespace()

class TBasicCFile : public ACollectionItem {
private:
  void PostLoad();
protected:
  olxstr FileName,  // file name if file is loaded
    Title;     // title of the file
  RefinementModel RefMod;
  TAsymmUnit AsymmUnit;
  // do not use it directly - use LoadStrings instead
  virtual void LoadFromStrings(const TStrList& Strings) = 0;
  void GenerateCellForCartesianFormat();
  virtual void SaveToStrings(TStrList& Strings) = 0;
public:
  TBasicCFile();
  virtual ~TBasicCFile();

  const TAsymmUnit& GetAsymmUnit() const { return AsymmUnit; }
  TAsymmUnit& GetAsymmUnit() { return AsymmUnit; }
  const RefinementModel& GetRM() const { return RefMod; }
  RefinementModel& GetRM() { return RefMod; }
  DefPropC(olxstr, Title)
  const olxstr& GetFileName() const { return FileName; }
  /* this function could be const, but many file handlers might do some
  preprocessing of changes before flushing...
  */
  virtual void SaveToFile(const olxstr& fileName);
  virtual void LoadFromFile(const olxstr& fileName);
  // default implementation read strings and calls LoadStrings
  virtual void LoadFromStream(IInputStream &is, const olxstr& nameToken);
  // name token can specify the dataset index or name
  void LoadStrings(const TStrList &lines,
    const olxstr &nameToken = EmptyString());
  // only oxm loader is native
  virtual bool IsNative() const { return false; }
  // adopts the content of the AsymmUnit to the virtual format
  virtual bool Adopt(class TXFile &, int flags = 0) = 0;
  virtual void RearrangeAtoms(const TSizeList & new_indices);
  // saves just title and file name
  virtual void ToDataItem(TDataItem& item);
  // loads only title and file name
  virtual void FromDataItem(const TDataItem& item);
};
//---------------------------------------------------------------------------

enum {
  XFILE_EVT_SG_Change,
  XFILE_EVT_UNIQ,
  XFILE_EVT_LAST
};

//---------------------------------------------------------------------------
class TXFile : public AEventsDispatcher {
private:
  TLattice Lattice;
  RefinementModel RefMod;
protected:
  TActionQList Actions;
  TStringToList<olxstr, TBasicCFile*> FileFormats;
  TBasicCFile *FLastLoader;
  TSpaceGroup* FSG;
  virtual bool Dispatch(int MsgId, short MsgSubId, const IOlxObject *Sender,
    const IOlxObject *Data, TActionQueue *);
  void ValidateTabs();
  void PostLoad(const olxstr &fn, TBasicCFile *loader, bool replicated);
  double CalcMass(const ContentList &) const;
  TDataItem const* data_source;
public:
  TXFile(ASObjectProvider& Objects);
  virtual ~TXFile();

  TActionQueue &OnFileLoad,
    &OnFileSave,
    &OnFileClose; // OnEnter, LastLoader is passed as Data

  /* takes over the object event queues
  */
  virtual void TakeOver(TXFile &f);

  const TLattice& GetLattice() const { return Lattice; }
  TLattice& GetLattice() { return Lattice; }
  TUnitCell& GetUnitCell() const { return Lattice.GetUnitCell(); }
  const RefinementModel& GetRM() const { return RefMod; }
  RefinementModel& GetRM() { return RefMod; }
  TAsymmUnit& GetAsymmUnit() const { return Lattice.GetAsymmUnit(); }
  /* a propper pointer, created with new should be passed
   the object will be deleted in the destructor !! */
  void RegisterFileFormat(TBasicCFile* F, const olxstr& Ext);

  virtual IOlxObject* Replicate() const;
  /* the space group is initialised upon file loading
   if the space group is unknow, TFunctionFailedException is thrown
  */
  TSpaceGroup& GetLastLoaderSG() const {
    if (FSG == 0) {
      throw TFunctionFailedException(__OlxSourceInfo, "unknown space group");
    }
    return *FSG;
  }
  // returns file loader associated with given file extension
  TBasicCFile* FindFormat(const olxstr& Ext);
  // returns a reference to the last loader (type safe)
  template <class LoaderClass>
  LoaderClass& GetLastLoader() const {
    if (FLastLoader == 0) {
      throw TFunctionFailedException(__OlxSourceInfo, "no last loader");
    }
    LoaderClass* rv = dynamic_cast<LoaderClass*>(FLastLoader);
    if (rv == 0) {
      throw TInvalidArgumentException(__OlxSourceInfo, "wrong last loader type");
    }
    return *rv;
  }
  /* this will use a loader from internal registry - not what is provided unless
  it is owned by this object. COuld also be used to reset the last loader to null
  */
  void SetLastLoader(const TBasicCFile* ll);
  // returns true if a file is loaded
  bool HasLastLoader() const { return FLastLoader != 0; }
  /* returns last loader object to access properties of the base class if type
  is not required. Use HasLastLoader to check if valid!
  */
  TBasicCFile* LastLoader() const {
    if (FLastLoader == 0) {
      throw TFunctionFailedException(__OlxSourceInfo, "no last loader");
    }
    return FLastLoader;
  }
  // locates related HKL file, processes raw or hkc file if necessary
  olxstr LocateHklFile();
  void UpdateAsymmUnit();
  /* Generic sort procedure, taking string instructions...
    instructions: Mw, Label, Label1, moiety size, weight, heaviest
  */
  void Sort(const TStrList& instructions, const TParamList &options);
  /* this function checks how the atom sequence will change when saving to a
  file and synchronise it with the atom Ids
  */
  void UpdateAtomIds();
  // nameToken is build is similar way to the NameArg!
  void LoadFromStream(IInputStream &is, const olxstr &nameToken);
  void LoadFromStrings(const TStrList& lines, const olxstr &nameToken);
  void LoadFromFile(const olxstr&FN);
  void SaveToFile(const olxstr &FN, int flags = 0);
  // clears the last loader and the model
  void Close();
  // returns last loaded file name (if any) or empty string
  const olxstr& GetFileName() const {
    return FLastLoader != 0 ? FLastLoader->GetFileName() : EmptyString();
  }
  /* returns 'file_path/olex2' if the folder does not exists - it tries to
  create one and may throw an exception if it fails.
  */
  olxstr GetStructureDataFolder() const;

  void EndUpdate();
  void LastLoaderChanged();  // performs complete reinitialisation

  const_strlist ToJSON() const;
  virtual void ToDataItem(TDataItem& item);
  virtual void FromDataItem(const TDataItem& item);
  /* this loads planes, should be called after all files have been
  loaded. For internal use
  */
  virtual void FinaliseFromDataItem_();

  void LibDataCount(const TStrObjList& Params, TMacroData& E);
  void LibCurrentData(const TStrObjList& Params, TMacroData& E);
  void LibDataName(const TStrObjList& Params, TMacroData& E);
  void LibGetFormula(const TStrObjList& Params, TMacroData& E);
  void LibSetFormula(const TStrObjList& Params, TMacroData& E);
  void LibEndUpdate(TStrObjList &Cmds, const TParamList &Options,
    TMacroData &E);
  void LibSaveSolution(const TStrObjList& Params, TMacroData& E);

  void LibGetMu(const TStrObjList& Params, TMacroData& E);
  void LibGetMass(const TStrObjList& Params, TMacroData& E);
  void LibGetF000(const TStrObjList& Params, TMacroData& E);
  void LibGetDensity(const TStrObjList& Params, TMacroData& E);

  void LibRefinementInfo(const TStrObjList& Params, TMacroData& E);
  void LibIncludedFiles(const TStrObjList& Params, TMacroData& E);

  TLibrary* ExportLibrary(const olxstr& name = EmptyString());

  struct VPtr : public olx_virtual_ptr<TXFile> {
    virtual IOlxObject *get_ptr() const;
  };
  // include leading '.'!
  static const olxstr& Olex2SameExt();

  /* describes a file name with which may carry reference to the dataset in the
  case of multiple-dataset files
  */
  struct NameArg {
    olxstr file_name;
    olxstr data_name;
    bool is_index;
    NameArg() : is_index(false) {}
    NameArg(const olxstr &name) { Parse(name); }
    NameArg &operator = (const olxstr &name) {
      Parse(name);
      return *this;
    }
    olxstr ToString() const;
    void Parse(const olxstr& fn);
  };
};
//---------------------------------------------------------------------------
EndXlibNamespace()
#endif
