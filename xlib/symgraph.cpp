/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "symgraph.h"

TSymmNodeRegistry::TSymmNodeRegistry(const TAsymmUnit& au)
  : unit_cell(au.GetLattice().GetUnitCell())
{
  TPtrList<TSymmNode> nodes(olx_reserve(au.AtomCount()));
  smatd I = smatd::Identity();
  I.SetRawId(FirstMatrixRawId);

  for (size_t i = 0; i < au.AtomCount(); i++) {
    const TCAtom& ca = au.GetAtom(i);
    if (ca.GetType().z < 1 || !ca.IsAvailable()) {
      nodes.Add(0);
      continue;
    }
    TSymmNode* n = new TSymmNode(ca, I);
    nodes.Add(n)->SetTag(i);
    nodes.GetLast()->init();
    registry.Add(n->build_id(), n);
  }
  for (size_t i = 0; i < nodes.Count(); i++) {
    if (nodes[i] != 0) {
      copy_au_(nodes, *nodes[i], au);
    }
  }
}
//..............................................................................
TSymmNodeRegistry::TSymmNodeRegistry(const TNetwork& net)
  : unit_cell(net.GetLattice().GetUnitCell())
{
  TPtrList<TSymmNode> nodes(olx_reserve(net.NodeCount()));
  for (size_t i = 0; i < net.NodeCount(); i++) {
    const TSAtom& sa = net.Node(i);
    if (sa.GetType().z < 1 || !sa.IsAvailable()) {
      nodes.Add(0);
      continue;
    }
    TSymmNode* n = new TSymmNode(sa.CAtom(), sa.GetMatrix());
    nodes.Add(n)->SetTag(i);
    nodes.GetLast()->init();
    registry.Add(n->build_id(), n);
  }
  for (size_t i = 0; i < nodes.Count(); i++) {
    if (nodes[i] != 0) {
      copy_net_(nodes, *nodes[i], net);
    }
  }
}
//..............................................................................
TSymmNodeRegistry::~TSymmNodeRegistry() {
  for (size_t i = 0; i < registry.Count(); i++) {
    delete registry.GetValue(i);
  }
}
//..............................................................................
TSymmNode* TSymmNodeRegistry::find_or_add(const TSymmNode& parent,
  const TSymmNode& child, index_t def_tag) const
{
  uint32_t m_id = unit_cell.MulMatrixId(child.matrix, parent.matrix);
  uint64_t key = TSymmNode::build_id(*child.atom, m_id);
  olx_pair_t<size_t, bool> ii = registry.AddEx(key);
  if (ii.b) {
    TSymmNode* sn = new TSymmNode(*child.atom,
      unit_cell.MulMatrix(child.matrix, parent.matrix));
    registry.GetValue(ii.a) = sn;
    sn->init();
    sn->SetTag(def_tag);
  }
  return registry.GetValue(ii.a);
}
//..............................................................................
void TSymmNodeRegistry::copy_au_(const TPtrList<TSymmNode>& nodes, TSymmNode& n,
  const TAsymmUnit& au)
{
  const TCAtom& ca = au.GetAtom(n.GetTag());
  for (size_t i = 0; i < ca.AttachedSiteCount(); i++) {
    const TCAtom::Site& s = ca.GetAttachedSite(i);
    if (s.atom->GetType().z < 1 || !s.atom->IsAvailable()) {
      continue;
    }
    TSymmNode* an = 0;
    if (s.matrix.IsFirst()) {
      an = nodes[s.atom->GetId()];
      if (an == 0) {
        continue;
      }
    }
    else {
      uint64_t id = TSymmNode::build_id(s);
      an = find(id);
      if (an == 0) {
        an = new TSymmNode(s);
        registry.Add(id, an)->init();
      }
    }
    n.children << an;
  }
}
//..............................................................................
void TSymmNodeRegistry::copy_net_(const TPtrList<TSymmNode>& nodes, TSymmNode& n,
  const TNetwork& net)
{
  const TSAtom& sa = net.Node(n.GetTag());
  for (size_t i = 0; i < sa.NodeCount(); i++) {
    const TSAtom& aa = sa.Node(i);
    if (aa.GetType().z < 1 || !aa.IsAvailable()) {
      continue;
    }
    
    TSymmNode* an = nodes[aa.GetIdInNetwork()];
    if (an == 0) {
      continue;
    }
    n.children << an;
  }
}
//..............................................................................
