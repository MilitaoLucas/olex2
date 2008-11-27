// if this is not inluded: internal compiler error at 0xc19d9e with base 0xc10000
#include "egc.h"

#include "auto.h"
#include "ins.h"

#include "integration.h"
#include "xmacro.h"
#include "sptrlist.h"
#include "beevers-lipson.h"
#include "arrays.h"
#include "maputil.h"
#include "estopwatch.h"


void XLibMacros::funATA(const TStrObjList &Cmds, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  olxstr folder( Cmds.IsEmpty() ? EmptyString : Cmds[0] );
  int arg = 0;
  if( folder.IsNumber() )  {
    arg = folder.ToInt();
    folder = EmptyString;
  }
  if( folder.IsEmpty() && olex::IOlexProcessor::GetInstance() != NULL )
    olex::IOlexProcessor::GetInstance()->executeMacro("clean -npd");
  // Qpeak rings analysis ...
  try  {
    TTypeList< TSAtomPList > rings;
    xapp.FindRings("QQQQQQ", rings);
    for(int i=0; i < rings.Count(); i++ )  {
      double rms = TSPlane::CalcRMS( rings[i] );
      if( rms < 0.1 && TNetwork::IsRingRegular( rings[i]) )  {
        for( int j=0; j < rings[i].Count(); j++ )  {
          rings[i][j]->CAtom().SetLabel("Cph");
        }
      }
    }
  }
  catch( ... )  {  ;  }
  static olxstr FileName(xapp.XFile().GetFileName());
//  static TAtomTypePermutator AtomPermutator;
//  AtomPermutator.SetActive( false );
//  AtomPermutator.SetActive( !Options.Contains("p") );
//  if( !AtomPermutator.IsActive() )  AtomPermutator.Init();

//  if( FileName != xapp.XFile().GetFileName() )  {
//    AtomPermutator.Init();
//    FileName = xapp.XFile().GetFileName();
//  }
//  if( AtomPermutator.IsActive() )  AtomPermutator.ReInit( xapp.XFile().GetAsymmUnit() );
  olxstr autodbf( xapp.BaseDir() + "acidb.db");
  if( TAutoDB::GetInstance() == NULL )  {
    TEGC::AddP( new TAutoDB(*((TXFile*)xapp.XFile().Replicate()), xapp ) );
    if( TEFile::FileExists( autodbf ) )  {
      TEFile dbf(autodbf, "rb");
      TAutoDB::GetInstance()->LoadFromStream( dbf );
    }
  }
  if( !folder.IsEmpty() )  {
    TAutoDB::GetInstance()->ProcessFolder( folder );
    TEFile dbf(autodbf, "w+b");
    TAutoDB::GetInstance()->SaveToStream( dbf );
  }

  TLattice& latt = xapp.XFile().GetLattice();
  TAsymmUnit& au = latt.GetAsymmUnit();
  TAtomsInfo& atomsInfo = TAtomsInfo::GetInstance();
  TBAIPList bai_l;
  if( arg == 1 )  {
    if( xapp.CheckFileType<TIns>() )  {
      TIns& ins = xapp.XFile().GetLastLoader<TIns>();
      TStrList sl(ins.GetSfac(), ' ');
      for( int i=0; i < sl.Count(); i++ ) 
        bai_l.Add( atomsInfo.FindAtomInfoBySymbol(sl[i]) );
    }    
  }
  TAutoDB::AnalysisStat stat;
  uint64_t st = TETime::msNow();
  TAutoDB::GetInstance()->AnalyseStructure( xapp.XFile().GetFileName(), latt, 
    NULL, stat, bai_l.IsEmpty() ? NULL : &bai_l);
  st = TETime::msNow() - st;
  TBasicApp::GetLog().Info( olxstr("Elapsed time ") << st << " ms");

//  if( AtomPermutator.IsActive() )  AtomPermutator.Permutate();
  if( olex::IOlexProcessor::GetInstance() != NULL )
    olex::IOlexProcessor::GetInstance()->executeMacro("fuse");
  int ac = 0;
  for( int i=0; i < au.AtomCount(); i++ )  {
    TCAtom& a = au.GetAtom(i);
      if( a.IsDeleted() || a.GetAtomInfo() == iHydrogenIndex || 
        a.GetAtomInfo() == iDeuteriumIndex || a.GetAtomInfo() == iQPeakIndex )  continue;
    ac++;
  }
  Error.SetRetVal( olxstr(stat.AtomTypeChanges!=0) << ';' << 
    (double)stat.ConfidentAtomTypes*100/ac );
}
//..............................................................................
void XLibMacros::macAtomInfo(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  TSAtomPList satoms;
  xapp.FindSAtoms( Cmds.Text(' '), satoms );
  if( TAutoDB::GetInstance() == NULL )  {
    TEGC::AddP( new TAutoDB(*((TXFile*)xapp.XFile().Replicate()), xapp ) );
    olxstr autodbf( xapp.BaseDir() + "acidb.db");
    if( TEFile::FileExists( autodbf ) )  {
      TEFile dbf(autodbf, "rb");
      TAutoDB::GetInstance()->LoadFromStream( dbf );
    }
  }
  TStrList report;
  for( int i=0; i < satoms.Count(); i++ ) 
    TAutoDB::GetInstance()->AnalyseNode( *satoms[i], report );
  xapp.GetLog() << ( report );
}
//..............................................................................
void XLibMacros::macVATA(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  TEFile log(Cmds.Text(' '), "a+b");
  if( TAutoDB::GetInstance() == NULL )  {
    TEGC::AddP( new TAutoDB(*((TXFile*)xapp.XFile().Replicate()), xapp ) );
    olxstr autodbf( xapp.BaseDir() + "acidb.db");
    if( TEFile::FileExists( autodbf ) )  {
      TEFile dbf(autodbf, "rb");
      TAutoDB::GetInstance()->LoadFromStream( dbf );
    }
  }
  TStrList report;
  TAutoDB::GetInstance()->ValidateResult( xapp.XFile().GetFileName(), xapp.XFile().GetLattice(), report);
  for( int i=0; i < report.Count(); i++ )
    log.Writenl( report[i] );
}
//..............................................................................
struct Main_BaiComparator {
  static int Compare(const TPrimitiveStrListData<olxstr,TBasicAtomInfo*>* a, 
                     const TPrimitiveStrListData<olxstr,TBasicAtomInfo*>* b)  {
      return a->GetObject()->GetIndex() - b->GetObject()->GetIndex();
  }
};
void helper_CleanBaiList(TStrPObjList<olxstr,TBasicAtomInfo*>& list, TSPtrList<TBasicAtomInfo>& au_bais)  {
  TXApp& xapp = TXApp::GetInstance();
  if( xapp.CheckFileType<TIns>() )  {
    TIns& ins = xapp.XFile().GetLastLoader<TIns>();
    list.Clear();   
    list.Strtok(ins.GetSfac(), ' ');
    TAtomsInfo& bai = TAtomsInfo::GetInstance();
    for( int i=0; i < list.Count(); i++ )  { 
      list.Object(i) = bai.FindAtomInfoBySymbol(list[i]);
      au_bais.Add(list.Last().Object());
    }
    list.QuickSort<Main_BaiComparator>();
  }
}

void XLibMacros::macClean(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  TAtomsInfo& AtomsInfo = TAtomsInfo::GetInstance();
  TStrPObjList<olxstr,TBasicAtomInfo*> sfac;
  TSPtrList<TBasicAtomInfo> AvailableTypes;
  static TPtrList<TBasicAtomInfo> StandAlone;
  if( StandAlone.IsEmpty() )  {
    StandAlone.Add( &AtomsInfo.GetAtomInfo(iOxygenIndex) );
    StandAlone.Add( &AtomsInfo.GetAtomInfo(iSodiumIndex) );
    StandAlone.Add( &AtomsInfo.GetAtomInfo(iMagnesiumIndex) );
    StandAlone.Add( &AtomsInfo.GetAtomInfo(iChlorineIndex) );
    StandAlone.Add( &AtomsInfo.GetAtomInfo(iPotassiumIndex) );
    StandAlone.Add( &AtomsInfo.GetAtomInfo(iCalciumIndex) );
  }
  helper_CleanBaiList(sfac, AvailableTypes);
  if( TAutoDB::GetInstance() == NULL )  {
    olxstr autodbf( xapp.BaseDir() + "acidb.db");
    TEGC::AddP( new TAutoDB(*((TXFile*)xapp.XFile().Replicate()), xapp ) );
    if( TEFile::FileExists( autodbf ) )  {
      TEFile dbf(autodbf, "rb");
      TAutoDB::GetInstance()->LoadFromStream( dbf );
    }
  }

  bool runFuse = !Options.Contains("f");
  bool changeNPD = !Options.Contains("npd");
  bool analyseQ = !Options.Contains("aq");
  bool assignTypes = !Options.Contains("at");
  const double aqV = Options.FindValue("aq", "0.3").ToDouble(); // R+aqV
  // qpeak anlysis
  TAsymmUnit& au = xapp.XFile().GetAsymmUnit();
  if( analyseQ )  {
    TPSTypeList<double, TCAtom*> SortedQPeaks;
    TTypeList< AnAssociation2<double, TCAtomPList*> > vals;
    int cnt = 0;
    double avQPeak = 0;
    bool OnlyQPeakModel = true;
    for( int i=0; i < au.AtomCount(); i++ )  {
      if( au.GetAtom(i).IsDeleted() )  continue;
      if( au.GetAtom(i).GetAtomInfo() != iQPeakIndex )
        OnlyQPeakModel  = false;
      else  {
        SortedQPeaks.Add( au.GetAtom(i).GetQPeak(), &au.GetAtom(i));
        avQPeak += au.GetAtom(i).GetQPeak();
        cnt++;
      }
    }
    if( cnt != 0 )
      avQPeak /= cnt;
    cnt = 0;
    if( SortedQPeaks.Count() != 0 )  {
      vals.AddNew<double, TCAtomPList*>(0, new TCAtomPList);
      for(int i=SortedQPeaks.Count()-1; i >=1; i-- )  {
        if( (SortedQPeaks.GetComparable(i) - SortedQPeaks.GetComparable(i-1))/SortedQPeaks.GetComparable(i) > 0.05 )  {
          //FGlConsole->PostText( olxstr("Threshold here: ") << SortedQPeaks.GetObject(i)->GetLabel() );
          vals.Last().A() += SortedQPeaks.GetComparable(i);
          vals.Last().B()->Add( SortedQPeaks.GetObject(i));
          cnt++;
          vals.Last().A() /= cnt;
          cnt = 0;
          vals.AddNew<double, TCAtomPList*>(0, new TCAtomPList);
          continue;
        }
        vals.Last().A() += SortedQPeaks.GetComparable(i);
        vals.Last().B()->Add( SortedQPeaks.GetObject(i));
        cnt ++;
      }
      vals.Last().B()->Add(SortedQPeaks.GetObject(0));
      cnt++;
      if( cnt > 1 )
        vals.Last().A() /= cnt;

      TBasicApp::GetLog().Info( olxstr("Average QPeak: ") << avQPeak);
      TBasicApp::GetLog().Info("QPeak steps:");
      for( int i=0; i < vals.Count(); i++ )
        TBasicApp::GetLog().Info( vals[i].GetA() );

      //    double thVal = 2;
      double thVal = (avQPeak  < 2 ) ? 2 : avQPeak*0.75;

      TBasicApp::GetLog().Info(olxstr("QPeak threshold:") << thVal);

      if( SortedQPeaks.Count() == 1 )  {  // only one peak present
        if( SortedQPeaks.GetComparable(0) < thVal )
          SortedQPeaks.GetObject(0)->SetDeleted(true);
      }
      else  {
        double wght = (SortedQPeaks.Last().Comparable()-avQPeak)/
          (avQPeak-SortedQPeaks.GetComparable(0));
        for( int i=vals.Count()-1; i >= 0; i-- )  {
          if( vals[i].GetA() < thVal )  {
            for( int j=0; j < vals[i].GetB()->Count(); j++ )
              vals[i].GetB()->Item(j)->SetDeleted(true);
          }
        }
      }
      for( int i=0; i < vals.Count(); i++ )
        delete vals[i].B();
    }
  }
  // end qpeak analysis

  // distance analysis
  TTypeList<AnAssociation2<TCAtom*, vec3d> > neighbours;
  TLattice& latt = xapp.XFile().GetLattice();
  // qpeaks first
  TSAtomPList QPeaks;
  for( int i=0;  i < latt.AtomCount(); i++ )  {
    TSAtom& sa = latt.GetAtom(i);
    if( sa.IsDeleted() || sa.CAtom().IsDeleted() )  continue;
    if( sa.GetAtomInfo() == iQPeakIndex )
      QPeaks.Add( &sa );
  }
  for( int i=0; i < QPeaks.Count(); i++ )  {
    if( QPeaks[i]->IsDeleted() || QPeaks[i]->CAtom().IsDeleted() )  continue;
    neighbours.Clear();
    TAutoDBNode nd(*QPeaks[i], &neighbours);
    for( int j=0; j < nd.DistanceCount(); j++ )  {
      if( nd.GetDistance(j) < (neighbours[j].GetA()->GetAtomInfo().GetRad1()+aqV) )  {  // at leats H-bond
        if( neighbours[j].GetA()->GetAtomInfo() == iQPeakIndex )  {
          if( nd.GetDistance(j) < 1 )  {
            if( neighbours[j].GetA()->GetQPeak() < QPeaks[i]->CAtom().GetQPeak() )  {
              neighbours[j].GetA()->SetDeleted(true);
            }
          }
        }
        else {
          QPeaks[i]->SetDeleted(true);
          QPeaks[i]->CAtom().SetDeleted(true);
        }
      }
      if( nd.GetDistance(j) > 1.8 )  {
//        if( neighbours[j].GetA()->GetAtomInfo().GetIndex() < 20 )  {  // Ca
//          QPeaks[i]->SetDeleted(true);
//          QPeaks[i]->CAtom().SetDeleted(true);
//        }
      }
    }
    if( nd.NodeCount() == 2 && nd.GetAngle(0) < 90 )  {
      if( !neighbours[0].GetA()->IsDeleted() && !neighbours[1].GetA()->IsDeleted() )  {
        QPeaks[i]->SetDeleted(true);
        QPeaks[i]->CAtom().SetDeleted(true);
        continue;
      }
    }

//    for( int j=0; j < nd.AngleCount(); j++ )  {
//      if( nd.GetAngle(j) < 90
//    }
  }
  // call whatever left carbons ...
  if( assignTypes )  {
    for( int i=0; i < QPeaks.Count(); i++ )  {
      if( QPeaks[i]->IsDeleted() || QPeaks[i]->CAtom().IsDeleted() )  continue;
      TBasicApp::GetLog().Info( olxstr(QPeaks[i]->CAtom().GetLabel()) << " -> C" );
      QPeaks[i]->CAtom().Label() = "C";
      QPeaks[i]->CAtom().AtomInfo( &AtomsInfo.GetAtomInfo(iCarbonIndex));
    }
  }

  TDoubleList Uisos;
  if( xapp.XFile().GetFileName() == TAutoDB::GetInstance()->GetLastFileName() )
    Uisos.Assign( TAutoDB::GetInstance()->GetUisos() );
  for( int i=0; i < latt.FragmentCount(); i++ )  {
    if( latt.GetFragment(i).NodeCount() > 7 )   { // skip up to PF6 or so for Uiso analysis
      if( Uisos.Count() <= i )  Uisos.Add(0.0);
      if( Uisos[i] == 0 )  {
        int ac = 0;
        for( int j=0;  j < latt.GetFragment(i).NodeCount(); j++ )  {
          TSAtom& sa = latt.GetFragment(i).Node(j);
          if( sa.IsDeleted() || sa.GetAtomInfo() == iHydrogenIndex ||
                                sa.GetAtomInfo() == iQPeakIndex )  continue;
          Uisos[i] += sa.CAtom().GetUiso();
          ac++;
        }
        if( ac != 0 )  Uisos[i] /= ac;
      }
      if( Uisos[i] > 0 )  {
        for( int j=0;  j < latt.GetFragment(i).NodeCount(); j++ )  {
          TSAtom& sa = latt.GetFragment(i).Node(j);
          if( sa.IsDeleted() || sa.GetAtomInfo() == iHydrogenIndex )  continue;
          if( sa.GetAtomInfo() != iQPeakIndex && sa.CAtom().GetUiso() > Uisos[i]*3)  {
            sa.SetDeleted(true);
            sa.CAtom().SetDeleted(true);
            continue;
          }
        }
      }
    }
    else  if( assignTypes )  {  // treat O an Cl
      if( latt.GetFragment(i).NodeCount() == 1 && !latt.GetFragment(i).Node(0).IsDeleted() )  {
        TSAtom& sa = latt.GetFragment(i).Node(0);
        bool alone = true;
        for( int j=0; j < sa.CAtom().AttachedAtomCount(); j++ )
          if( sa.CAtom().GetAttachedAtom(j).GetAtomInfo() != iQPeakIndex )  {
            alone = false;
            break;
          }
        if( alone )  {
          bool assignHeaviest = false, assignLightest = false;
          const TAutoDB::AnalysisStat& stat = TAutoDB::GetInstance()->GetStats();
          if( stat.SNAtomTypeAssignments == 0 )  { // now we can make up types
            bool found = false;
            for( int j=0; j < StandAlone.Count(); j++ )  {
              if( sa.GetAtomInfo() == *StandAlone[j] )  {
                found = true;
                if( sa.CAtom().GetUiso() < 0.01 )  {  // search heavier
                  bool assigned = false;
                  for( int k=j+1; k < StandAlone.Count(); k++ )  {
                    if( AvailableTypes.IndexOf(StandAlone[k]) != -1 )  {
                      sa.CAtom().Label() = StandAlone[k]->GetSymbol();
                      sa.CAtom().AtomInfo(StandAlone[k]);
                      assigned = true;
                      break;
                    }
                  }
                  if( !assigned )  assignHeaviest = true;
                }
                else if( sa.CAtom().GetUiso() > 0.2 )  {  // search lighter
                  bool assigned = false;
                  for( int k=j-1; k >= 0; k-- )  {
                    if( AvailableTypes.IndexOf(StandAlone[k]) != -1 )  {
                      sa.CAtom().Label() = StandAlone[k]->GetSymbol();
                      sa.CAtom().AtomInfo(StandAlone[k]);
                      assigned = true;
                      break;
                    }
                  }
                  if( !assigned )  assignLightest = true;
                }
              }
            }
            if( !found || assignLightest )  {  // make lightest then
              sa.CAtom().Label() = StandAlone[0]->GetSymbol();
              sa.CAtom().AtomInfo(StandAlone[0]);
            }
            else if( assignHeaviest )  {
              sa.CAtom().Label() = StandAlone.Last()->GetSymbol();
              sa.CAtom().AtomInfo(StandAlone.Last());
            }
          }
        }
      }
      for( int j=0;  j < latt.GetFragment(i).NodeCount(); j++ )  {
        TSAtom& sa = latt.GetFragment(i).Node(j);
        if( sa.IsDeleted() || sa.GetAtomInfo() == iHydrogenIndex )  continue;
        if( sa.GetAtomInfo() != iQPeakIndex && sa.CAtom().GetUiso() > 0.25 )  {
          TBasicApp::GetLog().Info(olxstr(sa.GetLabel()) << " blown up");
          sa.SetDeleted(true);
          sa.CAtom().SetDeleted(true);
          continue;
        }
      }
    }
    for( int j=0;  j < latt.GetFragment(i).NodeCount(); j++ )  {
      TSAtom& sa = latt.GetFragment(i).Node(j);
      if( sa.IsDeleted() || sa.CAtom().IsDeleted() ||
          sa.GetAtomInfo() == iHydrogenIndex || sa.GetAtomInfo() == iQPeakIndex )  continue;

      neighbours.Clear();
      TAutoDBNode nd(sa, &neighbours);
      for( int k=0; k < nd.DistanceCount(); k++ )  {
        if( neighbours[k].GetA()->IsDeleted() )
          continue;
        if( nd.GetDistance(k) < (neighbours[k].GetA()->GetAtomInfo().GetRad1()+aqV) &&
            neighbours[k].GetA()->GetAtomInfo() != iHydrogenIndex )  {
          if( neighbours[k].GetA()->GetAtomInfo() == iQPeakIndex ||
                neighbours[k].GetA()->GetAtomInfo() <= iFluorineIndex )
          {
              neighbours[k].GetA()->SetDeleted(true);
          }
          else  {
            if( sa.GetAtomInfo() == iQPeakIndex || sa.GetAtomInfo() <= iFluorineIndex )  {
              sa.SetDeleted(true);
              sa.CAtom().SetDeleted(true);
            }
          }
        }
      }
    }
  }
  // treating NPD atoms... promoting to the next available type
  if( changeNPD && !sfac.IsEmpty() )  {
    for( int i=0; i < latt.AtomCount(); i++ )  {
      TSAtom& sa = latt.GetAtom(i);
      if( (sa.GetEllipsoid() != NULL && sa.GetEllipsoid()->IsNPD()) ||
        (sa.CAtom().GetUiso() <= 0.005) )  {
          int ind = sfac.IndexOfObject( &sa.GetAtomInfo() );
          if( ind >= 0 && ((ind+1) < sfac.Count()) )  {
            sa.CAtom().AtomInfo( sfac.Object(ind+1) );
          }
      }
    }
  }
  //end treating NDP atoms
  if( runFuse && olex::IOlexProcessor::GetInstance() != NULL )
    olex::IOlexProcessor::GetInstance()->executeMacro("fuse");
}
//..............................................................................
struct Main_SfacComparator {
  static int Compare(const AnAssociation2<int,TBasicAtomInfo*>& a, 
                     const AnAssociation2<int,TBasicAtomInfo*>& b)  {
      return b.GetB()->GetIndex() - a.GetB()->GetIndex();
  }
};
void XLibMacros::funVSS(const TStrObjList &Cmds, TMacroError &Error)  {
  //olxstr autodbf( xapp.BaseDir() + "acidb.db");
  //if( TAutoDB::GetInstance() == NULL )  {
  //  TEGC::AddP( new TAutoDB(*((TXFile*)xapp.XFile().Replicate()), *FXApp ) );
  //  if( TEFile::FileExists( autodbf ) )  {
  //    TEFile dbf(autodbf, "rb");
  //    TAutoDB::GetInstance()->LoadFromStream( dbf );
  //  }
  //}
  TXApp& xapp = TXApp::GetInstance();
  TLattice& latt = xapp.XFile().GetLattice();
  TUnitCell& uc = latt.GetUnitCell();
  TAsymmUnit& au = latt.GetAsymmUnit();
  int ValidatedAtomCount = 0, AtomCount=0;
  bool trim = Cmds[0].ToBool();
  bool use_formula = Cmds[0].ToBool();
  if( use_formula )  {
    if( xapp.CheckFileType<TIns>() )  {
      TIns& ins = xapp.XFile().GetLastLoader<TIns>();
      TTypeList< AnAssociation2<int,TBasicAtomInfo*> > sl;
      TStrList sfac(ins.GetSfac(), ' ');
      TStrList unit(ins.GetUnit(), ' ');
      int ac = 0;
      for( int i=0; i < sfac.Count(); i++ )  {
        int cnt = unit[i].ToInt();
        TBasicAtomInfo* bai = au.GetAtomsInfo()->FindAtomInfoBySymbol(sfac[i]);
        if( *bai == iHydrogenIndex )  continue;
        sl.AddNew( cnt, bai );
        ac += cnt;
      }
      sl.QuickSorter.Sort<Main_SfacComparator>(sl);  // sorts ascending
      double auv = latt.GetUnitCell().CalcVolume()/latt.GetUnitCell().MatrixCount();
      double ratio = auv/(16*ac);
      for( int i=0; i < sl.Count(); i++ )
        sl[i].A() = Round(ratio*sl[i].GetA());

      TPSTypeList<double, TCAtom*> SortedQPeaks;
      for( int i=0; i < au.AtomCount(); i++ )  {
        if( au.GetAtom(i).IsDeleted() )  continue;
        if( au.GetAtom(i).GetAtomInfo() == iQPeakIndex )
          SortedQPeaks.Add( au.GetAtom(i).GetQPeak(), &au.GetAtom(i));
        else  {
          for( int j=0; j < sl.Count(); j++ )  {
            if( *sl[j].GetB() == au.GetAtom(i).GetAtomInfo() )  {
              sl[j].A()--;
              break;
            }
          }
        }
      }
      for( int i=0; i < sl.Count(); i++ )  {
        while( sl[i].GetA() > 0 )  {
          if( SortedQPeaks.IsEmpty() )  break;
          sl[i].A() --;
          SortedQPeaks.Last().Object()->Label() = (olxstr(sl[i].GetB()->GetSymbol()) << i);
          SortedQPeaks.Last().Object()->AtomInfo( sl[i].B() );
          SortedQPeaks.Last().Object()->SetQPeak(0);
          SortedQPeaks.Remove( SortedQPeaks.Count()-1);
        }
        if( SortedQPeaks.IsEmpty() ) break;
      }
      // get rid of the rest of Q-peaks and "validate" geometry of atoms
      for( int i=0; i < au.AtomCount(); i++ )  {
        if( au.GetAtom(i).GetAtomInfo() == iQPeakIndex ) 
          au.GetAtom(i).SetDeleted(true);
      }
      TArrayList< AnAssociation2<TCAtom const*, vec3d> > res;
      for( int i=0; i < au.AtomCount(); i++ )  {
        if( au.GetAtom(i).IsDeleted() )  continue;
        uc.FindInRange( au.GetAtom(i).ccrd(), au.GetAtom(i).GetAtomInfo().GetRad1()+1.3, res);
        AtomCount++;
        double wght = 1;
        if( res.Count() > 1 )  {
          double awght = 1./(res.Count()*(res.Count()-1));
          for( int j=0; j < res.Count(); j++ )  {
            if( res[j].GetB().QLength() < 1 )  wght -= 0.5/res.Count();
            for( int k=j+1; k < res.Count(); k++ )  {
              double cang = res[j].GetB().CAngle(res[k].GetB());
              if( cang > 0.588 )  { // 56 degrees
                wght -= awght;
              }
            }
          }
        }
        else if( res.Count() == 1 ) {  // just one bond
          if( res[0].GetB().QLength() < 1 )
            wght = 0;
        }
        else  // no bonds, cannot say anything
          wght = 0;

        if( wght >= 0.95 )
          ValidatedAtomCount++;
        res.Clear();
      }
    }
    xapp.XFile().EndUpdate();
  }
  else if( trim && false )  {
    double auv = latt.GetUnitCell().CalcVolume()/latt.GetUnitCell().MatrixCount();
    int ac = Round(auv/18.6);
    int to_delete = au.AtomCount() - ac;
    if( to_delete > 0 )  {
      TPSTypeList<double, TCAtom*> SortedQPeaks;
      TAsymmUnit& au = xapp.XFile().GetAsymmUnit();
      for( int i=0; i < au.AtomCount(); i++ )  {
        if( au.GetAtom(i).IsDeleted() )  continue;
        if( au.GetAtom(i).GetAtomInfo() == iQPeakIndex )
          SortedQPeaks.Add( au.GetAtom(i).GetQPeak(), &au.GetAtom(i));
      }
      for( int i=0; i < olx_min(to_delete,SortedQPeaks.Count()); i++ )
        SortedQPeaks.Object(i)->SetDeleted(true);
    }
    xapp.XFile().EndUpdate();
  }
//  TAutoDB::AnalysisStat stat;
//  TAutoDB::GetInstance()->AnalyseStructure( xapp.XFile().GetFileName(), latt, 
//    NULL, stat, NULL);
  Error.SetRetVal( (double)ValidatedAtomCount*100/AtomCount );
}
//..............................................................................
void XLibMacros::funFATA(const TStrObjList &Cmds, TMacroError &E)  {
  TXApp& xapp = TXApp::GetInstance();
  TStopWatch sw(__FUNC__);
  double resolution = 0.2;
  resolution = 1./resolution;
  TRefList refs;
  TArrayList<compd> F;
  
  olxstr err( SFUtil::GetSF(refs, F, SFUtil::mapTypeDiff, SFUtil::sfOriginOlex2, SFUtil::scaleRegression) );
  if( !err.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, err);
    return;
  }
  TAsymmUnit& au = xapp.XFile().GetAsymmUnit();
  TUnitCell& uc = xapp.XFile().GetUnitCell();
  TSpaceGroup* sg = NULL;
  try  { sg = &xapp.XFile().GetLastLoaderSG();  }
  catch(...)  {
    E.ProcessingError(__OlxSrcInfo, "could not locate space group");
    return;
  }
  TArrayList<StructureFactor> P1SF;
  TArrayList<vec3i> hkl(refs.Count());
  for( int i=0; i < refs.Count(); i++ )  {
    hkl[i][0] = refs[i].GetH();
    hkl[i][1] = refs[i].GetK();
    hkl[i][2] = refs[i].GetL();
  }
  sw.start("Expanding structure factors to P1 (fast symm)");
  SFUtil::ExpandToP1(hkl, F, *sg, P1SF);
  sw.stop();
  double vol = xapp.XFile().GetLattice().GetUnitCell().CalcVolume();
  BVFourier::MapInfo mi;
// init map
  const int mapX = (int)(au.Axes()[0].GetV()*resolution),
			mapY = (int)(au.Axes()[1].GetV()*resolution),
			mapZ = (int)(au.Axes()[2].GetV()*resolution);
  TArray3D<float> map(0, mapX-1, 0, mapY-1, 0, mapZ-1);
  sw.start("Calculating electron density map in P1 (Beevers-Lipson)");
  mi = BVFourier::CalcEDM(P1SF, map.Data, mapX, mapY, mapZ, vol);
  sw.stop();
//////////////////////////////////////////////////////////////////////////////////////////
  // map integration
  TArrayList<MapUtil::peak> Peaks;
  sw.start("Integrating P1 map: ");
  MapUtil::Integrate<float>(map.Data, mapX, mapY, mapZ, mi.minVal, mi.maxVal, mi.sigma, Peaks);
  sw.stop();
  int PointCount = mapX*mapY*mapZ;
  int minR = Round((3*1.5/(4*M_PI))*resolution);  // at least 1.5 A^3
  int minPointCount = Round(4*M_PI*minR*minR*minR/3.0);
  TArrayList< AnAssociation3<TCAtom*,double, int> > atoms (au.AtomCount());
  for( int i=0; i < au.AtomCount(); i++ )  {
    atoms[i].A() = &au.GetAtom(i);
    atoms[i].B() = 0;
    atoms[i].C() = 0;
    atoms[i].A()->SetTag(i);
  }
  for( int i=0; i < Peaks.Count(); i++ )  {
    const MapUtil::peak& peak = Peaks[i];
    if( peak.count >= minPointCount )  {
      vec3d cnt((double)peak.x/mapX, (double)peak.y/mapY, (double)peak.z/mapZ); 
      double pv = (double)peak.count*vol/PointCount;
      double ed = peak.summ/(pv*218);
      TCAtom* oa = uc.FindOverlappingAtom(cnt, 0.1);
      if( oa != NULL && oa->GetAtomInfo() != iQPeakIndex )  {
        atoms[oa->GetTag()].B() += ed;
        atoms[oa->GetTag()].C()++;
      }
    }
    continue;
  }
  double minEd = mi.sigma*3;
  for( int i=0; i < atoms.Count(); i++ )  {
    if( atoms[i].GetC() != 0 )  {
      double ed = atoms[i].GetB() / atoms[i].GetC();  
      if( fabs(ed) < minEd )  continue;
      TBasicApp::GetLog() << (olxstr("Atom type under consideration ") << atoms[i].GetA()->GetLabel() << 
          (ed < 0 ? olxstr(ed) : olxstr("+") << ed) << '\n');
    }

  }
  sw.print( xapp.GetLog(), &TLog::Info );
  E.SetRetVal(false);
  //au.InitData();
//  xapp.XFile().EndUpdate();
}

