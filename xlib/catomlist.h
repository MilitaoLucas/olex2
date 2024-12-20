/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_catom_list_H
#define __olx_catom_list_H
#include "residue.h"
#include "eset.h"
/*
Atom list to handle list of explicit (by label), implicit (last, first) and
expandable atom labels (C1_*, $C etc)
*/

BeginXlibNamespace()

class RefinementModel;
class ExplicitCAtomRef;

typedef TTypeListExt<ExplicitCAtomRef, IOlxObject> TAtomRefList;

class AAtomRef : public ACollectionItem {
public:
  virtual ~AAtomRef() {}
  // returns either an atom label or a string of C1_tol kind or $C
  virtual olxstr GetExpression(TResidue *r=0) const = 0;
  virtual bool IsExpandable() const = 0;
  virtual bool IsExplicit() const = 0;
  virtual size_t Expand(const RefinementModel& rm, TAtomRefList& res,
    TResidue& resi) const = 0;
  virtual AAtomRef* Clone(RefinementModel& rm) const = 0;
  virtual void ToDataItem(TDataItem &di) const = 0;
  virtual bool IsValid() const { return true; }
  virtual AAtomRef *ToImplicit(const olxstr &resi) const = 0;
  static AAtomRef &FromDataItem(const TDataItem &di, RefinementModel& rm);
};

// C1, C1_$2, C1_2 expressions handling
class ExplicitCAtomRef : public AAtomRef  {
  TCAtom *atom;
  const smatd* matrix;
  void DealWithSymm(const smatd* m);
public:
  ExplicitCAtomRef(const ExplicitCAtomRef& ar) :
    atom(ar.atom)
  {
    DealWithSymm(ar.matrix);
  }
  ExplicitCAtomRef(TCAtom& _atom, const smatd* _matrix=0) :
    atom(&_atom)
  {
    DealWithSymm(_matrix);
  }
  ExplicitCAtomRef(const TDataItem & di, RefinementModel& rm);
  ~ExplicitCAtomRef();
  virtual olxstr GetExpression(TResidue *) const;
  virtual bool IsExpandable() const { return false; }
  virtual bool IsExplicit() const { return true; }
  virtual bool IsValid() const;
  virtual size_t Expand(const RefinementModel &, TAtomRefList& res,
    TResidue &) const
  {
    res.Add(new ExplicitCAtomRef(*this));
    return 1;
  }
  virtual AAtomRef *ToImplicit(const olxstr &resi) const;
  TCAtom& GetAtom() const { return *atom; }
  const smatd* GetMatrix() const {  return matrix;  }
  void UpdateMatrix(const smatd *m);
  vec3d GetCCrd() const {
    return matrix == 0 ? atom->ccrd() : *matrix *atom->ccrd();
  }
  // works correctly only if the atom Id is smaller than 32 bit
  uint64_t GetHash() const {
    return ((uint64_t)atom->GetId() << 32) |
      (uint64_t)(matrix == 0 ? 0 : matrix->GetId());
  }
  /* builds instance from C1 or C1_$1 expression for given residue, may return
  NULL
  */
  static ExplicitCAtomRef* NewInstance(const RefinementModel& rm,
    const olxstr& exp, TResidue* resi);
  virtual AAtomRef* Clone(RefinementModel& rm) const;
  virtual void ToDataItem(TDataItem &di) const;
  int Compare(const ExplicitCAtomRef &r) const;
  static const olxstr &GetTypeId() {
    static olxstr t = "explicit";
    return t;
  }

  struct AtomAccessor {
    TCAtom& operator() (ExplicitCAtomRef& r) const { return r.GetAtom(); }
    const TCAtom& operator() (const ExplicitCAtomRef& r) const { return r.GetAtom(); }
  };
};

/*
Last - last atom of a residue,
First - first atom of a residue,
* - all non H atoms of a residue
$Type - all Type atoms
$Type_tol - all Type atoms of a residue
C1_tol - all explicit atoms of a residue
C1_+ - an explicit atom of next residue
C1_- - an explicit atom of previous residue
*/
class ImplicitCAtomRef : public AAtomRef  {
  olxstr Name;
  ExplicitCAtomRef* ref;
public:
  ImplicitCAtomRef(const olxstr& _Name)
    : Name(_Name),
    ref(0)
  {}
  // IntRef must be called after
  ImplicitCAtomRef(const TDataItem &di);
  ~ImplicitCAtomRef() {
    olx_del_obj(ref);
  }
  // * is special char
  virtual olxstr GetExpression(TResidue *) const {
    return Name == '*' ? EmptyString() : Name;
  }
  virtual bool IsExpandable() const { return true; }
  virtual bool IsExplicit() const { return false; }
  virtual size_t Expand(const RefinementModel& rm, TAtomRefList& res,
    TResidue& resi) const;
  void InitRef(const RefinementModel& rm, const olxstr& rname,
    TResidue *resi=0);
  void Update(const RefinementModel& rm);
  // may return 0
  static AAtomRef* NewInstance(const RefinementModel& rm, const olxstr& exp,
    const olxstr& resi, TResidue* _resi);
  virtual AAtomRef* Clone(RefinementModel& rm) const;
  virtual void ToDataItem(TDataItem &di) const;
  virtual AAtomRef *ToImplicit(const olxstr &resi) const;
  static const olxstr &GetTypeId() {
    static olxstr t = "implicit";
    return t;
  }
};

//manages C1 > C5 and C5 < C1 expressions
class ListAtomRef : public AAtomRef {
  AAtomRef &start, &end;
  olxstr op;
public:
  ListAtomRef(AAtomRef& _start, AAtomRef& _end, const olxstr& _op) :
    start(_start), end(_end), op(_op)
  {}
  ListAtomRef(const TDataItem &di, RefinementModel& rm);
  virtual ~ListAtomRef()  {
    delete &start;
    delete &end;
  }
  AAtomRef &GetStart() { return start;  }
  AAtomRef &GetEnd() { return end; }
  virtual bool IsExpandable() const { return true; }
  virtual bool IsExplicit() const {
    return start.IsExplicit() && end.IsExplicit();
  }
  virtual bool IsValid() const {
    return start.IsValid() && end.IsValid();
  }
  // * is special char
  virtual olxstr GetExpression(TResidue *r) const;
  virtual size_t Expand(const RefinementModel& rm, TAtomRefList& res,
    TResidue& resi) const;
  virtual AAtomRef* Clone(RefinementModel& rm) const {
    return new ListAtomRef(*start.Clone(rm), *end.Clone(rm), op);
  }
  virtual void ToDataItem(TDataItem &di) const;
  AAtomRef *ToImplicit(const olxstr &resi) const {
    return new ListAtomRef(*start.ToImplicit(resi),
      *end.ToImplicit(resi), op);
  }
  static const olxstr &GetTypeId() {
    static olxstr t = "list";
    return t;
  }
};

class AtomRefList {
  TTypeList<AAtomRef> refs;
  RefinementModel& rm;
  olxstr residue;
  olxstr expression;
  bool ContainsImplicitAtoms;
  olxstr BuildExpression(TResidue* r) const;
  void EnsureAtomGroups(const RefinementModel& rm, TAtomRefList& al,
    size_t groups_size) const;
  void EnsureAtomGroups(size_t group_size);
  static int RefIdCmp(const ExplicitCAtomRef& a, const ExplicitCAtomRef& b) {
    return olx_cmp(a.GetAtom().GetId(), b.GetAtom().GetId());
  }
  TTypeList<AAtomRef>& GetRefs() { return refs; }
  TPtrList<ImplicitCAtomRef>::const_list_type ExtractImplicit();
public:
  /* creates an instance of the object from given expression for given residue
  class, number or alias. Empty residue specifies the main residue.
  */
  AtomRefList(RefinementModel& rm, const olxstr& exp,
    const olxstr& resi = EmptyString());
  AtomRefList(RefinementModel& rm)
    : rm(rm), ContainsImplicitAtoms(false)
  {}
  /* expands the underlying expressions into a list. If the residue name is a
  class name (and there are several residues of the kind), there will be more
  than one entry in the res with each entry corresponding to any particular
  residue. One of the list type constants can be provided to validate the lists
  content to have pairs or triplets of atoms
  */
  TTypeList<TAtomRefList>& Expand(const RefinementModel& rm,
    TTypeList<TAtomRefList>& res, size_t group_size = InvalidSize) const;
  TTypeList<TAtomRefList>::const_list_type Expand(const RefinementModel& rm,
    size_t group_size = InvalidSize) const
  {
    TTypeList<TAtomRefList> res;
    return Expand(rm, res, group_size);
  }
  TAtomRefList::const_list_type ExpandList(const RefinementModel& rm,
    size_t group_size = InvalidSize) const;
  /* parses the expression into a list */
  void Build(const olxstr& exp, const olxstr& resi = EmptyString());
  /* if implicit is true - the residue class of the atoms is used to build an
  implicit atom list.
  */
  void Build(const TPtrList<class TSAtom>& atoms, bool implicit = false);
  /* recreates the expression for the object. If there are any explicit atom
  names - the new names will come from the updated model. Implicit atoms will
  stay as provided in the constructor
  */
  olxstr GetExpression() const;
  const olxstr& GetResi() const { return residue; }
  /* checks if any of the references are expandable */
  bool IsExpandable() const;
  bool IsValid() const;
  /* this can be used to decide if the atom list is valid */
  virtual bool IsExplicit() const;
  /* converts this list to list of exlicit references */
  AtomRefList& ConvertToExplicit();
  /* converts this list to list of implicit references */
  AtomRefList& ConvertToImplicit();
  void AddExplicit(TCAtom& a, const smatd* m = 0) {
    refs.Add(new ExplicitCAtomRef(a, m));
  }
  void AddAll(const TAtomRefList& list, bool same_rm);
  void AddExplicit(class TSAtom& a);
  // checks if all atoms are in the same RESI
  void UpdateResi();
  void InitImplicitRefs();
  void UpdateImplicitRefs();
  void Clear();
  bool IsEmpty() const { return refs.IsEmpty(); }
  size_t RefCount() const { return refs.Count(); }
  AtomRefList& Validate(size_t group_size = InvalidSize);
  void Assign(const AtomRefList& arl);
  void ToDataItem(TDataItem& di) const;
  void FromDataItem(const TDataItem& di);
  /* returns all explicit references */
  TPtrList<ExplicitCAtomRef>::const_list_type GetExplicit() const;
  void OnAUUpdate();
  /* sorts the references by atom tag */
  void SortByTag(const TPtrList<AtomRefList>& sync);
  void BeginAUSort();
  void EndAUSort(bool allow_implicit);
  /* sorts explicit refs according to atom tags, calls to ConvertToExplicit
  if needed
  */
  void SortExplicitRefs();

  static int RefTagCmp(const ExplicitCAtomRef& a, const ExplicitCAtomRef& b) {
    return olx_cmp(a.GetAtom().GetTag(), b.GetAtom().GetTag());
  }
};
EndXlibNamespace()

#endif
