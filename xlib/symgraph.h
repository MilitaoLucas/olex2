/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "analysis.h"
#include "unitcell.h"

BeginXlibNamespace()

struct TSymmNode : public ACollectionItem {
  const TCAtom* atom;
  smatd matrix;
  vec3d crd;
  TPtrList<TSymmNode> children;

  TSymmNode(const TCAtom::Site& s)
    : atom(s.atom), matrix(s.matrix)
  {}

  TSymmNode(const TCAtom& a, const smatd& m)
    : atom(&a), matrix(m)
  {}

  void init() {
    crd = atom->GetParent()->Orthogonalise(matrix * atom->ccrd());
  }

  static uint64_t build_id(const TCAtom& a, uint32_t m_id) {
    return (((uint64_t)a.GetId()) << 32) | (uint64_t)m_id;
  }
  static uint64_t build_id(const TCAtom& a, const smatd& m) {
    return build_id(a, m.GetId());
  }
  static uint64_t build_id(const TCAtom& a) {
    return build_id(a, FirstMatrixRawId);
  }
  static uint64_t build_id(const TCAtom::Site& s) {
    return build_id(*s.atom, s.matrix);
  }
  uint64_t build_id() const {
    return build_id(*atom, matrix);
  }
};

struct TSymmNodeRegistry {
  mutable olx_pdict<uint64_t, TSymmNode*> registry;
  const TUnitCell& unit_cell;

  TSymmNodeRegistry(const TAsymmUnit& au);
  TSymmNodeRegistry(const TNetwork& net);

  ~TSymmNodeRegistry();

  TSymmNode* find(uint64_t id) const {
    return registry.Find(id, 0);
  }
  // def_tag - tag for the new node
  TSymmNode* find_or_add(const TSymmNode& parent, const TSymmNode& child,
    index_t def_tag = 0) const;

  olx_pair_t<TCAtom*, TCAtom::Site> remap(const TSymmNode& parent,
    const TSymmNode& child) const
  {
    if (parent.matrix.IsFirst()) {
      return olx_pair::make(const_cast<TCAtom*>(parent.atom),
        TCAtom::Site(const_cast<TCAtom*>(child.atom), child.matrix));
    }
    return olx_pair::make(const_cast<TCAtom*>(parent.atom),
      TCAtom::Site(const_cast<TCAtom*>(child.atom),
        unit_cell.MulMatrix(child.matrix, unit_cell.InvMatrix(parent.matrix))));
  }

  template <class Functor>
  void ForEach(const Functor& f) {
    for (size_t i = 0; i < registry.Count(); i++) {
      f.OnItem(registry.GetValue(i), i);
    }
  }
protected:
  void copy_au_(const TPtrList<TSymmNode>& nodes, TSymmNode& n, const TAsymmUnit& au);
  void copy_net_(const TPtrList<TSymmNode>& nodes, TSymmNode& n, const TNetwork& net);
};
EndXlibNamespace()
