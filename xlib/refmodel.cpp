/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "refmodel.h"
#include "lattice.h"
#include "symmparser.h"
#include "hkl.h"
#include "symmlib.h"
#include "pers_util.h"
#include "unitcell.h"
#include "xapp.h"
#include "refmerge.h"
#include "infotab.h"
#include "twinning.h"
#include "math/plane.h"
#include "ins.h"
#include "math/composite.h"
#include "estopwatch.h"
#include "encodings.h"

RefinementModel::RefinementModel(TAsymmUnit& au) :
  Omitted(*this),
  HklStatFileID(EmptyString(), 0, 0),
  HklFileID(EmptyString(), 0, 0),
  Vars(*this),
  rDFIX(*this, rltGroup2, "DFIX", rptValue|rptEsd, true),
  rDANG(*this, rltGroup2, "DANG", rptValue|rptEsd, true),
  rSADI(*this, rltGroup2, "SADI", rptEsd, true),
  rCHIV(*this, rltAtoms, "CHIV", rptValue|rptEsd, true),
  rFLAT(*this, rltGroup, "FLAT", rptEsd, true),
  rDELU(*this, rltAtoms, "DELU", rptEsd|rptEsd1, false),
  rSIMU(*this, rltAtoms, "SIMU", rptEsd|rptEsd1|rptValue1, false),
  rISOR(*this, rltAtoms, "ISOR", rptEsd, false),
  rEADP(*this, rltAtoms, "EADP", rptNone, true),
  rAngle(*this, rltGroup3, "olex2.restraint.angle", rptValue|rptEsd, true),
  rDihedralAngle(*this, rltGroup4, "olex2.restraint.dihedral", rptValue|rptEsd, true),
  rFixedUeq(*this, rltAtoms, "olex2.restraint.adp_u_eq", rptValue|rptEsd, false),
  rSimilarUeq(*this, rltAtoms, "olex2.restraint.adp_u_eq_similar", rptEsd, false),
  rSimilarAdpVolume(*this, rltAtoms, "olex2.restraint.adp_volume_similar", rptEsd, false),
  rRIGU(*this, rltAtoms, "RIGU", rptEsd|rptEsd1, false),
  ExyzGroups(*this),
  AfixGroups(*this),
  rSAME(*this),
  OnSetBadReflections(Actions.New("OnSetBadReflections")),
  OnCellDifference(Actions.New("OnCellDifference")),
  aunit(au),
  Conn(*this),
  CVars(*this)
{
  SetDefaults();
  RefContainers(rDFIX.GetIdName(), &rDFIX);
  RefContainers(rDANG.GetIdName(), &rDANG);
  RefContainers(TAsymmUnit::_GetIdName(), &aunit);
  rcRegister.Add(SharedRotatedADPs.GetName(), &SharedRotatedADPs);
  rcRegister.Add(Directions.GetName(), &Directions);
  rcRegister.Add(SameGroups.GetName(), &SameGroups);
  rcRegister.Add(cTLS.GetName(), &cTLS);
  rcList.Add(&Directions);
  rcList.Add(&SharedRotatedADPs);
  rcList.Add(&SameGroups);
  rcList1 << rDFIX <<rDANG << rSADI << rCHIV << rFLAT << rDELU
    << rSIMU << rISOR  << rEADP <<
    rAngle << rDihedralAngle << rFixedUeq << rSimilarUeq << rSimilarAdpVolume
    << rRIGU;
  //RefContainers(aunit.GetIdName(), &aunit);
  au.SetRefMod(this);
}
//.............................................................................
void RefinementModel::SetDefaults() {
  HKLF = 4;
  HKLF_s = def_HKLF_s;
  HKLF_mat.I();
  HKLF_wt = def_HKLF_wt;
  HKLF_wt = def_HKLF_wt;
  HKLF_m = def_HKLF_m;
  MERG = def_MERG;
  OMIT_s = def_OMIT_s;
  OMIT_2t = def_OMIT_2t;
  SHEL_hr = def_SHEL_hr;
  SHEL_lr = def_SHEL_lr;
  HKLF_set = MERG_set = OMIT_set = TWIN_set = SHEL_set = false;
  DEFS_set = false;
  DEFS << 0.02 << 0.1 << 0.01 << 0.04 << 1;
  TWIN_n = def_TWIN_n;
  TWIN_mat.I() *= -1;
}
//.............................................................................
void RefinementModel::Clear(uint32_t clear_mask) {
  for (size_t i = 0; i < SfacData.Count(); i++) {
    delete SfacData.GetValue(i);
  }
  SfacData.Clear();
  UserContent.Clear();
  RefineDisp.Clear();
  for (size_t i = 0; i < Frags.Count(); i++) {
    delete Frags.GetValue(i);
  }
  Frags.Clear();
  for (size_t i = 0; i < rcList1.Count(); i++) {
    rcList1[i]->Clear();
  }
  ExyzGroups.Clear();
  for (size_t i = 0; i < rcRegister.Count(); i++) {
    rcRegister.GetValue(i)->Clear();
  }
  if ((clear_mask & rm_clear_SAME) != 0) {
    rSAME.Clear();
  }
  else if ((clear_mask & rm_clear_ISAME) != 0) {
    TPtrList<TSameGroup> to_del;
    for (size_t i = 0; i < rSAME.Count(); i++) {
      if (!rSAME[i].GetAtoms().IsExplicit()) {
        to_del << rSAME[i];
      }
    }
    if (!to_del.IsEmpty()) {
      rSAME.Delete(to_del);
    }
  }
  //ExyzGroups.Clear();
  //AfixGroups.Clear();
  InfoTables.Clear();
  UsedSymm.Clear();
  used_weight.Clear();
  proposed_weight.Clear();
  RefinementMethod = "L.S.";
  SolutionMethod.SetLength(0);
  HKLSource.SetLength(0);
  ModelSource.SetLength(0);
  Omits.Clear();
  DEFS.Clear();
  SetDefaults();
  expl.Clear();
  Conn.Clear();
  PLAN.Clear();
  LS.Clear();
  Omitted.Clear();
  selectedTableRows.Clear();
  CVars.Clear();
  Vars.Clear();
  Vars.ClearBASF();
  Vars.ClearEXTI();
  if ((clear_mask & rm_clear_AFIX) != 0) {
    AfixGroups.Clear();
  }
  if ((clear_mask & rm_clear_VARS) != 0) {
    Vars.ClearAll();
  }
  if ((clear_mask & rm_clear_BadRefs) != 0) {
    BadReflections.Clear();
  }
}
//.............................................................................
void RefinementModel::ClearVarRefs() {
  for (size_t i = 0; i < RefContainers.Count(); i++) {
    IXVarReferencerContainer* rc = RefContainers.GetValue(i);
    for (size_t j = 0; j < rc->ReferencerCount(); j++) {
      IXVarReferencer& vr = rc->GetReferencer(j);
      for (size_t k = 0; k < vr.VarCount(); k++) {
        vr.SetVarRef(k, NULL);
      }
    }
  }
}
//.............................................................................
const smatd& RefinementModel::AddUsedSymm(const smatd& matr, const olxstr& id_)
{
  for (size_t i=0;  i < UsedSymm.Count(); i++) {
    if (UsedSymm.GetValue(i).symop == matr) {
      UsedSymm.GetValue(i).ref_cnt++;
      return UsedSymm.GetValue(i).symop;
    }
  }
  olxstr id = id_;
  if (id.IsEmpty()) {
    size_t idx = UsedSymm.Count();
    while (UsedSymm.HasKey(olxstr("$") << ++idx))
      ;
    id = (olxstr("S") << idx);
  }
  return UsedSymm.Add(id, RefinementModel::Equiv(matr), true).symop;
}
//.............................................................................
void RefinementModel::UpdateUsedSymm(const class TUnitCell& uc)  {
  try {
    for (size_t i = 0; i < UsedSymm.Count(); i++) {
      uc.InitMatrixId(UsedSymm.GetValue(i).symop);
    }
  }
  catch (const TExceptionBase &) {
    TBasicApp::NewLogEntry(logError) <<
      "Failed to update EQIV list, resetting to identity";
    TBasicApp::NewLogEntry(logError) <<
      "This could have happened as a result of the space group change";
    for (size_t i = 0; i < UsedSymm.Count(); i++) {
      UsedSymm.GetValue(i).symop = uc.GetMatrix(0);
    }
  }
}
//.............................................................................
void RefinementModel::RemUsedSymm(const smatd& matr) const {
  for (size_t i = 0; i < UsedSymm.Count(); i++) {
    if (UsedSymm.GetValue(i).symop == matr) {
      if (UsedSymm.GetValue(i).ref_cnt > 0) {
        UsedSymm.GetValue(i).ref_cnt--;
      }
      return;
    }
  }
  throw TInvalidArgumentException(__OlxSourceInfo, "matrix is not in the list");
}
//.............................................................................
size_t RefinementModel::UsedSymmIndex(const smatd& matr) const {
  for (size_t i = 0; i < UsedSymm.Count(); i++) {
    if (UsedSymm.GetValue(i).symop == matr) {
      return i;
    }
  }
  return InvalidIndex;
}
//.............................................................................
RefinementModel& RefinementModel::Assign(const RefinementModel& rm,
  bool AssignAUnit)
{
  Clear(rm_clear_ALL);
  expl = rm.expl;
  used_weight = rm.used_weight;
  proposed_weight = rm.proposed_weight;
  RefineDisp = rm.RefineDisp;
  LS = rm.LS;
  PLAN = rm.PLAN;
  HKLF = rm.HKLF;
  HKLF_s = rm.HKLF_s;
  HKLF_mat = rm.HKLF_mat;
  HKLF_wt = rm.HKLF_wt;
  HKLF_m = rm.HKLF_m;
  HKLF_set = rm.HKLF_set;
  MERG = rm.MERG;
  MERG_set = rm.MERG_set;
  OMIT_s = rm.OMIT_s;
  OMIT_2t = rm.OMIT_2t;
  OMIT_set = rm.OMIT_set;
  SHEL_lr = rm.SHEL_lr;
  SHEL_hr = rm.SHEL_hr;
  SHEL_set = rm.SHEL_set;
  Omits = rm.Omits;
  BadReflections = rm.BadReflections;
  TWIN_mat = rm.TWIN_mat;
  TWIN_n = rm.TWIN_n;
  TWIN_set = rm.TWIN_set;
  DEFS = rm.DEFS;
  DEFS_set = rm.DEFS_set;
  ModelSource = rm.ModelSource;
  HKLSource = rm.HKLSource;
  RefinementMethod = rm.RefinementMethod;
  SolutionMethod = rm.SolutionMethod;

  for (size_t i = 0; i < rm.Frags.Count(); i++) {
    Frags(rm.Frags.GetKey(i), new Fragment(*rm.Frags.GetValue(i)));
  }

  if (AssignAUnit) {
    aunit.Assign(rm.aunit);
  }

  /* need to copy the ID's before any restraints or info tabs use them or all
  gets broken... !!!  */
  for (size_t i = 0; i < rm.UsedSymm.Count(); i++) {
    AddUsedSymm(rm.UsedSymm.GetValue(i).symop, rm.UsedSymm.GetKey(i));
  }

  if (AssignAUnit || aunit.AtomCount() >= rm.aunit.AtomCount()) {
    for (size_t i = 0; i < rcList1.Count(); i++) {
      rcList1[i]->Assign(*rm.rcList1[i]);
    }

    rSAME.Assign(rm.rSAME);
    ExyzGroups.Assign(rm.ExyzGroups);
    AfixGroups.Assign(rm.AfixGroups);
    for (size_t i = 0; i < rcList.Count(); i++) {
      rcList[i]->Assign(*this, *rm.rcList[i]);
    }
    // restraints have to be copied first, as some may refer to vars
    Vars.Assign(rm.Vars);

    Conn.Assign(rm.Conn);
    aunit._UpdateConnInfo();

    for (size_t i = 0; i < rm.InfoTables.Count(); i++) {
      if (rm.InfoTables[i].IsValid()) {
        InfoTables.Add(new InfoTab(*this, rm.InfoTables[i]));
      }
    }
  }
  for (size_t i = 0; i < rm.SfacData.Count(); i++) {
    SfacData(rm.SfacData.GetKey(i), new XScatterer(*rm.SfacData.GetValue(i)));
  }
  UserContent = rm.UserContent;
  // check if all EQIV are used
  for (size_t i = 0; i < UsedSymm.Count(); i++) {
    if (UsedSymm.GetValue(i).ref_cnt == 0) {
      UsedSymm.Delete(i--);
    }
  }
  Omitted.Assign(rm.Omitted);
  selectedTableRows.Assign(rm.selectedTableRows, aunit);
  CVars.Assign(rm.CVars);
  return *this;
}
//.............................................................................
void RefinementModel::SetDEFS(const TStrList &df) {
  size_t mc = olx_min(df.Count(), DEFS.Count());
  for (size_t i = 0; i < mc; i++) {
    DEFS[i] = df[i].ToDouble();
  }
  DEFS_set = true;
}
//.............................................................................
TDoubleList::const_list_type RefinementModel::GetBASFAsDoubleList() const {
  TDoubleList rv;
  rv.SetCapacity(Vars.GetBASFCount());
  for (size_t bi = 0; bi < Vars.GetBASFCount(); bi++) {
    rv << Vars.GetBASF(bi).GetValue();
  }
  return rv;
}
//.............................................................................
TDoubleList::const_list_type RefinementModel::GetScales() const {
  TDoubleList rv;
  if (Vars.HasBASF()) {
    double pi = 0;  // 'prime' reflection fraction
    for (size_t bi = 0; bi < Vars.GetBASFCount(); bi++) {
      pi += Vars.GetBASF(bi).GetValue();
    }
    rv << 1 - pi << GetBASFAsDoubleList();
  }
  else {
    if (GetTWIN_n() != 0) {  // all the fractions are the same
      double f = 1. / olx_abs(GetTWIN_n());
      rv.SetCount(olx_abs(GetTWIN_n()));
      for (size_t i = 0; i < rv.Count(); i++) {
        rv[i] = f;
      }
    }
  }
  return rv;
}
olxstr RefinementModel::GetBASFStr() const {
  olxstr rv;
  for (size_t bi = 0; bi < Vars.GetBASFCount(); bi++) {
    rv << ' ' << Vars.GetBASF(bi).GetValue();
  }
  return rv.IsEmpty() ? rv : rv.SubStringFrom(1);
}
//.............................................................................
olxstr RefinementModel::GetDEFSStr() const {
  olxstr rv;
  for (size_t i = 0; i < DEFS.Count(); i++) {
    rv << ' ' << DEFS[i];
  }
  return rv.IsEmpty() ? rv : rv.SubStringFrom(1);
}
//.............................................................................
olxstr RefinementModel::GetTWINStr() const {
  olxstr rv;
  for (size_t i = 0; i < 9; i++) {
    if (TWIN_mat[i / 3][i % 3] == 0) {
      rv << "0 ";
    }
    else {
      rv << TWIN_mat[i / 3][i % 3] << ' ';
    }
  }
  return rv << TWIN_n;
}
//.............................................................................
void RefinementModel::SetIterations(int v) {
  if (LS.IsEmpty()) {
    LS.Add(v);
  }
  else {
    LS[0] = v;
  }
}
//.............................................................................
void RefinementModel::SetPlan(int v) {
  if (PLAN.IsEmpty()) {
    PLAN.Add(v);
  }
  else {
    PLAN[0] = v;
  }
}
//.............................................................................
void RefinementModel::AddSfac(XScatterer& sc) {
  const size_t i = SfacData.IndexOf(sc.GetLabel());
  if (i != InvalidIndex) {
    SfacData.GetEntry(i).val->Merge(sc);
    delete &sc;
  }
  else {
    SfacData.Add(sc.GetLabel(), &sc);
  }
}
//.............................................................................
InfoTab& RefinementModel::AddHTAB() {
  return InfoTables.Add(new InfoTab(*this, infotab_htab));
}
//.............................................................................
InfoTab& RefinementModel::AddRTAB(const olxstr& codename) {
  return InfoTables.Add(new InfoTab(*this, infotab_rtab, codename));
}
//.............................................................................
InfoTab& RefinementModel::AddCONF() {
  return InfoTables.Add(new InfoTab(*this, infotab_conf));
}
//.............................................................................
void RefinementModel::Validate() {
  for (size_t i = 0; i < rcList1.Count(); i++) {
    rcList1[i]->ValidateAll();
  }
  // this is to be done in reverse ordedr - Directions are first and if invalid must
  // removed first!!
  for (size_t i = rcList.Count(); i != 0; i--) {
    rcList[i - 1]->ValidateAll();
  }
  ExyzGroups.ValidateAll();
  AfixGroups.ValidateAll();
  Vars.Validate();
  for (size_t i = 0; i < InfoTables.Count(); i++) {
    if (!InfoTables[i].IsValid()) {
      InfoTables.Delete(i--);
    }
  }
}
//.............................................................................
bool RefinementModel::ValidateInfoTab(const InfoTab& it) {
  size_t it_ind = InvalidIndex;
  bool unique = true;
  for (size_t i = 0; i < InfoTables.Count(); i++) {
    if (&InfoTables[i] == &it) {
      it_ind = i;
    }
    else {
      if (unique && (InfoTables[i] == it)) {
        unique = false;
      }
    }
  }
  if (!unique || !it.IsValid()) {
    if (it_ind != InvalidIndex) {
      InfoTables.Delete(it_ind);
    }
    return false;
  }
  return true;
}
//.............................................................................
void RefinementModel::ClearInfoTab(const olxstr &name) {
  if (name.IsEmpty()) {
    InfoTables.Clear();
  }
  else {
    for (size_t i = 0; i < InfoTables.Count(); i++) {
      if (InfoTables[i].GetName() == name) {
        InfoTables.NullItem(i);
      }
    }
    InfoTables.Pack();
  }
}
//.............................................................................
void RefinementModel::AddInfoTab(const TStrList& l) {
  size_t atom_start = 1;
  size_t resi_ind = l[0].IndexOf('_');
  olxstr tab_name = (resi_ind == InvalidIndex ? l[0]
    : l[0].SubStringTo(resi_ind));
  olxstr resi_name = (resi_ind == InvalidIndex ? EmptyString()
    : l[0].SubStringFrom(resi_ind + 1));
  if (tab_name.Equalsi("HTAB")) {
    InfoTables.Add(
      new InfoTab(*this, infotab_htab, EmptyString()));
  }
  else if (tab_name.Equalsi("BOND")) {
    InfoTables.Add(
      new InfoTab(*this, infotab_bond, EmptyString()));
  }
  else if (tab_name.Equalsi("CONF")) {
    InfoTables.Add(
      new InfoTab(*this, infotab_conf, EmptyString()));
  }
  else if (tab_name.Equalsi("RTAB")) {
    InfoTables.Add(
      new InfoTab(*this, infotab_rtab, l[atom_start++]));
  }
  else if (tab_name.Equalsi("MPLA")) {
    if (l[atom_start].IsNumber()) {
      InfoTables.Add(
        new InfoTab(*this, infotab_mpla, l[atom_start + 1],
          l[atom_start].ToInt()));
      atom_start++;
    }
    else {
      InfoTables.Add(
        new InfoTab(*this, infotab_mpla));
    }
  }
  else {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "unknown information table name");
  }
  try {
    InfoTables.GetLast().FromExpression(l.Text(' ', atom_start), resi_name);
  }
  catch (const TExceptionBase& ex) {
    TBasicApp::NewLogEntry(logError) <<
      "Invalid info table atoms: " << l.Text(' ');
    TBasicApp::NewLogEntry(logError) << ex.GetException()->GetFullMessage();
    InfoTables.Delete(InfoTables.Count() - 1);
    return;
  }
  if (!InfoTables.GetLast().IsValid()) {
    TBasicApp::NewLogEntry(logError) <<
      "Invalid info table: " << l.Text(' ');
    if (!TIns::DoPreserveInvalid()) {
      InfoTables.Delete(InfoTables.Count() - 1);
    }
    return;
  }
  for (size_t i = 0; i < InfoTables.Count() - 1; i++) {
    if (InfoTables[i] == InfoTables.GetLast()) {
      TBasicApp::NewLogEntry(logError) <<
        "Duplicate info table: " << l.Text(' ');
      InfoTables.Delete(InfoTables.Count() - 1);
      return;
    }
  }
}
//.............................................................................
void RefinementModel::SetSHEL(const TStrList& shel) {
  if (shel.Count() > 0) {
    SHEL_lr = shel[0].ToDouble();
    if (shel.Count() > 1) {
      SHEL_hr = shel[1].ToDouble();
    }
    SHEL_set = true;
  }
}
//.............................................................................
void RefinementModel::Omit(const vec3i& r) {
  if (!Omits.Contains(r)) {
    Omits.AddCopy(r);
  }
}
//.............................................................................
void RefinementModel::AddOMIT(const TStrList& omit) {
  if (!omit.IsEmpty() && !omit[0].IsNumber()) {
    Omitted.Build(omit.Text(' '));
  }
  else if (omit.Count() >= 3) {  // reflection omit
    Omit(vec3i(omit[0].ToInt(), omit[1].ToInt(), omit[2].ToInt()));
  }
  else {  // reflection transformation/filtering
    if (omit.Count() > 0) {
      OMIT_s = omit[0].ToDouble();
    }
    if (omit.Count() > 1) {
      OMIT_2t = omit[1].ToDouble();
    }
    OMIT_set = !(OMIT_s == -2 && OMIT_2t == 180);
  }
}
//.............................................................................
void RefinementModel::DelOMIT(const TStrList& omit) {
  if (omit.Count() == 3) {
    Omits.Remove(vec3i(omit[0].ToInt(), omit[1].ToInt(), omit[2].ToInt()));
  }
}
//.............................................................................
double RefinementModel::FindRestrainedDistance(const TCAtom& a1,
  const TCAtom& a2)
{
  for(size_t i=0; i < rDFIX.Count(); i++ )  {
    TTypeList<ExplicitCAtomRef> ds = rDFIX[i].GetAtoms().ExpandList(*this, 2);
    for( size_t j=0; j < ds.Count(); j+=2 )  {
      if( (ds[j].GetAtom() == a1 && ds[j+1].GetAtom() == a2) ||
          (ds[j].GetAtom() == a2 && ds[j+1].GetAtom() == a1) )
      {
        return rDFIX[i].GetValue();
      }
    }
  }
  return -1;
}
//.............................................................................
void RefinementModel::SetModelSource(const olxstr &src) {
  ModelSource = src;
}
//.............................................................................
void RefinementModel::SetHKLSource(const olxstr &src) {
  HKLSource = src;
}
//.............................................................................
void RefinementModel::SetReflections(const TRefList &refs) const {
  TStopWatch sw(__FUNC__);
  if (HKLSource.IsEmpty()) {
    HklFileID.name = HKLSource;
    HklFileID.timestamp = TETime::msNow();
  }
  if (refs.IsEmpty()) {
    _Reflections.Clear();
    return;
  }
  _HklStat.FileMinInd = vec3i(100);
  _HklStat.FileMaxInd = vec3i(-100);
  _Reflections.Clear();
  _FriedelPairCount = 0;
  _Reflections.SetCapacity(refs.Count());
  const bool use_batch = HKLF >= 5;
  for (size_t i = 0; i < refs.Count(); i++) {
    if (refs[i].IsOmitted()) continue;
    TReflection& r = _Reflections.AddNew(refs[i]);
    if (HKLF == 3) {
      double F = r.GetI()*HKLF_s;
      double sF = r.GetS()*HKLF_s / HKLF_wt;
      r.SetI(F*F);
      r.SetS(2 * (olx_max(0.01, sF)*olx_max(olx_max(0.01, olx_abs(F)), sF)));
    }
    else {
      r.SetI(r.GetI()*HKLF_s);
      r.SetS(r.GetS()*HKLF_s / HKLF_wt);
    }
    vec3i::UpdateMinMax(r.GetHkl(), _HklStat.FileMinInd, _HklStat.FileMaxInd);
  }
  size_t maxRedundancy = 0;
  _Redundancy.Clear();

  sw.start("Building 3D reflection array");
  TArray3D<TRefPList *> hkl3d(
    _HklStat.FileMinInd, _HklStat.FileMaxInd);
  hkl3d.FastInitWith(0);
  for (size_t i = 0; i < _Reflections.Count(); i++) {
    TReflection &r = _Reflections[i];
    if (use_batch && r.GetBatch() <= 0) {
      continue;
    }
    TRefPList *& rl = hkl3d(r.GetHkl());
    if (rl == 0) {
      rl = new TRefPList();
    }
    rl->Add(r);
    if (rl->Count() > maxRedundancy) {
      maxRedundancy = rl->Count();
    }
  }
  sw.start("Analysing redundancy and Friedel pairs");
  _Redundancy.SetCount(maxRedundancy);
  _Redundancy.ForEach(olx_list_init::zero());
  for (int h = _HklStat.FileMinInd[0]; h <= _HklStat.FileMaxInd[0]; h++) {
    for (int k = _HklStat.FileMinInd[1]; k <= _HklStat.FileMaxInd[1]; k++) {
      for (int l = _HklStat.FileMinInd[2]; l <= _HklStat.FileMaxInd[2]; l++) {
        TRefPList* rl1 = hkl3d(h, k, l);
        if (rl1 == 0) {
          continue;
        }
        const vec3i ind(-h, -k, -l);
        if (hkl3d.IsInRange(ind)) {
          TRefPList* rl2 = hkl3d(ind);
          if (rl2 != 0 && rl2 != rl1) {
            _FriedelPairCount++;
            _Redundancy[rl2->Count() - 1]++;
            delete rl2;
            hkl3d(ind) = 0;
          }
        }
        _Redundancy[rl1->Count() - 1]++;
        delete rl1;
        hkl3d(h, k, l) = 0;
      }
    }
  }
}
//.............................................................................
const TRefList& RefinementModel::GetReflections() const {
  TStopWatch sw(__FUNC__);
  try {
    if (HKLSource.IsEmpty()) {
      return _Reflections;
    }
    TEFile::FileID hkl_src_id = TEFile::GetFileID(HKLSource);
    if( !_Reflections.IsEmpty() &&
        hkl_src_id == HklFileID &&
        HklFileMat == HKLF_mat)
    {
      return _Reflections;
    }
    HklFileID = hkl_src_id;
    THklFile hf(HKLF_mat);
    HklFileMat = HKLF_mat;
    olx_object_ptr<TIns> ins = hf.LoadFromFile(HKLSource, true);
    if (ins.ok()) {
      evecd cell = evecd::FromAny(
        Composite::Vector(vec3d_list() << aunit.GetAxes() <<
          aunit.GetAngles()));
      evecd esd = evecd::FromAny(
        Composite::Vector(vec3d_list() << aunit.GetAxisEsds() <<
          aunit.GetAngleEsds()));
      bool cell_changed = false;
      for (int i = 0; i < 3; i++) {
        if (olx_abs(aunit.GetAxes()[i] - ins->GetAsymmUnit().GetAxes()[i]) >
          olx_min(aunit.GetAxisEsds()[i], ins->GetAsymmUnit().GetAxisEsds()[i]))
        {
          cell_changed = true;
          break;
        }
        if (olx_abs(aunit.GetAngles()[i] - ins->GetAsymmUnit().GetAngles()[i]) >
          olx_min(aunit.GetAngleEsds()[i], ins->GetAsymmUnit().GetAngleEsds()[i]))
        {
          cell_changed = true;
          break;
        }
      }
      if (cell_changed) {
        OnCellDifference.Execute(this, &ins);
      }
      if (ins->GetRM().IsHKLFSet() && ins->GetRM().GetHKLF() != GetHKLF()) {
        HKLF = ins->GetRM().GetHKLF();
        if (ins->GetRM().Vars.GetBASFCount() != Vars.GetBASFCount()) {
          TStrList l(ins->GetRM().GetBASFStr(), ' ');
          // dirty tricks...
          const_cast<XVarManager &>(Vars).ClearBASF();
          const_cast<XVarManager &>(Vars).SetBASF(l);
        }
      }
    }
    else if (hf.GetHKLF() != -1) {
      HKLF = hf.GetHKLF();
    }
    SetReflections(hf.RefList());
    return _Reflections;
  }
  catch(TExceptionBase& exc) {
    _Reflections.Clear();
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
}
//.............................................................................
const RefinementModel::HklStat& RefinementModel::GetMergeStat() {
  // we need to take into the account MERG, HKLF and OMIT things here...
  TStopWatch sw(__FUNC__);
  try {
    GetReflections();
    bool update = (HklStatFileID != HklFileID || _HklStat.need_updating(*this));
    if (!update) {
      return _HklStat;
    }
    else  {
      completeness_cache.Clear();
      HklStatFileID = HklFileID;
      _HklStat.SetDefaults();
      TRefList refs;
      FilterHkl(refs, _HklStat);
      TRefPList non_overlapping_1, batch_1;
      if (HKLF >= 5) {
        if (!refs.IsEmpty() && !refs[0].IsBatchSet()) {
          TBasicApp::NewLogEntry(logWarning) << "HKL file is not compatible with"
            " the HKLF instruction - clearing BASF and resetting to HKLF 4";
          HKLF = 4;
          Vars.ClearBASF();
        }
      }
      if (HKLF >= 5) {
        non_overlapping_1 = GetNonoverlappingRefs(refs).obj()
          .Filter(olx_alg::olx_eq(1,
          FunctionAccessor::MakeConst(&TReflection::GetBatch)));
        batch_1.SetCapacity(refs.Count());
        for (size_t i = 0; i < refs.Count(); i++) {
          if (refs[i].GetBatch() >= 0) {
            _HklStat.DataCount++;
            if (refs[i].GetBatch() == 1) {
              batch_1.Add(refs[i]);
            }
          }
          refs[i].SetBatch(TReflection::NoBatchSet);
        }
      }
      TUnitCell::SymmSpace sp =
        aunit.GetLattice().GetUnitCell().GetSymmSpace();
      if (!refs.IsEmpty()) {
        bool mergeFP = (MERG == 4 || MERG == 3 || sp.IsCentrosymmetric());
        _HklStat = RefMerger::DryMerge<RefMerger::ShelxMerger>(
          sp, refs, (HKLF >= 5 ? vec3i_list() : Omits), mergeFP,
          2);
      }
      _HklStat.HKLF = HKLF;
      _HklStat.HKLF_mat = HKLF_mat;
      _HklStat.HKLF_m = HKLF_m;
      _HklStat.HKLF_s = HKLF_s;
      _HklStat.MERG = MERG;
      _HklStat.omits = Omits;
      if (refs.IsEmpty()) {
        return _HklStat;
      }
      sw.start("Analysing reflections: absent, completeness, limits");
      mat3d h2c = aunit.GetHklToCartesian();
      size_t e_cnt = 0;
      SymmSpace::InfoEx info_ex = SymmSpace::Compact(sp);
      info_ex.centrosymmetric = sp.IsCentrosymmetric();
      double min_ds_sq = olx_sqr(1. / _HklStat.MinD)+1e-5,
        max_ds_sq = olx_sqr(1. / _HklStat.MaxD)-1e-5;
      olx_pair_t<vec3i, vec3i> range = CalcIndicesToD(_HklStat.MinD, &info_ex);
      for (int h = range.a[0]; h <= range.b[0]; h++) {
        for (int k = range.a[1]; k <= range.b[1]; k++) {
          for (int l = range.a[2]; l <= range.b[2]; l++) {
            if (h == 0 && k == 0 && l == 0) {
              continue;
            }
            vec3i hkl(h,k,l);
            vec3i shkl = TReflection::Standardise(hkl, info_ex);
            if (shkl != hkl) {
              continue;
            }
            if (TReflection::IsAbsent(hkl, info_ex)) {
              continue;
            }
            double qd = TReflection::ToCart(hkl, h2c).QLength();
            if (qd <= min_ds_sq && qd >= max_ds_sq) {
              e_cnt++;
            }
          }
        }
      }
      if (HKLF >= 5) {
        if (!non_overlapping_1.IsEmpty()) {
          MergeStats st =
            RefMerger::DryMerge<RefMerger::ShelxMerger>(sp, non_overlapping_1,
              vec3i_list(), info_ex.centrosymmetric);
          _HklStat.Rint = st.Rint;
          _HklStat.Rsigma = st.Rsigma;
          _HklStat.InconsistentEquivalents = st.InconsistentEquivalents;
        }
        else {
          _HklStat.Rint = _HklStat.Rsigma = -1;
          _HklStat.InconsistentEquivalents = 0;
        }
        if (!batch_1.IsEmpty()) {
          MergeStats st =
            RefMerger::DryMerge<RefMerger::ShelxMerger>(sp, batch_1,
              vec3i_list(), info_ex.centrosymmetric);
          _HklStat.UniqueReflections = st.UniqueReflections;
        }
        _HklStat.ReflectionAPotMax = 0;
      }
      else {
        _HklStat.DataCount = _HklStat.UniqueReflections;
      }
     _HklStat.Completeness = double(_HklStat.UniqueReflections) / e_cnt;
     completeness_cache.Add(
       d_to_key(_HklStat.MinD, sp.IsCentrosymmetric()),
       _HklStat.Completeness);
    }
  }
  catch(const TExceptionBase& e) {
    _HklStat.SetDefaults();
    throw TFunctionFailedException(__OlxSourceInfo, e);
  }
  return _HklStat;
}
//.............................................................................
RefinementModel::HklStat& RefinementModel::FilterHkl(TRefList& out,
  RefinementModel::HklStat& stats) const
{
  TStopWatch sw(__FUNC__);
  const TRefList& all_refs = GetReflections();
  RefUtil::ResolutionAndSigmaFilter rsf(*this);
  rsf.SetStats(stats);
  const size_t ref_cnt = all_refs.Count();
  out.SetCapacity(ref_cnt);
  for (size_t i = 0; i < ref_cnt; i++) {
    size_t start = i;
    if (HKLF >= 5 && all_refs[i].GetBatch() < 0) {
      while (++i < ref_cnt && all_refs[i].GetBatch() < 0) {
        ;
      }
      i--;
    }
    bool add = true;
    for (size_t j = start; j <= i; j++) {
      if (all_refs[j].IsOmitted()) {
        stats.OmittedReflections++;
        add = false;
        break;
      }
      if (rsf.IsOutside(all_refs[j])) {
        add = false;
        break;
      }
      if (HKLF >= 5 && Omits.Contains(all_refs[j].GetHkl())) {
        add = false;
        break;
      }
    }
    if (add) {
      for (size_t j = start; j <= i; j++) {
        out.AddCopy(all_refs[j]);
      }
    }
  }
  stats.TotalReflections = out.Count();
  return stats;
}
//.............................................................................
TRefPList::const_list_type RefinementModel::GetNonoverlappingRefs(
  const TRefList& refs)
{
  TStopWatch sw(__FUNC__);
  TRefPList out;
  const size_t ref_cnt = refs.Count();
  out.SetCapacity(ref_cnt);
  for (size_t i = 0; i < ref_cnt; i++) {
    if (refs[i].GetBatch() >= 0 && (i == 0 || refs[i - 1].GetBatch() >= 0)) {
      out.Add(refs[i]);
    }
  }
  return out;
}
//.............................................................................
RefinementModel::HklStat& RefinementModel::AdjustIntensity(TRefList& out,
  RefinementModel::HklStat& stats) const
{
  const double h_o_s = 0.5*OMIT_s;
  const size_t ref_cnt = out.Count();
  for (size_t i = 0; i < ref_cnt; i++) {
    TReflection& r = out[i];
    if (r.GetI() < h_o_s*r.GetS()) {
      r.SetI(h_o_s*r.GetS());
      stats.IntensityTransformed++;
    }
  }
  return stats;
}
//.............................................................................
size_t RefinementModel::ProcessOmits(TRefList& refs) {
  if (Omits.IsEmpty())  return 0;
  size_t processed = 0;
  const size_t ref_c = refs.Count();
  for (size_t i = 0; i < ref_c; i++) {
    const TReflection& r = refs[i];
    const size_t omit_cnt = Omits.Count();
    for (size_t j = 0; j < omit_cnt; j++) {
      if (r.GetH() == Omits[j][0] &&
        r.GetK() == Omits[j][1] &&
        r.GetL() == Omits[j][2])
      {
        refs.NullItem(i);
        processed++;
        break;
      }
    }
  }
  if (processed != 0) {
    refs.Pack();
  }
  return processed;
}
//.............................................................................
void RefinementModel::DetwinAlgebraic(TRefList& refs, const HklStat& st,
  const SymmSpace::InfoEx& info_ex) const
{
  using namespace twinning;
  if (Vars.HasBASF()) {
    TDoubleList scales = GetScales();
    merohedral tw(info_ex, refs, scales,
      GetTWIN_mat().GetT(), GetTWIN_n());
    detwinner_algebraic dtw(scales);
    TRefList dtr;
    dtr.SetCapacity(refs.Count());
    for (size_t i = 0; i < refs.Count(); i++) {
      if (refs[i].GetTag() < 0) {
        continue;
      }
      const size_t s = dtr.Count();
      dtw.detwin(
        obs_twin_mate_generator<merohedral::iterator>(
          merohedral::iterator(tw, i), refs), dtr);
      for (size_t j = s; j < dtr.Count(); j++) {
        size_t ri = tw.find(dtr[j].GetHkl());
        if (ri == InvalidIndex) {
          continue;
        }
        TReflection& r = refs[ri];
        r.SetI(dtr[j].GetI());
        r.SetS(dtr[j].GetS());
        r.SetTag(-1);
      }
    }
  }
}
//.............................................................................
olxstr RefinementModel::AtomListToStr(const TTypeList<ExplicitCAtomRef> &al,
  size_t group_size, const olxstr &sep) const
{
  olxstr_buf rv;
  if (olx_is_valid_size(group_size)) {
    olxstr ss = '-';
    for (size_t i = 0; i < al.Count(); i += group_size) {
      for (size_t j = 0; j < group_size; j++) {
        if ((i + j) >= al.Count()) {
          rv << '?';
        }
        else {
          rv << al[i + j].GetExpression(0);
        }
        if (j + 1 < group_size) {
          rv << ss;
        }
      }
      if ((i + group_size) < al.Count()) {
        rv << sep;
      }
    }
  }
  else {
    for (size_t i = 0; i < al.Count(); i++) {
      rv << al[i].GetExpression(0);
      if ((i + 1) < al.Count()) {
        rv << sep;
      }
    }
  }
  return rv;
}
//.............................................................................
void formatRidingUHelper(olxstr &l,
  olxdict<const TCAtom *, TCAtomPList, TPointerComparator> &r)
{
  for (size_t j=0; j < r.Count(); j++) {
    if (l.Length() > 2) {
      l << ", ";
    }
    TCAtomPList &al = r.GetValue(j);
    if (al.Count() == 1) {
      l << al[0]->GetLabel() << " of " <<
        r.GetKey(j)->GetLabel();
    }
    else {
      l << '{';
      for (size_t k=0; k < al.Count(); k++) {
        l << al[k]->GetLabel();
        if ((k + 1) < al.Count()) {
          l << ',';
        }
      }
      l << "} of " << r.GetKey(j)->GetLabel();
    }
  }
}
const_strlist RefinementModel::Describe() {
  TStrList lst;
  Validate();
  int sec_num = 0;
  // twinning...
  if (Vars.HasBASF()) {
    lst.Add(olxstr(++sec_num)) << ". Twinned data refinement";
    double esd = 0, sum = 0;
    olxstr_buf basf;
    for (size_t i = 0; i < Vars.GetBASFCount(); i++) {
      sum += Vars.GetBASF(i).GetValue();
      esd += olx_sqr(Vars.GetBASF(i).GetEsd());
      basf << ' ' << Vars.GetBASF(i).ToString();
    }
    olxstr str_s = " Scales: ";
    str_s << TEValueD(1 - sum, sqrt(esd)).ToString();
    lst << str_s << olxstr(basf);
  }
  // riding atoms..
  olx_pdict<double, // scale
    olxdict<const TCAtom *, TCAtomPList, TPointerComparator> > riding_u;
  for (size_t i = 0; i < aunit.AtomCount(); i++) {
    TCAtom &a = aunit.GetAtom(i);
    if (a.GetUisoOwner() != NULL && !a.IsDeleted()) {
      riding_u.Add(a.GetUisoScale()).Add(a.GetUisoOwner()).Add(a);
    }
  }
  if (!riding_u.IsEmpty()) {
    olx_pdict<uint32_t, //low-to-high: 8 - bond count, 8 - riding z, 8 - pivot z
      olxdict<double,
      sorted::PointerPointer<const TCAtom>,
      TPrimitiveComparator> > riding_u_g;
    for (size_t i = 0; i < riding_u.Count(); i++) {
      for (size_t j = 0; j < riding_u.GetValue(i).Count(); j++) {
        TCAtomPList &al = riding_u.GetValue(i).GetValue(j);
        bool same_type = true;
        for (size_t k = 1; k < al.Count(); k++) {
          if (al[k]->GetType() != al[0]->GetType()) {
            same_type = false;
            break;
          }
        }
        if (!same_type) {
          continue;
        }
        uint32_t key1 = riding_u.GetValue(i).GetKey(j)->GetType().z << 16;
        key1 = key1 | (al[0]->GetType().z << 8) | (uint8_t)(al.Count());
        riding_u_g.Add(key1).Add(riding_u.GetKey(i)).AddUnique(
          riding_u.GetValue(i).GetKey(j));
      }
    }
    // eliminate groups
    for (size_t i = 0; i < riding_u_g.Count(); i++) {
      if (riding_u_g.GetValue(i).Count() != 1) {
        continue;
      }
      size_t idx = riding_u.IndexOf(riding_u_g.GetValue(i).GetKey(0));
      for (size_t j = 0; j < riding_u_g.GetValue(i).GetValue(0).Count(); j++) {
        riding_u.GetValue(idx).Remove(riding_u_g.GetValue(i).GetValue(0)[j]);
      }
      if (riding_u.GetValue(idx).IsEmpty()) {
        riding_u.Delete(idx);
      }
    }

    lst.Add(olxstr(++sec_num)) << ". Fixed Uiso";
    olx_pdict<double, TSizeList> gg;
    // groups first
    for (size_t i = 0; i < riding_u_g.Count(); i++) {
      if (riding_u_g.GetValue(i).Count() != 1) {
        continue;
      }
      gg.Add(riding_u_g.GetValue(i).GetKey(0)).Add(i);
    }
    for (size_t i = 0; i < gg.Count(); i++) {
      lst.Add(" At ") << gg.GetKey(i) << " times of:";
      olxstr &l = lst.Add("  ");
      for (size_t j = 0; j < gg.GetValue(i).Count(); j++) {
        uint32_t gk = riding_u_g.GetKey(gg.GetValue(i)[j]);
        cm_Element *e1 = XElementLib::FindByZ((gk & 0x00ff0000) >> 16);
        cm_Element *e2 = XElementLib::FindByZ((gk & 0x0000ff00) >> 8);
        size_t bonds = (gk & 0x000000ff);
        if (l.Length() > 2) {
          l << ", ";
        }
        l << "All " << e1->symbol << '(' << e2->symbol;
        for (size_t bi = 1; bi < bonds; bi++) {
          l << ',' << e2->symbol;
        }
        l << ") groups";
        size_t idx = riding_u.IndexOf(gg.GetKey(i));
        if (idx != InvalidIndex) {
          formatRidingUHelper(l, riding_u.GetValue(idx));
          riding_u.Delete(idx);
        }
      }
    }
    for (size_t i = 0; i < riding_u.Count(); i++) {
      lst.Add(" At ") << riding_u.GetKey(i) << " times of:";
      formatRidingUHelper(lst.Add("  "), riding_u.GetValue(i));
    }
  }
  // site related
  if (ExyzGroups.Count() != 0) {
    lst.Add(olxstr(++sec_num)) << ". Shared sites";
    for (size_t i = 0; i < ExyzGroups.Count(); i++) {
      TExyzGroup& sr = ExyzGroups[i];
      olxstr& str = lst.Add('{');
      for (size_t j = 0; j < sr.Count(); j++) {
        str << sr[j].GetLabel();
        if ((j + 1) < sr.Count()) {
          str << ", ";
        }
      }
      str << '}';
    }
  }
  if ((rDFIX.Count() | rDANG.Count() | rSADI.Count()) != 0) {
    TPtrList<TSRestraintList> ress;
    ress << rDFIX << rDANG << rSADI;
    lst.Add(olxstr(++sec_num)) << ". Restrained distances";
    for (size_t ri = 0; ri < ress.Count(); ri++) {
      TSRestraintList &res = *ress[ri];
      for (size_t i = 0; i < res.Count(); i++) {
        TSimpleRestraint& sr = res[i];
        TTypeList<TAtomRefList> atoms = sr.GetAtoms().Expand(*this, 2);
        for (size_t j = 0; j < atoms.Count(); j++) {
          lst.Add(' ') << AtomListToStr(atoms[j], 2,
            (&res == &rSADI ? " ~ " : " = "));
        }
        if (!atoms.IsEmpty()) {
          if (&res == &rSADI) {
            lst.Add(" with sigma of ") << sr.GetEsd();
          }
          else {
            lst.Add(" ") << sr.GetValue() << " with sigma of " << sr.GetEsd();
          }
        }
      }
    }
  }
  if (rAngle.Count() != 0) {
    lst.Add(olxstr(++sec_num)) << ". Restrained angles";
    for (size_t i = 0; i < rAngle.Count(); i++) {
      TSimpleRestraint& sr = rAngle[i];
      TTypeList<TAtomRefList> atoms = sr.GetAtoms().Expand(*this, 3);
      for (size_t j = 0; j < atoms.Count(); j++) {
        lst.Add(' ') << AtomListToStr(atoms[j], 3, ", ");
      }
      lst.Add(" fixed at ") << sr.GetValue() << " with sigma of " << sr.GetEsd();
    }
  }
  if (rDihedralAngle.Count() != 0) {
    lst.Add(olxstr(++sec_num)) << ". Restrained dihedral angles";
    for (size_t i = 0; i < rDihedralAngle.Count(); i++) {
      TSimpleRestraint& sr = rDihedralAngle[i];
      TTypeList<TAtomRefList> atoms = sr.GetAtoms().Expand(*this, 4);
      for (size_t j = 0; j < atoms.Count(); j++) {
        lst.Add(' ') << AtomListToStr(atoms[j], 4, ", ");
      }
      lst.Add(" fixed at ") << sr.GetValue() << " with sigma of " << sr.GetEsd();
    }
  }
  if (rCHIV.Count() != 0) {
    lst.Add(olxstr(++sec_num)) << ". Restrained atomic chiral volume";
    for (size_t i = 0; i < rCHIV.Count(); i++) {
      TSimpleRestraint& sr = rCHIV[i];
      TTypeList<TAtomRefList> atoms = sr.GetAtoms().Expand(*this);
      for (size_t j = 0; j < atoms.Count(); j++) {
        lst.Add(' ') << AtomListToStr(atoms[j], InvalidSize, ", ");
      }
      lst.Add(" fixed at ") << sr.GetValue() << " with sigma of " << sr.GetEsd();
    }
  }
  if (rFLAT.Count() != 0) {
    lst.Add(olxstr(++sec_num)) << ". Restrained planarity";
    for (size_t i = 0; i < rFLAT.Count(); i++) {
      TSimpleRestraint& sr = rFLAT[i];
      TTypeList<TAtomRefList> atoms = sr.GetAtoms().Expand(*this);
      for (size_t j = 0; j < atoms.Count(); j++) {
        lst.Add(' ') << AtomListToStr(atoms[j], InvalidSize, ", ");
      }
      lst.Add(" with sigma of ") << sr.GetEsd();
    }
  }
  // ADP related
  if (rDELU.Count() != 0) {
    lst.Add(olxstr(++sec_num)) << ". Rigid bond restraints";
    for (size_t i = 0; i < rDELU.Count(); i++) {
      TSimpleRestraint& sr = rDELU[i];
      if (sr.GetEsd() == 0 || sr.GetEsd1() == 0)  continue;
      if (sr.IsAllNonHAtoms())
        lst.Add(" All non-hydrogen atoms");
      else {
        TTypeList<TAtomRefList> atoms = sr.GetAtoms().Expand(*this);
        for (size_t j = 0; j < atoms.Count(); j++)
          lst.Add(' ') << AtomListToStr(atoms[j], InvalidSize, ", ");
      }
      lst.Add(" with sigma for 1-2 distances of ") << sr.GetEsd() <<
        " and sigma for 1-3 distances of " <<
        sr.GetEsd1();
    }
  }
  if ((rSIMU.Count() | rISOR.Count() | rEADP.Count() |
    rFixedUeq.Count() | rSimilarUeq.Count() | rSimilarAdpVolume.Count()) != 0)
  {
    lst.Add(olxstr(++sec_num)) << ". Uiso/Uaniso restraints and constraints";
    for (size_t i = 0; i < rSIMU.Count(); i++) {
      TSimpleRestraint& sr = rSIMU[i];
      olxstr& str = lst.Add(EmptyString());
      if (sr.IsAllNonHAtoms()) {
        str << "All non-hydrogen atoms" << ' ' << "have similar U";
      }
      else {
        str << AtomListToStr(sr.GetAtoms().ExpandList(*this), InvalidSize, " ~ ");
      }
      str << ": within " << sr.GetValue() << "A with sigma of " << sr.GetEsd() <<
        " and sigma for terminal atoms of " << sr.GetEsd1() << " within " <<
        sr.GetValue() << "A";
    }
    for (size_t i = 0; i < rISOR.Count(); i++) {
      TSimpleRestraint& sr = rISOR[i];
      olxstr& str = lst.Add(EmptyString());
      if (sr.IsAllNonHAtoms())
        str << "All non-hydrogen atoms" << ' ' << "restrained to be isotropic";
      else {
        TAtomRefList al = sr.GetAtoms().ExpandList(*this);
        for (size_t j = 0; j < al.Count(); j++) {
          if (al[j].GetAtom().GetEllipsoid() == NULL)  continue;
          str << "Uanis(" << al[j].GetExpression(NULL) << ") ~ Ueq";
          if ((j + 1) < al.Count())
            str << ", ";
        }
      }
      str << ": with sigma of " << sr.GetEsd() <<
        " and sigma for terminal atoms of " << sr.GetEsd1();
    }
    for (size_t i = 0; i < rEADP.Count(); i++) {
      TSimpleRestraint& sr = rEADP[i];
      olxstr& str = lst.Add(EmptyString());
      TAtomRefList al = sr.GetAtoms().ExpandList(*this);
      for (size_t j = 0; j < al.Count(); j++) {
        if (al[j].GetAtom().GetEllipsoid() == 0) {
          str << "Uiso(";
        }
        else {
          str << "Uanis(";
        }
        str << al[j].GetExpression(NULL) << ')';
        if ((j + 1) < al.Count()) {
          str << " = ";
        }
      }
    }
    for (size_t i = 0; i < rFixedUeq.Count(); i++) {
      TSimpleRestraint& sr = rFixedUeq[i];
      olxstr& str = lst.Add(EmptyString());
      TAtomRefList al = sr.GetAtoms().ExpandList(*this);
      for (size_t j = 0; j < al.Count(); j++) {
        str << "Ueq(" << al[j].GetExpression(NULL) << ')';
        if ((j + 1) < al.Count()) {
          str << ", ";
        }
      }
      str << ": fixed at " << sr.GetValue() << " with sigma of " << sr.GetEsd();
    }
    for (size_t i = 0; i < rSimilarUeq.Count(); i++) {
      TSimpleRestraint& sr = rSimilarUeq[i];
      olxstr& str = lst.Add(EmptyString());
      if (sr.IsAllNonHAtoms()) {
        str << "All non-hydrogen atoms" << ' ' << "have similar Ueq";
      }
      else {
        TAtomRefList al = sr.GetAtoms().ExpandList(*this);
        for (size_t j = 0; j < al.Count(); j++) {
          str << "Ueq(" << al[j].GetExpression(NULL) << ')';
          if ((j + 1) < al.Count()) {
            str << " ~ ";
          }
        }
      }
      str << ": with sigma of " << sr.GetEsd();
    }
    for (size_t i = 0; i < rSimilarAdpVolume.Count(); i++) {
      TSimpleRestraint& sr = rSimilarAdpVolume[i];
      olxstr& str = lst.Add(EmptyString());
      if (sr.IsAllNonHAtoms()) {
        str << "All non-hydrogen atoms" << ' ' << "have similar Uvol";
      }
      else {
        TAtomRefList al = sr.GetAtoms().ExpandList(*this);
        for (size_t j = 0; j < al.Count(); j++) {
          str << "Uvol(" << al[j].GetExpression(NULL) << ')';
          if ((j + 1) < al.Count()) {
            str << " ~ ";
          }
        }
      }
      str << ": with sigma of " << sr.GetEsd();
    }
  }
  if (rRIGU.Count() != 0) {
    lst.Add(olxstr(++sec_num)) << ". Rigid body (RIGU) restrains";
    for (size_t i = 0; i < rRIGU.Count(); i++) {
      TSimpleRestraint& sr = rRIGU[i];
      if (sr.GetEsd() == 0 || sr.GetEsd1() == 0) {
        continue;
      }
      if (sr.IsAllNonHAtoms())
        lst.Add(" All non-hydrogen atoms");
      else {
        TTypeList<TAtomRefList> atoms = sr.GetAtoms().Expand(*this);
        for (size_t j = 0; j < atoms.Count(); j++) {
          lst.Add(' ') << AtomListToStr(atoms[j], InvalidSize, ", ");
        }
      }
      lst.Add(" with sigma for 1-2 distances of ") << sr.GetEsd() <<
        " and sigma for 1-3 distances of " <<
        sr.GetEsd1();
    }
  }
  // fragment related
  if (rSAME.Count() != 0) {
    lst.Add(olxstr(++sec_num)) << ". Same fragment restrains";
    for (size_t i = 0; i < rSAME.Count(); i++) {
      TSameGroup& sg = rSAME[i];
      if (sg.DependentCount() == 0 || !sg.IsValidForSave() ||
        !sg.GetAtoms().IsExplicit())
      {
        continue;
      }
      TAtomRefList ratoms = sg.GetAtoms().ExpandList(*this);
      if (ratoms.IsEmpty()) {
        continue;
      }
      for (size_t j = 0; j < sg.DependentCount(); j++) {
        if (!sg.GetDependent(j).IsValidForSave()) {
          continue;
        }
        TAtomRefList atoms = sg.GetDependent(j).GetAtoms().ExpandList(*this);
        lst.Add('{') << AtomListToStr(atoms, InvalidSize, ", ") << '}' <<
          " sigma for 1-2: " << sg.GetDependent(j).Esd12 << ", 1-3: " <<
          sg.GetDependent(j).Esd13;
      }
      lst.Add("as in");
      lst.Add('{') << AtomListToStr(ratoms, InvalidSize, ", ") << '}';
    }
    for (size_t i = 0; i < rSAME.Count(); i++) {
      TSameGroup& sg = rSAME[i];
      if (sg.GetAtoms().IsExplicit() || !sg.IsValidForSave()) {
        continue;
      }
      TTypeList<TAtomRefList> atoms = sg.GetAtoms().Expand(*this);
      if (atoms.Count() < 2 || atoms[0].Count() < 2) {
        continue;
      }
      for (size_t j = 1; j < atoms.Count(); j++) {
        if (atoms[j].Count() != atoms[0].Count()) {
          continue;
        }
        lst.Add('{') << AtomListToStr(atoms[j], InvalidSize, ", ") << '}';
      }
      lst.Add("as");
      lst.Add('{') << AtomListToStr(atoms[0], InvalidSize, ", ") << '}' <<
        " sigma for 1-2: " << sg.Esd12 << " 1-3: " <<
        sg.Esd13;
    }
  }
  if (!SameGroups.items.IsEmpty()) {
    lst.Add(olxstr(++sec_num)) << ". Same fragment constrains";
    for (size_t i = 0; i < SameGroups.items.Count(); i++) {
      if (!SameGroups.items[i].IsValid()) {
        continue;
      }
      lst.Add(SameGroups.items[i].Describe());
    }
  }
  if (!SharedRotatedADPs.items.IsEmpty()) {
    lst.Add(olxstr(++sec_num)) << ". Shared rotated ADPs";
    for (size_t i = 0; i < SharedRotatedADPs.items.Count(); i++) {
      if (!SharedRotatedADPs.items[i].IsValid()) {
        continue;
      }
      lst.Add(SharedRotatedADPs.items[i].Describe());
    }
  }
  TStrList vars;
  Vars.Describe(vars);
  if (!vars.IsEmpty()) {
    lst.Add(++sec_num) << ". Others";
    lst.AddAll(vars);
  }
  size_t afix_sn = 0;
  olx_pdict<int, TPtrList<TAfixGroup> > a_gs;
  for (size_t i = 0; i < AfixGroups.Count(); i++) {
    if (!AfixGroups[i].IsEmpty()) {
      a_gs.Add(AfixGroups[i].GetAfix()).Add(AfixGroups[i]);
    }
  }
  sec_num++;
  for (size_t i = 0; i < a_gs.Count(); i++) {
    TPtrList<TAfixGroup>& gl = a_gs.GetValue(i);
    if (gl[0]->GetAfix() < 0) { // skip internals
      continue;
    }
    olxstr ag_name = gl[0]->Describe();
    if (!ag_name.IsEmpty()) {
      ag_name[0] = olxstr::o_toupper(ag_name.CharAt(0));
    }
    lst.Add(olxstr(sec_num) << '.' << (olxch)('a' + afix_sn++)) << ' ' <<
      ag_name << ':';
    olxstr& line = (lst.Add(' ') << gl[0]->ToString());
    for (size_t j = 1; j < gl.Count(); j++) {
      line << ", " << gl[j]->ToString();
    }
  }
  for (size_t i = 0; i < lst.Count(); i++) {
    size_t wsc = lst[i].LeadingCharCount(' ');
    if (wsc > 0 && lst[i].Length() > 80) {
      TStrList sl;
      sl.Hyphenate(lst[i], " \t,=+-*/", 80 - wsc, true);
      lst[i] = sl[0];
      for (size_t li = 1; li < sl.Count(); li++) {
        lst.Insert(++i, sl[li].Insert(' ', 0, wsc));
      }
    }
  }
  return lst;
}
//.............................................................................
void RefinementModel::ProcessFrags() {
  // generate missing atoms for the AFIX 59, 66
  olx_pdict<int, TPtrList<TAfixGroup> > a_groups;
  olx_pdict<int, Fragment*> frags;
  for (size_t i = 0; i < AfixGroups.Count(); i++) {
    TAfixGroup& ag = AfixGroups[i];
    int m = ag.GetM();
    if (!ag.IsFittedRing())  continue;
    if (m == 7)  m = 6;
    bool generate = false;
    for (size_t j = 0; j < ag.Count(); j++) {
      if (ag[j].ccrd().IsNull()) {
        generate = true;
        a_groups.Add(ag.GetAfix()).Add(ag)->SetAfix(m * 10 + ag.GetN());
        break;
      }
    }
    if (generate) {
      if (frags.IndexOf(m) == InvalidIndex) {
        vec3d_list crds;
        if (m == 5) {
          Fragment::GenerateFragCrds(frag_id_cp, crds);
        }
        else if (m == 6) {
          Fragment::GenerateFragCrds(frag_id_ph, crds);
        }
        else if (m == 10) {
          Fragment::GenerateFragCrds(frag_id_cp_star, crds);
        }
        else if (m == 11) {
          Fragment::GenerateFragCrds(frag_id_naphthalene, crds);
        }
        Fragment& f = AddFrag(m);
        const olxstr label("C");
        for (size_t i = 0; i < crds.Count(); i++) {
          f.Add(label, crds[i]);
        }
        frags.Add(m, &f);
      }
    }
  }
  for (size_t i = 0; i < Frags.Count(); i++) {
    Fragment* frag = Frags.GetValue(i);
    for (size_t j = 0; j < AfixGroups.Count(); j++) {
      TAfixGroup& ag = AfixGroups[j];
      if (ag.GetM() == frag->GetCode() && (ag.Count() + 1) == frag->Count()) {
        TTypeList<AnAssociation3<TCAtom*, const cm_Element*, bool> > atoms;
        vec3d_list crds;
        TCAtomPList all_atoms(ag.Count() + 1);
        all_atoms[0] = &ag.GetPivot();
        for (size_t k = 0; k < ag.Count(); k++) {
          all_atoms[k + 1] = &ag[k];
        }
        for (size_t k = 0; k < all_atoms.Count(); k++) {
          atoms.AddNew(all_atoms[k],
            (const cm_Element*)NULL, all_atoms[k]->ccrd().QLength() > 1e-6);
          crds.AddCopy((*frag)[k].crd);
        }
        aunit.FitAtoms(atoms, crds, false);
        ag.SetAfix(ag.GetN());
      }
    }
  }
  for (size_t i = 0; i < a_groups.Count(); i++) {
    TPtrList<TAfixGroup>& gs = a_groups.GetValue(i);
    for (size_t j = 0; j < gs.Count(); j++) {
      gs[j]->SetAfix(a_groups.GetKey(i));
    }
  }
  // remove the 'special' frags
  for (size_t i = 0; i < frags.Count(); i++) {
    const size_t ind = Frags.IndexOf(frags.GetKey(i));
    if (ind == InvalidIndex) {
      continue;  // ?
    }
    delete Frags.GetValue(ind);
    Frags.Delete(ind);
  }
}
//.............................................................................
void RefinementModel::ToDataItem(TDataItem& item) {
  // fields
  item.AddField("RefOutArg", PersUtil::NumberListToStr(PLAN))
    .AddField("Weight", PersUtil::NumberListToStr(used_weight))
    .AddField("ProposedWeight", PersUtil::NumberListToStr(proposed_weight))
    .AddField("ModelSrc", ModelSource)
    .AddField("HklSrc", HKLSource)
    .AddField("RefMeth", RefinementMethod)
    .AddField("SolMeth", SolutionMethod)
    .AddField("RefInArg", PersUtil::NumberListToStr(LS));
  if (!RefineDisp.IsEmpty()) {
    item.AddField("RefineDisp", RefineDisp.Text(','));
  }

  // save used equivalent positions
  TArrayList<uint32_t> mat_tags(UsedSymm.Count());
  TDataItem& eqiv = item.AddItem("EQIV");
  for (size_t i=0; i < UsedSymm.Count(); i++) {
    eqiv.AddItem(UsedSymm.GetKey(i),
      TSymmParser::MatrixToSymmEx(UsedSymm.GetValue(i).symop));
    mat_tags[i] = UsedSymm.GetValue(i).symop.GetId();
    UsedSymm.GetValue(i).symop.SetRawId((uint32_t)i);
  }

  Vars.ToDataItem(item.AddItem("LEQS"));
  expl.ToDataItem(item.AddItem("EXPL"));

  AfixGroups.ToDataItem(item.AddItem("AFIX"));
  ExyzGroups.ToDataItem(item.AddItem("EXYZ"));
  rSAME.ToDataItem(item.AddItem("SAME"));
  for( size_t i=0; i < rcList1.Count(); i++ )
    rcList1[i]->ToDataItem(item.AddItem(rcList1[i]->GetIdName()));
  for( size_t i=0; i < rcList.Count(); i++ )
    rcList[i]->ToDataItem(item.AddItem(rcList[i]->GetName()));

  item.AddItem("HKLF", HKLF)
    .AddField("s", HKLF_s)
    .AddField("wt", HKLF_wt)
    .AddField("m", HKLF_m)
    .AddField("mat", TSymmParser::MatrixToSymmEx(HKLF_mat));

  item.AddItem("OMIT", OMIT_set)
    .AddField("s", OMIT_s)
    .AddField("two_theta", OMIT_2t)
    .AddField("hkl", PersUtil::VecListToStr(Omits));
  item.AddItem("TWIN", TWIN_set).AddField("mat",
    TSymmParser::MatrixToSymmEx(TWIN_mat)).AddField("n", TWIN_n);
  item.AddItem("MERG", MERG_set).AddField("val", MERG);
  item.AddItem("SHEL", SHEL_set).AddField("high",
    SHEL_hr).AddField("low", SHEL_lr);
  Conn.ToDataItem(item.AddItem("CONN"));
  item.AddField("UserContent", GetUserContentStr());
  TDataItem& info_tables = item.AddItem("INFO_TABLES");
  size_t info_tab_cnt=0;
  for (size_t i=0; i < InfoTables.Count(); i++) {
    if (InfoTables[i].IsValid())
      InfoTables[i].ToDataItem(info_tables.AddItem("item"));
  }

  if (!SfacData.IsEmpty()) {
    TDataItem& sfacs = item.AddItem("SFAC");
    for (size_t i = 0; i < SfacData.Count(); i++) {
      SfacData.GetValue(i)->ToDataItem(sfacs);
    }
  }
  selectedTableRows.ToDataItem(item.AddItem("selected_cif_records"));
  if (CVars.Validate()) {
    CVars.ToDataItem(item.AddItem("to_calculate"), true);
  }
  // restore matrix tags
  for (size_t i = 0; i < UsedSymm.Count(); i++) {
    UsedSymm.GetValue(i).symop.SetRawId(mat_tags[i]);
  }
}
//.............................................................................
void RefinementModel::FromDataItem(TDataItem& item) {
  Clear(rm_clear_ALL);
  PersUtil::NumberListFromStr(item.GetFieldByName("RefOutArg"), PLAN);
  PersUtil::NumberListFromStr(item.GetFieldByName("Weight"), used_weight);
  PersUtil::NumberListFromStr(item.GetFieldByName("ProposedWeight"),
    proposed_weight);
  RefineDisp.Strtok(item.FindField("RefineDisp"), ',');
  ModelSource = item.FindField("ModelSrc");
  HKLSource = item.GetFieldByName("HklSrc");
  RefinementMethod = item.GetFieldByName("RefMeth");
  SolutionMethod = item.GetFieldByName("SolMeth");
  PersUtil::NumberListFromStr(item.GetFieldByName("RefInArg"), LS);

  TDataItem& eqiv = item.GetItemByName("EQIV");
  for (size_t i = 0; i < eqiv.ItemCount(); i++) {
    UsedSymm.Add(eqiv.GetItemByIndex(i).GetName()).symop =
      TSymmParser::SymmToMatrix(eqiv.GetItemByIndex(i).GetValue());
  }

  UpdateUsedSymm(aunit.GetLattice().GetUnitCell());
  expl.FromDataItem(item.GetItemByName("EXPL"));

  AfixGroups.FromDataItem(item.GetItemByName("AFIX"));
  ExyzGroups.FromDataItem(item.GetItemByName("EXYZ"));
  rSAME.FromDataItem(item.GetItemByName("SAME"));
  for (size_t i = 0; i < rcList1.Count(); i++)
    rcList1[i]->FromDataItem(item.FindItem(rcList1[i]->GetIdName()));
  for (size_t i = 0; i < rcList.Count(); i++)
    rcList[i]->FromDataItem(item.FindItem(rcList[i]->GetName()), *this);

  TDataItem& hklf = item.GetItemByName("HKLF");
  HKLF = hklf.GetValue().ToInt();
  HKLF_s = hklf.GetFieldByName("s").ToDouble();
  HKLF_wt = hklf.GetFieldByName("wt").ToDouble();
  HKLF_m = hklf.GetFieldByName("m").ToInt();
  HKLF_mat = TSymmParser::SymmToMatrix(hklf.GetFieldByName("mat")).r;

  TDataItem& omits = item.GetItemByName("OMIT");
  OMIT_set = omits.GetValue().ToBool();
  OMIT_s = omits.GetFieldByName("s").ToDouble();
  size_t tt_idx = omits.FieldIndex("two_theta");
  if (tt_idx == InvalidIndex)
    OMIT_2t = omits.GetFieldByName("2theta").ToDouble();
  else
    OMIT_2t = omits.GetFieldByIndex(tt_idx).ToDouble();
  PersUtil::VecListFromStr(omits.GetFieldByName("hkl"), Omits);

  TDataItem& twin = item.GetItemByName("TWIN");
  TWIN_set = twin.GetValue().ToBool();
  TWIN_mat = TSymmParser::SymmToMatrix(twin.GetFieldByName("mat")).r;
  TWIN_n = twin.GetFieldByName("n").ToInt();
  {
    TDataItem& merge = item.GetItemByName("MERG");
    MERG_set = merge.GetValue().ToBool();
    MERG = merge.GetFieldByName("val").ToInt();
  }
  {
    TDataItem* shel = item.FindItem("SHEL");
    if (shel != 0) {
      SHEL_set = shel->GetValue().ToBool();
      SHEL_lr = shel->GetFieldByName("low").ToDouble();
      SHEL_hr = shel->GetFieldByName("high").ToDouble();
    }
  }
  // restraints and BASF may use some of the vars...
  Vars.FromDataItem(item.GetItemByName("LEQS"));
  Conn.FromDataItem(item.GetItemByName("CONN"));
  SetUserFormula(item.FindField("UserContent"), false);

  TDataItem* info_tables = item.FindItem("INFO_TABLES");
  if (info_tables != 0) {
    for (size_t i = 0; i < info_tables->ItemCount(); i++)
      InfoTables.Add(new InfoTab(*this, info_tables->GetItemByIndex(i)));
  }
  TDataItem* sfac = item.FindItem("SFAC");
  if (sfac != 0) {
    for (size_t i = 0; i < sfac->ItemCount(); i++) {
      XScatterer* sc = new XScatterer(EmptyString());
      sc->FromDataItem(sfac->GetItemByIndex(i));
      SfacData.Add(sc->GetLabel(), sc);
    }
  }
  TDataItem *cif_sel = item.FindItem("selected_cif_records");
  if (cif_sel != 0) {
    selectedTableRows.FromDataItem(*cif_sel, aunit);
  }
  TDataItem *to_calc = item.FindItem("to_calculate");
  if (to_calc != 0) {
    CVars.FromDataItem(*to_calc, true);
  }
  aunit._UpdateConnInfo();
}
//.............................................................................
#ifdef _PYTHON
PyObject* RefinementModel::PyExport(bool export_conn) {
  PyObject* main = PyDict_New(),
    *hklf = PyDict_New(),
    *eq = PyTuple_New(UsedSymm.Count());
  TPtrList<PyObject> atoms, equivs;
  PythonExt::SetDictItem(main, "aunit", aunit.PyExport(atoms, export_conn));
  TArrayList<uint32_t> mat_tags(UsedSymm.Count());
  for (size_t i = 0; i < UsedSymm.Count(); i++) {
    smatd& m = UsedSymm.GetValue(i).symop;
    PyTuple_SetItem(eq, i,
      equivs.Add(
        Py_BuildValue("(iii)(iii)(iii)(ddd)", m.r[0][0], m.r[0][1], m.r[0][2],
          m.r[1][0], m.r[1][1], m.r[1][2],
          m.r[2][0], m.r[2][1], m.r[2][2],
          m.t[0], m.t[1], m.t[2]
        )));
    mat_tags[i] = m.GetId();
    m.SetRawId((uint32_t)i);
  }
  PythonExt::SetDictItem(main, "equivalents", eq);

  PythonExt::SetDictItem(main, "variables", Vars.PyExport(atoms));
  PythonExt::SetDictItem(main, "exptl", expl.PyExport());
  PythonExt::SetDictItem(main, "afix", AfixGroups.PyExport(atoms));
  PythonExt::SetDictItem(main, "exyz", ExyzGroups.PyExport(atoms));
  PythonExt::SetDictItem(main, "same", rSAME.PyExport(atoms, equivs));
  for (size_t i = 0; i < rcList1.Count(); i++) {
    PythonExt::SetDictItem(main, rcList1[i]->GetIdName().ToLowerCase(),
      rcList1[i]->PyExport(atoms, equivs));
  }
  for (size_t i = 0; i < rcList.Count(); i++) {
    PythonExt::SetDictItem(main, rcList[i]->GetName(), rcList[i]->PyExport());
  }

  PythonExt::SetDictItem(hklf, "value", Py_BuildValue("i", HKLF));
  PythonExt::SetDictItem(hklf, "s", Py_BuildValue("d", HKLF_s));
  PythonExt::SetDictItem(hklf, "m", Py_BuildValue("d", HKLF_m));
  PythonExt::SetDictItem(hklf, "wt", Py_BuildValue("d", HKLF_wt));
  PythonExt::SetDictItem(hklf, "matrix",
    Py_BuildValue("(ddd)(ddd)(ddd)", HKLF_mat[0][0], HKLF_mat[0][1], HKLF_mat[0][2],
      HKLF_mat[1][0], HKLF_mat[1][1], HKLF_mat[1][2],
      HKLF_mat[2][0], HKLF_mat[2][1], HKLF_mat[2][2]));
  if (HKLF > 4) {  // special case, twin entry also has BASF!
    PyObject* basf = PyTuple_New(Vars.GetBASFCount());
    for (size_t i = 0; i < Vars.GetBASFCount(); i++) {
      PyTuple_SetItem(basf, i, Py_BuildValue("d", Vars.GetBASF(i).GetValue()));
    }
    PythonExt::SetDictItem(hklf, "basf", basf);
  }
  PythonExt::SetDictItem(main, "hklf", hklf);
  {
    PyObject* uweight = PyTuple_New(used_weight.Count());
    PyObject* pweight = PyTuple_New(proposed_weight.Count());
    for (size_t i = 0; i < used_weight.Count(); i++)
      PyTuple_SetItem(uweight, i, Py_BuildValue("d", used_weight[i]));
    for (size_t i = 0; i < proposed_weight.Count(); i++)
      PyTuple_SetItem(pweight, i, Py_BuildValue("d", proposed_weight[i]));
    PythonExt::SetDictItem(main, "weight", uweight);
    PythonExt::SetDictItem(main, "proposed_weight", pweight);
  }
  {
    PyObject* omit;
    PythonExt::SetDictItem(main, "omit", omit = PyDict_New());
    PythonExt::SetDictItem(omit, "s", Py_BuildValue("d", OMIT_s));
    PythonExt::SetDictItem(omit, "2theta", Py_BuildValue("d", OMIT_2t));
    if (!Omits.IsEmpty()) {
      PyObject* omits = PyTuple_New(Omits.Count());
      for (size_t i = 0; i < Omits.Count(); i++) {
        PyTuple_SetItem(omits, i,
          Py_BuildValue("(iii)", Omits[i][0], Omits[i][1], Omits[i][2]));
      }
      PythonExt::SetDictItem(omit, "hkl", omits);
    }
    PythonExt::SetDictItem(main, "merge", Py_BuildValue("i", MERG));
  }
  if (TWIN_set) {
    PyObject* twin = PyDict_New(),
      *basf = PyTuple_New(Vars.GetBASFCount());
    PythonExt::SetDictItem(twin, "n", Py_BuildValue("i", TWIN_n));
    PythonExt::SetDictItem(twin, "matrix",
      Py_BuildValue("(ddd)(ddd)(ddd)", TWIN_mat[0][0], TWIN_mat[0][1], TWIN_mat[0][2],
        TWIN_mat[1][0], TWIN_mat[1][1], TWIN_mat[1][2],
        TWIN_mat[2][0], TWIN_mat[2][1], TWIN_mat[2][2]));
    for (size_t i = 0; i < Vars.GetBASFCount(); i++) {
      PyTuple_SetItem(basf, i, Py_BuildValue("d", Vars.GetBASF(i).GetValue()));
    }
    PythonExt::SetDictItem(twin, "basf", basf);
    PythonExt::SetDictItem(main, "twin", twin);
  }
  if (SHEL_set) {
    PyObject* shel;
    PythonExt::SetDictItem(main, "shel", shel = PyDict_New());
    PythonExt::SetDictItem(shel, "low", Py_BuildValue("d", SHEL_lr));
    PythonExt::SetDictItem(shel, "high", Py_BuildValue("d", SHEL_hr));
  }
  if (Vars.HasEXTI()) {
    PythonExt::SetDictItem(main, "exti",
      Py_BuildValue("f", Vars.GetEXTI().GetValue()));
  }

  PythonExt::SetDictItem(main, "conn", Conn.PyExport());

  if (!SfacData.IsEmpty()) {
    PyObject* sfac = PyDict_New();
    for (size_t i = 0; i < SfacData.Count(); i++) {
      PythonExt::SetDictItem(sfac, SfacData.GetKey(i).c_str(),
        SfacData.GetValue(i)->PyExport());
    }
    PythonExt::SetDictItem(main, "sfac", sfac);
  }

  size_t inft_cnt = 0;
  for (size_t i = 0; i < InfoTables.Count(); i++) {
    if (InfoTables[i].IsValid()) {
      inft_cnt++;
    }
  }

  PyObject* info_tabs = PyTuple_New(inft_cnt);
  if (inft_cnt > 0) {
    inft_cnt = 0;
    for (size_t i = 0; i < InfoTables.Count(); i++) {
      if (InfoTables[i].IsValid()) {
        PyTuple_SetItem(info_tabs, inft_cnt++, InfoTables[i].PyExport());
      }
    }
  }
  PythonExt::SetDictItem(main, "info_tables", info_tabs);

  // restore matrix tags
  for (size_t i = 0; i < UsedSymm.Count(); i++) {
    UsedSymm.GetValue(i).symop.SetRawId(mat_tags[i]);
  }
  if (!RefineDisp.IsEmpty()) {
    PyObject* disp = PyTuple_New(RefineDisp.Count());
    for (size_t i = 0; i < RefineDisp.Count(); i++) {
      PyTuple_SetItem(disp, i, PythonExt::BuildString(RefineDisp[i]));
    }
    PythonExt::SetDictItem(main, "refine_disp", disp);
  }
  return main;
}
#endif
//..............................................................................
bool RefinementModel::Update(const RefinementModel& rm) {
  if (aunit.GetAngles().DistanceTo(rm.aunit.GetAngles()) > 1e-6 ||
    aunit.GetAxes().DistanceTo(rm.aunit.GetAxes()) > 1e-6 ||
    Vars.VarCount() != rm.Vars.VarCount() ||
    Vars.GetBASFCount() != rm.Vars.GetBASFCount() ||
    aunit.EllpCount() != rm.aunit.EllpCount() ||
    Vars.VarCount() != rm.Vars.VarCount())
  {
    return false;
  }
  for (size_t i = 0; i < rm.aunit.AtomCount(); i++) {
    const TCAtom& a = rm.aunit.GetAtom(i);
    if (a.IsDeleted()) {
      continue;
    }
    TCAtom* this_a = 0;
    for (size_t j = 0; j < aunit.AtomCount(); j++) {
      if (aunit.GetAtom(j).GetLabel().Equalsi(a.GetLabel())) {
        this_a = &aunit.GetAtom(j);
        break;
      }
    }
    if (this_a == 0) {// new atom?
      this_a = &aunit.NewAtom();
      this_a->SetLabel(a.GetLabel(), false);
      this_a->SetType(a.GetType());
    }
    this_a->SetDeleted(false);
    this_a->ccrd() = a.ccrd();
    this_a->ccrdEsd() = a.ccrdEsd();
    this_a->SetOccu(a.GetOccu());
    this_a->SetOccuEsd(a.GetOccuEsd());
    this_a->SetUiso(a.GetUiso());
    this_a->SetUisoEsd(a.GetUisoEsd());
    this_a->SetQPeak(a.GetQPeak());
  }

  for (size_t i = 0; i < aunit.EllpCount(); i++) {
    aunit.GetEllp(i) = rm.aunit.GetEllp(i);
  }

  Vars.Assign(rm.Vars);
  used_weight = rm.used_weight;
  proposed_weight = rm.proposed_weight;
  for (size_t i = 0; i < Vars.VarCount(); i++) {
    Vars.GetVar(i).SetValue(rm.Vars.GetVar(i).GetValue());
  }
  // update Q-peak scale...
  aunit.InitData();
  return true;
}
//..............................................................................
olx_pair_t<vec3i, vec3i> RefinementModel::CalcIndicesToD(double d,
  const SymmSpace::InfoEx *si) const
{
  vec3i mx = CalcMaxHklIndexForD(d);
  return olx_pair::make(-mx, mx);
}
//..............................................................................
double RefinementModel::CalcCompletenessTo2Theta(double tt, bool Laue) {
  // this resets the cache if needed
  GetMergeStat();
  // check cache
  double two_sin_2t = 2 * sin(tt*M_PI / 360.0);
  double min_d = expl.GetRadiation() / (two_sin_2t == 0 ? 1e-6 : two_sin_2t);
  long key = d_to_key(min_d, Laue);
  {
    double v = completeness_cache.Find(key, -1.0);
    if (v > 0) {
      return v;
    }
  }
  TUnitCell::SymmSpace sp =
    aunit.GetLattice().GetUnitCell().GetSymmSpace();
  mat3d h2c = aunit.GetHklToCartesian();
  SymmSpace::InfoEx info_ex = SymmSpace::Compact(sp);
  if (Laue) {
    info_ex.centrosymmetric = true;
  }
  double min_ds_sq = olx_sqr(1.0 / min_d);
  SortedObjectList<vec3i, TComparableComparator> omits;
  for (size_t i = 0; i < Omits.Count(); i++) {
    if (HKLF <= 4) {
      omits.AddUnique(TReflection::Standardise(Omits[i], info_ex));
    }
  }
  TRefList refs_;
  TRefPList refs;
  HklStat st;
  FilterHkl(refs_, st);
  if (GetHKLF() >= 5) {
    for (size_t i = 0; i < refs_.Count(); i++) {
      if (refs_[i].GetBatch() == 1) {
        refs.Add(refs_[i]);
      }
    }
  }
  else {
    refs.AddAll(refs_);
  }
  for (size_t i = 0; i < refs.Count(); i++) {
    refs[i]->Standardise(info_ex);
  }
  QuickSorter::SortSF(refs, &TReflection::Compare);
  size_t u_cnt = 0;
  for (size_t i=0; i < refs.Count(); i++) {
    TReflection &r = *refs[i];
    bool skip = omits.Contains(r.GetHkl());
    while (++i < refs.Count() && r.CompareTo(*refs[i]) == 0) {
      ;
    }
    i--;
    if (skip || r.IsAbsent()) {
      continue;
    }
    double qd = r.ToCart(h2c).QLength();
    if (qd <= min_ds_sq) {
      u_cnt++;
    }
  }

  olx_pair_t<vec3i, vec3i> range = CalcIndicesToD(min_d);
  size_t e_cnt=0;
  for (int h = range.a[0]; h <= range.b[0]; h++) {
    for (int k = range.a[1]; k <= range.b[1]; k++) {
      for (int l = range.a[2]; l <= range.b[2]; l++) {
        if (l == 0 && k == 0 && h == 0) {
          continue;
        }
        vec3i hkl(h,k,l);
        vec3i shkl = TReflection::Standardise(hkl, info_ex);
        if (shkl != hkl) {
          continue;
        }
        if (TReflection::IsAbsent(hkl, info_ex)) {
          continue;
        }
        double qd = TReflection::ToCart(hkl, h2c).QLength();
        if (qd <= min_ds_sq) {
          e_cnt++;
        }
      }
    }
  }
  double rv = double(u_cnt) / (e_cnt);
  completeness_cache.Add(key, rv);
  return rv;
}
//..............................................................................
adirection& RefinementModel::DirectionById(const olxstr &id) const {
  for (size_t i = 0; i < Directions.items.Count(); i++) {
    if (Directions.items[i].id.Equalsi(id)) {
      return Directions.items[i];
    }
  }
  throw TInvalidArgumentException(__OlxSourceInfo, "direction ID");
}
//..............................................................................
adirection *RefinementModel::AddDirection(const TCAtomGroup &atoms, uint16_t type) {
  olxstr dname;
  if (type == direction_vector)
    dname << 'v';
  else if (type == direction_normal)
    dname << 'n';
  else {
    throw TInvalidArgumentException(__OlxSourceInfo,
      olxstr("direction type: ").quote() << type);
  }
  for (size_t i = 0; i < atoms.Count(); i++) {
    dname << atoms[i].GetFullLabel(*this);
  }
  for (size_t i = 0; i < Directions.items.Count(); i++) {
    if (Directions.items[i].id == dname) {
      return &Directions.items[i];
    }
  }
  return &Directions.items.Add(new direction(dname, atoms, type));
}
//..............................................................................
TSimpleRestraint & RefinementModel::SetRestraintDefaults(
  TSimpleRestraint &r) const
{
  const TSRestraintList& container = r.GetParent();
  if (container.GetIdName().Equals("DFIX")) {
    r.SetEsd(DEFS[0]);
  }
  else if (container.GetIdName().Equals("DANG")) {
    r.SetEsd(DEFS[0] * 2);
  }
  else if (container.GetIdName().Equals("SADI")) {
    r.SetEsd(DEFS[0]);
  }
  else if (container.GetIdName().Equals("CHIV")) {
    r.SetEsd(DEFS[1]);
  }
  else if (container.GetIdName().Equals("FLAT")) {
    r.SetEsd(DEFS[1]);
  }
  else if (container.GetIdName().Equals("DELU")) {
    r.SetEsd(DEFS[2]);
    r.SetEsd1(DEFS[2]);
  }
  else if (container.GetIdName().Equals("RIGU")) {
    r.SetEsd(0.004);
    r.SetEsd1(0.004);
  }
  else if (container.GetIdName().Equals("SIMU")) {
    r.SetEsd(DEFS[3]);
    r.SetEsd1(DEFS[3] * 2);
    r.SetValue(2);
  }
  else if (container.GetIdName().Equals("ISOR")) {
    r.SetEsd(0.1);
    r.SetEsd1(0.2);
  }
  else if (container.GetIdName().Equals("olex2.restraint.angle")) {
    r.SetEsd(0.02);
  }
  else if (container.GetIdName().Equals("olex2.restraint.dihedral")) {
    r.SetEsd(0.04);
  }
  else if (container.GetIdName().StartsFromi("olex2.restraint.adp")) {
    r.SetEsd(0.1);
  }
  return r;
}
//..............................................................................
bool RefinementModel::IsDefaultRestraint(const TSameGroup &r) const {
  return r.Esd12 == 0.02 && r.Esd13 == 0.02;
}
//..............................................................................
bool RefinementModel::IsDefaultRestraint(const TSimpleRestraint& r) const {
  const TSRestraintList& container = r.GetParent();
  if (container.GetIdName().Equals("DFIX")) {
    return r.GetEsd() == DEFS[0];
  }
  else if (container.GetIdName().Equals("DANG")) {
    return r.GetEsd() == DEFS[0] * 2;
  }
  else if (container.GetIdName().Equals("SADI")) {
    return r.GetEsd() == DEFS[0];
  }
  else if (container.GetIdName().Equals("CHIV")) {
    return r.GetEsd() == DEFS[1];
  }
  else if (container.GetIdName().Equals("FLAT")) {
    return r.GetEsd() == DEFS[1];
  }
  else if (container.GetIdName().Equals("DELU")) {
    return r.GetEsd() == DEFS[2] && r.GetEsd1() == DEFS[2];
  }
  else if (container.GetIdName().Equals("RIGU")) {
    return r.GetEsd() == 0.004 && r.GetEsd1() == 0.004;
  }
  else if (container.GetIdName().Equals("SIMU")) {
    return r.GetEsd() == DEFS[3] && r.GetEsd1() == DEFS[3] * 2 &&
      r.GetValue() == 2;
  }
  else if (container.GetIdName().Equals("ISOR")) {
    return r.GetEsd() == 0.1 && r.GetEsd1() == 0.2;
  }
  else if (container.GetIdName().Equals("olex2.restraint.angle")) {
    return r.GetEsd() == 0.02;
  }
  else if (container.GetIdName().Equals("olex2.restraint.dihedral")) {
    return r.GetEsd() == 0.04;
  }
  else if (container.GetIdName().StartsFromi("olex2.restraint.adp")) {
    return r.GetEsd() == 0.1;
  }
  return false;
}
//..............................................................................
bool RefinementModel::DoShowRestraintDefaults() const {
  try {
    static bool v = TBasicApp::GetInstance().GetOptions().FindValue(
      "preserve_restraint_defaults", FalseString()).ToBool();
    return v;
  }
  catch (const TExceptionBase &e) {
    e.GetException()->PrintStackTrace();
    return false;
  }
}
//..............................................................................
olxstr RefinementModel::WriteInsExtras(const TCAtomPList* atoms,
  bool write_internals) const
{
  TDataItem di(NULL, "root");
  typedef olx_pair_t<const TSRestraintList*, TIns::RCInfo> ResInfo;
  TTypeList<ResInfo> restraints;
  restraints.AddNew(&rAngle, TIns::RCInfo(1, 1, -1, true));
  restraints.AddNew(&rDihedralAngle, TIns::RCInfo(1, 1, -1, true));
  restraints.AddNew(&rFixedUeq, TIns::RCInfo(1, 1, -1, true));
  restraints.AddNew(&rSimilarUeq, TIns::RCInfo(0, 1, -1, false));
  restraints.AddNew(&rSimilarAdpVolume, TIns::RCInfo(0, 1, -1, false));
  TStrList rl;
  for (size_t i = 0; i < restraints.Count(); i++) {
    for (size_t j = 0; j < restraints[i].GetA()->Count(); j++) {
      olxstr line = TIns::RestraintToString(
        (*restraints[i].GetA())[j], restraints[i].GetB());
      if (!line.IsEmpty()) {
        rl.Add(line);
      }
    }
  }
  if (!rl.IsEmpty()) {
    TDataItem &ri = di.AddItem("restraints");
    for (size_t i = 0; i < rl.Count(); i++) {
      ri.AddItem("item", rl[i]);
    }
  }
  rl.Clear();
  for (size_t i = 0; i < rcList.Count(); i++) {
    rl << rcList[i]->ToInsList(*this);
  }
  if (write_internals) {
    bool has_int_groups = false;
    for (size_t i = 0; i < AfixGroups.Count(); i++) {
      if (AfixGroups[i].GetAfix() == -1 && !AfixGroups[i].IsEmpty()) {
        has_int_groups = true;
        break;
      }
    }
    if (has_int_groups) {
      for (size_t i = 0; i < AfixGroups.Count(); i++) {
        if (AfixGroups[i].GetAfix() == -1 && !AfixGroups[i].IsEmpty()) {
          olxstr line = "olex2.constraint.u_proxy ";
          line << AfixGroups[i].GetPivot().GetLabel();
          for (size_t j = 0; j < AfixGroups[i].Count(); j++) {
            if (AfixGroups[i][j].IsDeleted()) {
              continue;
            }
            line << ' ' << AfixGroups[i][j].GetLabel();
          }
          rl << line;
        }
      }
    }
  }
  if (!rl.IsEmpty()) {
    TDataItem &ri = di.AddItem("constraints");
    for (size_t i = 0; i < rl.Count(); i++) {
      ri.AddItem("item", rl[i]);
    }
  }
  olxstr fixed_types;
  for (size_t i = 0; i < aunit.AtomCount(); i++) {
    TCAtom &a = aunit.GetAtom(i);
    if (!a.IsDeleted() && a.IsFixedType()) {
      fixed_types << ' ' << a.GetResiLabel();
    }
  }
  if (!fixed_types.IsEmpty()) {
    di.AddItem("fixed_types", fixed_types.SubStringFrom(1));
  }
  {
    TDataItem *sci;
    selectedTableRows.ToDataItem(*(sci = &di.AddItem("selected_cif_records")));
    if (sci->ItemCount() == 0) {
      di.DeleteItem(sci);
    }
  }
  if (CVars.Validate()) {
    CVars.ToDataItem(di.AddItem("to_calculate"), false);
  }
  // write anharmonic ADP parts
  {
    TDataItem *aa = 0;
    for (size_t i = 0; i < aunit.AtomCount(); i++) {
      TCAtom &a = aunit.GetAtom(i);
      if (a.IsDeleted()) {
        continue;
      }
      if (a.GetEllipsoid() != 0 && a.GetEllipsoid()->IsAnharmonic()) {
        if (aa == 0) {
          aa = &di.AddItem("anharmonics");
        }
        TDataItem &ai = aa->AddItem(a.GetResiLabel());
        olxstr_buf tmp;
        const GramCharlier4 &ap = a.GetEllipsoid()->GetAnharmonicPart();
        for (size_t ci = 0; ci < ap.C.size(); ci++) {
          tmp << ' ' << olx_print("%.4le", ap.C[ci]);
        }
        ai.AddField("Cijk", olxstr(tmp).SubStringFrom(1));
        tmp.Clear();
        for (size_t ci = 0; ci < ap.D.size(); ci++) {
          tmp << ' ' << olx_print("%.4le", ap.D[ci]);
        }
        ai.AddField("Dijkl", olxstr(tmp).SubStringFrom(1));
      }
    }
  }

  di.AddItem("HklSrc").SetValue(
    olxstr('%') << encoding::percent::encode(HKLSource));
  if (!RefineDisp.IsEmpty()) {
    di.AddField("RefineDisp", RefineDisp.Text(','));
  }
  TEStrBuffer bf;
  di.SaveToStrBuffer(bf);
  return bf.ToString();
}
//..............................................................................
void RefinementModel::ReadInsExtras(const TStrList &items) {
  TDataItem di(0, EmptyString());
  di.LoadFromString(0, olxstr().Join(items), 0);
  TDataItem *restraints = di.FindItem("restraints");
  if (restraints != 0) {
    for (size_t i = 0; i < restraints->ItemCount(); i++) {
      TStrList toks(restraints->GetItemByIndex(i).GetValue(), ' ');
      if (!TIns::ParseRestraint(*this, toks)) {
        TBasicApp::NewLogEntry() << (olxstr(
          "Invalid Olex2 restraint: ").quote()
          << restraints->GetItemByIndex(i).GetValue());
      }
    }
  }
  TDataItem *constraints = di.FindItem("constraints");
  if (constraints != 0) {
    for (size_t i = 0; i < constraints->ItemCount(); i++) {
      TStrList toks(constraints->GetItemByIndex(i).GetValue(), ' ');
      IConstraintContainer *cc = rcRegister.Find(toks[0], 0);
      if (cc != 0) {
        cc->FromToks(toks.SubListFrom(1), *this);
      }
      else if (toks[0] == "olex2.constraint.u_proxy") {
        TCAtom *ca = aunit.FindCAtom(toks[1]);
        if (ca == 0) {
          TBasicApp::NewLogEntry() << (olxstr(
            "Invalid Olex2 constraint: ").quote()
            << constraints->GetItemByIndex(i).GetValue());
          continue;
        }
        if (ca->GetAfix() != 0)  // already set
          continue;
        TAfixGroup& ag = AfixGroups.New(ca, -1);
        for (size_t ti = 2; ti < toks.Count(); ti++) {
          ca = aunit.FindCAtom(toks[ti]);
          if (ca == 0) {
            TBasicApp::NewLogEntry(logError) << (olxstr(
              "Warning - possibly invalid Olex2 constraint: ").quote()
              << constraints->GetItemByIndex(i).GetValue());
            continue;
          }
          if (ca->GetAfix() == 0)
            ag.AddDependent(*ca);
        }
      }
      else {
        TBasicApp::NewLogEntry() << (olxstr(
          "Unknown Olex2 constraint: ").quote()
          << constraints->GetItemByIndex(i).GetValue());
      }
    }
  }
  TDataItem *fixed_types = di.FindItem("fixed_types");
  if (fixed_types != 0) {
    TStrList toks(fixed_types->GetValue(), ' ');
    for (size_t i = 0; i < toks.Count(); i++) {
      TCAtom *a = aunit.FindCAtom(toks[i]);
      if (a == 0) {
        TBasicApp::NewLogEntry(logError) <<
          (olxstr("Invalid fixed type atom name: ").quote() << toks[i]);
        continue;
      }
      a->SetFixedType(true);
    }
  }
  TDataItem *selected_cif_records = di.FindItem("selected_cif_records");
  if (selected_cif_records != NULL) {
    try {
      selectedTableRows.FromDataItem(*selected_cif_records, aunit);
    }
    catch (const TExceptionBase &e) {
      TBasicApp::NewLogEntry(logError) <<
        "While loading Selected CIF records: " <<
        e.GetException()->GetFullMessage();
    }
  }
  TDataItem *to_calc = di.FindItem("to_calculate");
  if (to_calc != 0) {
    try {
      CVars.FromDataItem(*to_calc, false);
    }
    catch (const TExceptionBase &e) {
      TBasicApp::NewLogEntry(logError) <<
        "While loading variables definitions: " <<
        e.GetException()->GetFullMessage();
    }
  }
  TDataItem *a_adp = di.FindAnyItem("anharmonics");
  // read anharmonic ADP parts
  if (a_adp != 0) {
    for (size_t i = 0; i < a_adp->ItemCount(); i++) {
      TDataItem &ai = a_adp->GetItemByIndex(i);
      TCAtom *a = aunit.FindCAtom(ai.GetName());
      if (a == 0 || a->GetEllipsoid() == 0) {
        TBasicApp::NewLogEntry(logError) << "Could not locate " <<
          ai.GetName() << " for the anharmonic contribution";
        continue;
      }
      TStrList c_toks(ai.FindField("Cijk"), " ");
      TStrList d_toks(ai.FindField("Dijkl"), " ");
      if (c_toks.Count() != 10 || d_toks.Count() != 15) {
        TBasicApp::NewLogEntry(logError) << "Ignoring invalid anharmonic ADP for "
          << ai.GetName();
        continue;
      }
      olx_object_ptr<GramCharlier4> ac(new GramCharlier4());
      for (size_t ci = 0; ci < 10; ci++) {
        ac->C[ci] = c_toks[ci].ToDouble();
      }
      for (size_t ci = 0; ci < 15; ci++) {
        ac->D[ci] = d_toks[ci].ToDouble();
      }
      a->GetEllipsoid()->SetAnharmonicPart(ac.release());
    }
  }
  TDataItem *hs = di.FindItem("HklSrc");
  if (hs != 0) {
    HKLSource = hs->GetValue();
    if (HKLSource.StartsFrom('%')) {
      HKLSource = encoding::percent::decode(HKLSource.SubStringFrom(1));
    }
  }
  RefineDisp.Clear();
  RefineDisp.Strtok(di.FindField("RefineDisp"), ',');
}
//..............................................................................
void RefinementModel::BeforeAUUpdate_() {
  if (!atom_refs.IsEmpty()) {
    TBasicApp::NewLogEntry(logError) << "Not clean operation";
    atom_refs.Clear();
  }
  TPtrList<ExplicitCAtomRef> rs;
  for (size_t i = 0; i < InfoTables.Count(); i++) {
    rs.AddAll(InfoTables[i].GetAtoms().GetExplicit().obj());
  }
  TPtrList<TSRestraintList> restraints = GetRestraints();
  for (size_t i = 0; i < restraints.Count(); i++) {
    for (size_t j = 0; j < restraints[i]->Count(); j++) {
      rs.AddAll((*restraints[i])[j].GetAtoms().GetExplicit());
    }
  }
  for (size_t i = 0; i < rs.Count(); i++) {
    atom_refs.Add(rs[i], rs[i]->GetCCrd());
  }
}
//..............................................................................
void RefinementModel::AfterAUUpdate_() {
  for (size_t i = 0; i < InfoTables.Count(); i++) {
    InfoTables[i].OnAUUpdate();
  }
  TPtrList<TSRestraintList> restraints = GetRestraints();
  for (size_t i = 0; i < restraints.Count(); i++) {
    restraints[i]->OnAUUpdate();
  }
  atom_refs.Clear();
}
//..............................................................................
void RefinementModel::BeforeAUSort_() {
  atom_ids.Clear();
  for (size_t i = 0; i < aunit.AtomCount(); i++) {
    atom_ids.Add(&aunit.GetAtom(i), aunit.GetAtom(i).GetId());
  }

  for (size_t i = 0; i < InfoTables.Count(); i++) {
    InfoTables[i].BeginAUSort();
  }
  TPtrList<TSRestraintList> restraints = GetRestraints();
  for (size_t i = 0; i < restraints.Count(); i++) {
    restraints[i]->BeginAUSort();
  }
  rSAME.BeginAUSort();
}
//..............................................................................
void RefinementModel::AfterAUSort_() {
  if (atom_ids.Count() != aunit.AtomCount()) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "atom count");
  }
  old_atom_ids.SetCount(atom_ids.Count());
  for (size_t i = 0; i < aunit.AtomCount(); i++) {
    old_atom_ids[i] = atom_ids[&aunit.GetAtom(i)];
  }

  for (size_t i = 0; i < InfoTables.Count(); i++) {
    InfoTables[i].EndAUSort();
  }
  TPtrList<TSRestraintList> restraints = GetRestraints();
  for (size_t i = 0; i < restraints.Count(); i++) {
    restraints[i]->EndAUSort();
  }
  
  rSAME.EndAUSort();
  old_atom_ids.Clear();
}
//..............................................................................
void RefinementModel::Sort_() {
  try {
    rSAME.SortGroupContent();
  }
  catch (const TExceptionBase &e) {
    TBasicApp::NewLogEntry(logError) << "Failed to sort SAME groups";
  }
  AfixGroups.SortGroupContent();
  rFLAT.SortAtomsByTags();
  rRIGU.SortAtomsByTags();
  rSIMU.SortAtomsByTags();
  rDELU.SortAtomsByTags();
}
  //..............................................................................
TPtrList<const TSRestraintList>::const_list_type
RefinementModel::GetRestraints() const
{
  TPtrList<const TSRestraintList> restraints;
  restraints << rDFIX << rDANG << rSADI << rCHIV << rFLAT << rDELU <<
    rSIMU << rAngle << rDihedralAngle << rFixedUeq << rSimilarUeq <<
    rSimilarAdpVolume << rRIGU;
  return restraints;
}
//..............................................................................
TPtrList<TSRestraintList>::const_list_type RefinementModel::GetRestraints() {
  TPtrList<TSRestraintList> restraints;
  restraints << rDFIX << rDANG << rSADI << rCHIV << rFLAT << rDELU <<
    rSIMU << rAngle << rDihedralAngle << rFixedUeq << rSimilarUeq <<
    rSimilarAdpVolume << rRIGU;
  return restraints;
}
//..............................................................................
void RefinementModel::SetHKLFString(const olxstr &str) {
  TStrList toks(olxstr(str).Replace('-', " -"), ' ');
  SetHKLF(toks);
}
//..............................................................................
RefinementModel::HklStat& RefinementModel::HklStat::operator = (
  const RefinementModel::HklStat& hs)
{
  MergeStats::operator = (hs);
  FileMinInd = hs.FileMinInd;
  FileMaxInd = hs.FileMaxInd;
  MaxD = hs.MaxD;         MinD = hs.MinD;
  OMIT_s = hs.OMIT_s;     OMIT_2t = hs.OMIT_2t;
  SHEL_lr = hs.SHEL_lr;   SHEL_hr = hs.SHEL_hr;
  LimDmin = hs.LimDmin;   LimDmax = hs.LimDmax;
  MaxI = hs.MaxI;
  MinI = hs.MinI;
  HKLF = hs.HKLF;
  HKLF_m = hs.HKLF_m;
  HKLF_s = hs.HKLF_s;
  HKLF_mat = hs.HKLF_mat;
  FilteredOff = hs.FilteredOff;
  IntensityTransformed = hs.IntensityTransformed;
  TotalReflections = hs.TotalReflections;
  OmittedReflections = hs.OmittedReflections;
  DataCount = hs.DataCount;
  MERG = hs.MERG;
  Completeness = hs.Completeness;
  omits = hs.omits;
  return *this;
}
//..............................................................................
void RefinementModel::HklStat::SetDefaults() {
  MergeStats::SetDefaults();
  MaxD = MinD = LimDmax = LimDmin = 0;
  MaxI = MinI = 0;
  HKLF_m = def_HKLF_m;
  HKLF_s = def_HKLF_s;
  HKLF_mat.I();
  HKLF = -1;
  FilteredOff = IntensityTransformed = OmittedByUser = 0;
  DataCount = TotalReflections = OmittedReflections = 0;
  MERG = def_MERG;
  OMIT_s = def_OMIT_s;
  OMIT_2t = def_OMIT_2t;
  SHEL_lr = def_SHEL_lr;
  SHEL_hr = def_SHEL_hr;
  Completeness = 0;
  omits.Clear();
}
//..............................................................................
bool RefinementModel::HklStat::need_updating(const RefinementModel &r) const {
  bool eq = r.OMIT_s == OMIT_s &&
    r.OMIT_2t == OMIT_2t &&
    r.SHEL_lr == SHEL_lr &&
    r.SHEL_hr == SHEL_hr &&
    r.HKLF == HKLF &&
    r.HKLF_m == HKLF_m &&
    r.HKLF_s == HKLF_s &&
    r.HKLF_mat == HKLF_mat &&
    r.MERG == MERG;
  if (!eq || omits.Count() != r.Omits.Count()) {
    return true;
  }
  for (size_t i = 0; i < omits.Count(); i++) {
    if (omits[i] != r.Omits[i]) {
      return true;
    }
  }
  return false;
}
//..............................................................................
olxstr RefinementModel::GetHKLFStr() const {
  olxstr rv(HKLF, 80);
  if (HKLF_m == def_HKLF_m) {
    if (HKLF_wt == def_HKLF_wt) {
      if (HKLF_mat.IsI()) {
        if (HKLF_s != def_HKLF_s) {
          rv << ' ' << HKLF_s;
        }
      }
      else {
        rv << ' ' << HKLF_s;
        for (int i = 0; i < 9; i++) {
          rv << ' ' << HKLF_mat[i / 3][i % 3];
        }
      }
    }
    else {
      rv << ' ' << HKLF_s;
      for (int i = 0; i < 9; i++) {
        rv << ' ' << HKLF_mat[i / 3][i % 3];
      }
      rv << ' ' << HKLF_wt;
    }
  }
  else {
    rv << ' ' << HKLF_s;
    for (int i = 0; i < 9; i++) {
      rv << ' ' << HKLF_mat[i / 3][i % 3];
    }
    rv << ' ' << HKLF_wt << ' ' << HKLF_m;
  }
  return rv;
}
//..............................................................................
RefinementModel::EXTI::Shelxl RefinementModel::GetShelxEXTICorrector() const {
  if (!Vars.HasEXTI()) {
    return EXTI::Shelxl(0, 0, mat3d());
  }
  return EXTI::Shelxl(expl.GetRadiation(),
    Vars.GetEXTI().GetValue(),
    aunit.GetHklToCartesian());
}
//..............................................................................
//..............................................................................
//..............................................................................
void RefinementModel::LibHasOccu(const TStrObjList& Params,
  TMacroData& E)
{
  bool has = false;
  for (size_t i = 0; i < aunit.AtomCount(); i++) {
    TCAtom &a = aunit.GetAtom(i);
    if (a.IsDeleted()) continue;
    XVarReference *vr = a.GetVarRef(catom_var_name_Sof);
    if (vr == NULL || vr->relation_type != relation_None ||
        olx_abs(a.GetChemOccu()-1) > 1e-3)
    {
      has = true;
      break;
    }
  }
  E.SetRetVal(has);
}
//..............................................................................
void RefinementModel::LibOSF(const TStrObjList& Params, TMacroData& E) {
  if (Params.IsEmpty()) {
    E.SetRetVal(Vars.VarCount() == 0 ? 0.0 : Vars.GetVar(0).GetValue());
  }
  else {
    if (Vars.VarCount() == 0) {
      Vars.NewVar(Params[0].ToDouble());
    }
    else {
      Vars.GetVar(0).SetValue(Params[0].ToDouble());
    }
  }
}
//..............................................................................
void RefinementModel::LibFVar(const TStrObjList& Params, TMacroData& E)  {
  size_t i = Params[0].ToSizeT();
  if (Vars.VarCount() <= i) {
    E.ProcessingError(__OlxSrcInfo, "FVar index out of bounds");
    return;
  }
  if (Params.Count() == 1)
    E.SetRetVal(Vars.GetVar(i).GetValue());
  else {
    Vars.GetVar(i).SetValue(Params[1].ToDouble());
    if (Params.Count() == 3)
      Vars.GetVar(i).SetEsd(Params[2].ToDouble());
  }
}
//..............................................................................
void RefinementModel::LibBASF(const TStrObjList& Params, TMacroData& E)  {
  size_t i = Params[0].ToSizeT();
  if (Vars.GetBASFCount() <= i) {
    E.ProcessingError(__OlxSrcInfo, "BASF index out of bounds");
    return;
  }
  if (Params.Count() == 1) {
    E.SetRetVal(Vars.GetBASF(i).ToString());
  }
  else {
    TEValueD v = Params[1];
    if (Params.Count() == 3) {
      v.E() = Params[2].ToDouble();
    }
    Vars.GetBASF(i).Update(v);
  }
}
//..............................................................................
void RefinementModel::LibEXTI(const TStrObjList& Params, TMacroData& E)  {
  if( Params.IsEmpty() )  {
    if (Vars.HasEXTI()) {
      E.SetRetVal(Vars.GetEXTI().ToString());
    }
    else {
      E.SetRetVal<olxstr>("n/a");
    }
  }
  else {
    Vars.SetEXTI(Params[0].ToDouble(),
      Params.Count() == 1 ? 0.0 : Params[1].ToDouble());
  }
}
//..............................................................................
void RefinementModel::LibUpdateCRParams(const TStrObjList& Params,
  TMacroData& E)
{
  IConstraintContainer* cc = rcRegister.Find(Params[0], NULL);
  if( cc == NULL )  {
    E.ProcessingError(__OlxSrcInfo, olxstr("Undefined container for: '") <<
      Params[0] << '\'');
    return;
  }
  cc->UpdateParams(Params[1].ToSizeT(), Params.SubListFrom(2));
}
//..............................................................................
void RefinementModel::LibShareADP(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  //size_t n = Cmds.Count();
  TSAtomPList atoms;
  double ang = -1001;
  if (Cmds.Count() > 0 && Cmds[0].IsNumber()) {
    //n = Cmds[0].ToSizeT();
    Cmds.Delete(0);
  }
  if (Cmds.Count() > 0 && Cmds[0].IsNumber()) {
    ang = Cmds[0].ToDouble();
    Cmds.Delete(0);
  }
  atoms = TXApp::GetInstance().FindSAtoms(Cmds);
  if (atoms.Count() < 3) {
    E.ProcessingError(__OlxSrcInfo, "At least three atoms are expected");
    return;
  }
  if (ang == -1001)
    ang = 360. / atoms.Count();
  adirection *d = 0;
  vec3d center, normal;
  // consider special cases... CF3, CM3 etc - need to find the bond direction
  if (atoms.Count() == 3) {
    TPtrList<TSAtom> cnt(atoms.Count());
    for (size_t i = 0; i < atoms.Count(); i++) {
      for (size_t j = 0; j < atoms[i]->NodeCount(); j++) {
        TSAtom &a = atoms[i]->Node(j);
        if (a.IsDeleted() || a.GetType() == iQPeakZ || a.GetType() == iHydrogenZ)
          continue;
        if (cnt[i] != 0) {  // attached to more than 2 atoms, invalidate
          cnt[i] = 0;
          break;
        }
        cnt[i] = &a;
      }
    }
    bool valid_for_bond = (cnt[0] != 0);
    if (valid_for_bond) {
      for (size_t i = 1; i < cnt.Count(); i++) {
        if (cnt[i] == 0 || cnt[i] != cnt[0]) {
          valid_for_bond = false;
          break;
        }
      }
    }
    if (valid_for_bond) {
      size_t p_ind = InvalidIndex;
      for (size_t i = 0; i < cnt[0]->NodeCount(); i++) {
        TSAtom &a = cnt[0]->Node(i);
        if (a.IsDeleted() || a.GetType() == iQPeakZ ||
          atoms.IndexOf(a) != InvalidIndex)
        {
          continue;
        }
        if (p_ind != InvalidIndex) {
          p_ind = InvalidIndex;
          break;
        }
        p_ind = i;
      }
      if (p_ind != InvalidIndex) { // add the direction then
        TCAtomGroup as;
        as.Add(new TGroupCAtom(
          cnt[0]->Node(p_ind).CAtom(), cnt[0]->Node(p_ind).GetMatrix()));
        as.Add(new TGroupCAtom(cnt[0]->CAtom(), cnt[0]->GetMatrix()));
        normal = (cnt[0]->crd() - cnt[0]->Node(p_ind).crd()).Normalise();
        center = (atoms[0]->crd() + atoms[1]->crd() + atoms[2]->crd()) / 3;
        d = AddDirection(as, direction_vector);
      }
    }
  }
  // create a normal direction
  if (d == 0) {
    TCAtomGroup as;
    for (size_t i = 0; i < atoms.Count(); i++) {
      as.Add(new TGroupCAtom(atoms[i]->CAtom(), atoms[i]->GetMatrix()));
    }
    d = AddDirection(as, direction_normal);
    plane::mean<>::out po = plane::mean<>::calc(atoms, TSAtom::CrdAccessor());
    center = po.center;
    normal = po.normals[0];
  }
  if (d == 0) {
    E.ProcessingError(__OlxSrcInfo, "could not add direction object");
    return;
  }
  plane::Sort(atoms, TSAtom::CrdAccessor(), center, normal);
  double ra = atoms.Count()*ang;
  for (size_t i = 1; i < atoms.Count(); i++) {
    SharedRotatedADPs.items.Add(
      new rotated_adp_constraint(
        atoms[0]->CAtom(), atoms[i]->CAtom(), *d, (ra -= ang), false));
  }
}
//..............................................................................
void RefinementModel::LibCalcCompleteness(const TStrObjList& Params,
  TMacroData& E)
{
  if (Params.Count() == 2) {
    E.SetRetVal(CalcCompletenessTo2Theta(Params[0].ToDouble(), Params[1].ToBool()));
    return;
  }
  E.SetRetVal(CalcCompletenessTo2Theta(Params[0].ToDouble(), false));
}
//..............................................................................
void RefinementModel::LibMaxIndex(const TStrObjList& Params,
  TMacroData& E)
{
  vec3i mi;
  if (Params.IsEmpty()) {
    mi = CalcMaxHklIndexForD(GetMergeStat().MinD);
  }
  else {
    mi = CalcMaxHklIndexFor2Theta(Params[0].ToDouble());
  }
  E.SetRetVal(olxstr(' ').Join(mi));
}
//..............................................................................
void RefinementModel::LibNewAfixGroup(TStrObjList &Cmds,
  const TParamList &Options, TMacroData &E)
{
  int afix = Cmds[0].ToInt();
  TCAtomPList atoms;
  for (size_t i = 1; i < Cmds.Count(); i++) {
    size_t ai = Cmds[i].ToSizeT();
    if (ai >= aunit.AtomCount()) {
      E.ProcessingError(__OlxSrcInfo, "atom index out of bonds");
      return;
    }
    atoms.Add(aunit.GetAtom(ai));
  }
  size_t exp_c = TAfixGroup::ExpectedAtomCount(afix);
  if (exp_c != InvalidSize && atoms.Count() != exp_c) {
    E.ProcessingError(__OlxSrcInfo, "unexpected atom count for given AFIX");
    return;
  }
  size_t st = 0;
  TCAtom *pvt = NULL;
  if (TAfixGroup::HasExcplicitPivot(afix)) {
    st = 1;
    pvt = atoms[0];
  }
 
  TAfixGroup &ag = AfixGroups.New(pvt, afix,
    Options.FindValue('d', '0').ToDouble(),
    Options.FindValue("sof", '0').ToDouble(),
    Options.FindValue('u', '0').ToDouble());
  for (size_t i = st; i < atoms.Count(); i++) {
    ag.AddDependent(*atoms[i]);
  }
  TBasicApp::NewLogEntry() << "Adding:";
  TBasicApp::NewLogEntry() << ag.ToString();
}
//..............................................................................
void RefinementModel::LibNewRestraint(TStrObjList &Cmds,
  const TParamList &Options, TMacroData &E)
{
  size_t st = 1;
  TSimpleRestraint *sr = NULL;
  if (Cmds[0].Equalsi("sadi")) {
    sr = &rSADI.AddNew();
  }
  else if (Cmds[0].Equalsi("dfix")) {
    sr = &rDFIX.AddNew();
    sr->SetValue(Cmds[st++].ToDouble());
  }
  else if (Cmds[0].Equalsi("dang")) {
    sr = &rDANG.AddNew();
    sr->SetValue(Cmds[st++].ToDouble());
  }
  else if (Cmds[0].Equalsi("flat")) {
    sr = &rFLAT.AddNew();
  }
  else if (Cmds[0].Equalsi("chiv")) {
    sr = &rCHIV.AddNew();
  }
  else if (Cmds[0].Equalsi("delu")) {
    sr = &rDELU.AddNew();
  }
  else if (Cmds[0].Equalsi("simu")) {
    sr = &rSIMU.AddNew();
  }
  else if (Cmds[0].Equalsi("rigu")) {
    sr = &rRIGU.AddNew();
  }
  if (sr == NULL) {
    E.ProcessingError(__OlxSrcInfo, "unknown restraint: ").quote() << Cmds[0];
    return;
  }
  for (size_t i = st; i < Cmds.Count(); i++) {
    size_t ai = Cmds[i].ToSizeT();
    if (ai >= aunit.AtomCount()) {
      E.ProcessingError(__OlxSrcInfo, "atom index out of bonds");
      return;
    }
    sr->AddAtom(aunit.GetAtom(ai), NULL);
  }
  double s = Options.FindValue("s1", '0').ToDouble();
  if (s != 0) {
    sr->SetEsd(s);
    s = Options.FindValue("s2", '0').ToDouble();
    if (s != 0) {
      sr->SetEsd1(s);
    }
  }
  TBasicApp::NewLogEntry() << "Adding:";
  TBasicApp::NewLogEntry() << sr->ToString();
}
//..............................................................................
void RefinementModel::LibModelSrc(const TStrObjList &Params, TMacroData &E) {
  if (Params.IsEmpty()) {
    E.SetRetVal(GetModelSource());
  }
  else {
    SetModelSource(Params[0]);
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
IOlxObject *RefinementModel::VPtr::get_ptr() const {
  return &TXApp::GetInstance().XFile().GetRM();
}
//..............................................................................
//..............................................................................
//..............................................................................
TLibrary* RefinementModel::ExportLibrary(const olxstr& name) {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("rm") : name);
  olx_vptr<RefinementModel> thip(new VPtr);
  lib->Register(
    new TFunction<RefinementModel>(thip, &RefinementModel::LibOSF,
      "OSF",
      fpNone|fpOne,
"Returns/sets OSF"));
  lib->Register(
    new TFunction<RefinementModel>(thip, &RefinementModel::LibFVar,
      "FVar",
      fpOne|fpTwo|fpThree,
"Returns/sets FVAR referred by index"));
  lib->Register(
    new TFunction<RefinementModel>(thip, &RefinementModel::LibBASF,
    "BASF",
    fpOne | fpTwo | fpThree,
    "Returns/sets BASF referred by index"));
  lib->Register(
    new TFunction<RefinementModel>(thip, &RefinementModel::LibEXTI,
      "Exti",
      fpNone|fpOne|fpTwo,
"Returns/sets EXTI"));
  lib->Register(
    new TFunction<RefinementModel>(thip, &RefinementModel::LibUpdateCRParams,
      "UpdateCR",
      fpAny^(fpNone|fpOne|fpTwo),
"Updates constraint or restraint parameters (name, index, {values})") );
  lib->Register(
    new TFunction<RefinementModel>(thip, &RefinementModel::LibCalcCompleteness,
      "Completeness",
      fpOne|fpTwo,
"Calculates completeness to the given 2 theta value") );
  lib->Register(
    new TMacro<RefinementModel>(thip, &RefinementModel::LibShareADP,
      "ShareADP", EmptyString(),
      fpAny,
"Creates a rotated ADP constraint for given atoms. Currently works only for "
"T-X3 groups (X-CMe3, X-CF3 etc) and for rings"
));

  lib->Register(
    new TFunction<RefinementModel>(thip, &RefinementModel::LibHasOccu,
    "HasOccu",
    fpNone,
    "Returns true if occupancy of any of the atoms is refined or deviates from 1"));

  lib->Register(
    new TFunction<RefinementModel>(thip, &RefinementModel::LibMaxIndex,
    "MaxIndex",
    fpNone|fpOne,
    "Calculates largest Miller index for current structure or the given 2 "
    "theta value"));

  lib->Register(
    new TMacro<RefinementModel>(thip, &RefinementModel::LibNewAfixGroup,
    "NewAfixGroup",
    "d-distance when applicable&;"
    "sof-occupancy [11]&;"
    "u-default U value for atoms",
    fpAny^(fpNone|fpOne|fpTwo),
    "Creates a new AFIX group expects AFIX code and atom ids"));

  lib->Register(
    new TMacro<RefinementModel>(thip, &RefinementModel::LibNewRestraint,
    "NewRestraint",
    "s1-standard deviation 1&;"
    "s2-standard deviation 2",
    fpAny ^ (fpNone|fpOne),
    "Creates a new restraint expects restraint name, parameters if required "
    "and atom ids"));

  lib->Register(
    new TFunction<RefinementModel>(thip, &RefinementModel::LibModelSrc,
    "ModelSrc",
    fpNone | fpOne,
    "Sets the source for this model - an identifier for the external code when "
    "reading a model file."));

  return lib;
}
//..............................................................................
//..............................................................................
//..............................................................................
