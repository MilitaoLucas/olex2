#include "sfutil.h"
#include "cif.h"
#include "hkl.h"
#include "estopwatch.h"

DefineFSFactory(ISF_Util, SF_Util)

//...........................................................................................
void SFUtil::ExpandToP1(const TArrayList<vec3i>& hkl, const TArrayList<compd>& F, const TSpaceGroup& sg, TArrayList<StructureFactor>& out)  {
  if( hkl.Count() != F.Count() )
    throw TInvalidArgumentException(__OlxSourceInfo, "hkl array and structure factors dimentions must be equal");
  ISF_Util* sf_util = fs_factory_ISF_Util(sg.GetName());
  if( sf_util == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid space group");
  out.SetCount( sf_util->GetSGOrder()* hkl.Count() );
  // test
  //smatd_list ml;
  //sg.GetMatrices(ml, mattAll);
  //const int ml_cnt = ml.Count();
  //for( int i=0; i < hkl.Count(); i++ )  {
  //  const int off = i*ml_cnt;
  //  for( int j=0; j < ml_cnt; j++ )  {
  //    const int ind = off+j;
  //    out[ind].hkl = hkl[i]*ml[j].r;
  //    out[ind].ps = ml[j].t.DotProd(hkl[i]);
  //    if( out[ind].ps != 0 )  {
  //      double ca=1, sa=0;
  //      SinCos(T_PI*out[ind].ps, &sa, &ca);
  //      out[ind].val = F[i]*compd(ca,sa);
  //    }
  //    else
  //      out[ind].val = F[i];
  //  }  
  //}
  //end test
  sf_util->Expand(hkl, F, out);
  delete sf_util;
}
//...........................................................................................
void SFUtil::FindMinMax(const TArrayList<StructureFactor>& F, vec3i& min, vec3i& max)  {
  min = vec3i(100, 100, 100);
  max = vec3i(-100, -100, -100);
  for( int i=0; i < F.Count(); i++ )  {
    if( F[i].hkl[0] > max[0] )  max[0] = F[i].hkl[0];
    if( F[i].hkl[0] < min[0] )  min[0] = F[i].hkl[0];

    if( F[i].hkl[1] > max[1] )  max[1] = F[i].hkl[1];
    if( F[i].hkl[1] < min[1] )  min[1] = F[i].hkl[1];

    if( F[i].hkl[2] > max[2] )  max[2] = F[i].hkl[2];
    if( F[i].hkl[2] < min[2] )  min[2] = F[i].hkl[2];
  }
}
//...........................................................................................
olxstr SFUtil::GetSF(TRefList& refs, TArrayList<compd>& F, 
                     short mapType, short sfOrigin, short scaleType)  {
  TXApp& xapp = TXApp::GetInstance();
  TStopWatch sw(__FUNC__);
  if( sfOrigin == sfOriginFcf )  {
    olxstr fcffn( TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "fcf") );
    if( !TEFile::Exists(fcffn) )  {
      fcffn = TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "fco");
      if( !TEFile::Exists(fcffn) )
        return "please load fcf file or make sure the one exists in current folder";
    }
    sw.start("Loading CIF");
    TCif cif;
    cif.LoadFromFile( fcffn );
    sw.stop();
    TCifLoop* hklLoop = cif.FindLoop("_refln");
    if( hklLoop == NULL )  {
      return "no hkl loop found";
    }
    sw.start("Extracting CIF data");
    int hInd = hklLoop->Table().ColIndex("_refln_index_h");
    int kInd = hklLoop->Table().ColIndex("_refln_index_k");
    int lInd = hklLoop->Table().ColIndex("_refln_index_l");
    // list 3, F
    int mfInd = hklLoop->Table().ColIndex("_refln_F_meas");
    int sfInd = hklLoop->Table().ColIndex("_refln_F_sigma");
    int aInd = hklLoop->Table().ColIndex("_refln_A_calc");
    int bInd = hklLoop->Table().ColIndex("_refln_B_calc");

    if( hInd == -1 || kInd == -1 || lInd == -1 || 
      mfInd == -1 || sfInd == -1 || aInd == -1 || bInd == -1  ) {
        return "list 3 fcf file is expected";
    }
    refs.SetCapacity( hklLoop->Table().RowCount() );
    F.SetCount( hklLoop->Table().RowCount() );
    for( int i=0; i < hklLoop->Table().RowCount(); i++ )  {
      TStrPObjList<olxstr,TCifLoopData*>& row = hklLoop->Table()[i];
      TReflection& ref = refs.AddNew(row[hInd].ToInt(), row[kInd].ToInt(), 
        row[lInd].ToInt(), row[mfInd].ToDouble(), row[sfInd].ToDouble());
      if( mapType == mapTypeDiff )  {
        const compd rv(row[aInd].ToDouble(), row[bInd].ToDouble());
        double dI = (ref.GetI() - rv.mod());
        F[i] = compd::polar(dI, rv.arg());
      }
      else if( mapType == mapType2OmC )  {
        const compd rv(row[aInd].ToDouble(), row[bInd].ToDouble());
        double dI = 2*ref.GetI() - rv.mod();
        F[i] = compd::polar(dI, rv.arg());
      }
      else if( mapType == mapTypeObs ) {
        const compd rv(row[aInd].ToDouble(), row[bInd].ToDouble());
        F[i] = compd::polar(ref.GetI(), rv.arg());
      }
      else  {
        F[i].SetRe(row[aInd].ToDouble());
        F[i].SetIm(row[bInd].ToDouble());
      }
    }
    sw.stop();
  }
  else  {  // olex2 calculated SF
    olxstr hklFileName( xapp.LocateHklFile() );
    if( !TEFile::Exists(hklFileName) )
      return "could not locate hkl file";
    double av = 0;
    sw.start("Loading/Filtering/Merging HKL");
    const TSpaceGroup& sg = xapp.XFile().GetLastLoaderSG();
    RefinementModel::HklStat ms = xapp.XFile().GetRM().GetFourierRefList<RefMerger::ShelxMerger>(sg, refs);
    F.SetCount(refs.Count());
    sw.start("Calculation structure factors");
    //xapp.CalcSF(refs, F);
    //sw.start("Calculation structure factors A");
    //fastsymm version is just about 10% faster...
    CalcSF(xapp.XFile(), refs, F, !sg.IsCentrosymmetric() );
    sw.start("Scaling structure factors");
    if( mapType != mapTypeCalc )  {
      // find a linear scale between F
      double a = 0, k = 1;
      if( scaleType == scaleRegression )  {
        CalcFScale(F, refs, k, a);
        TBasicApp::GetLog().Info(olxstr("Fc^2 = ") << k << "*Fo^2" << (a >= 0 ? " +" : " ") << a );
      }
      else  {  // simple scale on I/sigma > 3
        k = CalcFScale(F, refs);
        TBasicApp::GetLog().Info(olxstr("Fc^2 = ") << k << "*Fo^2");
      }
      const int f_cnt = F.Count();
      for( int i=0; i < f_cnt; i++ )  {
        double dI = refs[i].GetI() < 0 ? 0 : sqrt(refs[i].GetI());
        dI *= k;
        if( scaleType == scaleRegression )
          dI += a;
        if( mapType == mapTypeDiff )  {
          dI -= F[i].mod();
          F[i] = compd::polar(dI, F[i].arg());
        }
        else if( mapType == mapType2OmC )  {
          dI *= 2;
          dI -= F[i].mod();
          F[i] = compd::polar(dI, F[i].arg());
        }
        else if( mapType == mapTypeObs )  {
          F[i] = compd::polar(dI, F[i].arg());
        }
      }
    }
  }
  sw.print( xapp.GetLog(), &TLog::Info );
  return EmptyString;
}
//...........................................................................................
void SFUtil::PrepareCalcSF(const TAsymmUnit& au, double* U, TPtrList<cm_Element>& scatterers, TCAtomPList& alist)  {
  const mat3d& hkl2c = au.GetHklToCartesian();
  double quad[6];
  // the thermal ellipsoid scaling factors
  double BM[6] = {hkl2c[0].Length(), hkl2c[1].Length(), hkl2c[2].Length(), 0, 0, 0};
  BM[3] = 2*BM[1]*BM[2];
  BM[4] = 2*BM[0]*BM[2];
  BM[5] = 2*BM[0]*BM[1];
  BM[0] *= BM[0];
  BM[1] *= BM[1];
  BM[2] *= BM[2];
  
  TPtrList<TBasicAtomInfo> bais;
  for( int i=0; i < au.AtomCount(); i++ )  {
    TCAtom& ca = au.GetAtom(i);
    if( ca.IsDeleted() || ca.GetAtomInfo() == iQPeakIndex )  continue;
    int ind = bais.IndexOf( &ca.GetAtomInfo() );
    if( ind == -1 )  {
      cm_Element* elm;
      if( ca.GetAtomInfo() == iDeuteriumIndex ) // treat D as H
        elm = XElementLib::FindBySymbol("H");
      else 
        elm = XElementLib::FindBySymbol(ca.GetAtomInfo().GetSymbol());
      if( elm == NULL ) {
        delete [] U;
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("could not locate scatterer: ") << ca.GetAtomInfo().GetSymbol() );
      }
      scatterers.Add(elm);
      bais.Add( &ca.GetAtomInfo() );
      ind = scatterers.Count() - 1;
    }
    ca.SetTag(ind);
    ind = alist.Count()*6;
    alist.Add(&ca); 
    TEllipsoid* elp = ca.GetEllipsoid();
    if( elp != NULL )  {
      elp->GetQuad(quad);  // default is Ucart
      au.UcartToUcif(quad);
      for( int k=0; k < 6; k++ )
        U[ind+k] = -TQ_PI*quad[k]*BM[k];
    }
    else  {
      U[ind] = ca.GetUiso();//*ca.GetUiso();
      U[ind] *= -EQ_PI;
    }
  }
}
//...........................................................................................
void SFUtil::CalcSF(const TXFile& xfile, const TRefList& refs, TArrayList<TEComplex<double> >& F, bool useFpFdp)  {
  TSpaceGroup* sg = NULL;
  try  { sg = &xfile.GetLastLoaderSG();  }
  catch(...)  {
    throw TFunctionFailedException(__OlxSourceInfo, "unknown space group");
  }
  ISF_Util* sf_util = fs_factory_ISF_Util(sg->GetName());
  if( sf_util == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid space group");
  TAsymmUnit& au = xfile.GetAsymmUnit();
  double *U = new double[6*au.AtomCount() + 1];
  TPtrList<TCAtom> alist;
  TPtrList<cm_Element> scatterers;
  PrepareCalcSF(au, U, scatterers, alist);

  sf_util->Calculate(
    xfile.GetRM().expl.GetRadiationEnergy(), 
    refs, 
    au.GetHklToCartesian(), 
    F, scatterers, 
    alist, 
    U, 
    useFpFdp
  );
  delete sf_util;
  delete [] U;
}
//...........................................................................................
void SFUtil::CalcSF(const TXFile& xfile, const TRefPList& refs, TArrayList<TEComplex<double> >& F, bool useFpFdp)  {
  TSpaceGroup* sg = NULL;
  try  { sg = &xfile.GetLastLoaderSG();  }
  catch(...)  {
    throw TFunctionFailedException(__OlxSourceInfo, "unknown space group");
  }
  ISF_Util* sf_util = fs_factory_ISF_Util(sg->GetName());
  if( sf_util == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid space group");
  TAsymmUnit& au = xfile.GetAsymmUnit();
  double *U = new double[6*au.AtomCount() + 1];
  TPtrList<TCAtom> alist;
  TPtrList<cm_Element> scatterers;
  PrepareCalcSF(au, U, scatterers, alist);

  sf_util->Calculate(
    xfile.GetRM().expl.GetRadiationEnergy(), 
    refs, au.GetHklToCartesian(), 
    F, scatterers, 
    alist, 
    U, 
    useFpFdp
  );
  delete sf_util;
  delete [] U;
}

