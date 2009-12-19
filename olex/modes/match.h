#ifndef __OLX_MATCH_MODE_H
#define __OLX_MATCH_MODE_H


class TMatchMode : public AMode  {
  TXAtomPList AtomsToMatch;
protected:
  void FitAtoms();
public:
  TMatchMode(size_t id) : AMode(id)  {}
  bool Init(TStrObjList &Cmds, const TParamList &Options) {
    AtomsToMatch.Clear();
    TGlXApp::GetMainForm()->SetUserCursor( '0', "<M>");
    return true;
  }
  virtual bool OnObject(AGDrawObject &obj)  {
    if( EsdlInstanceOf( obj, TXAtom) )  {
      TXAtom &XA = (TXAtom&)obj;
      AtomsToMatch.Add( &XA );
      TGlXApp::GetMainForm()->SetUserCursor( AtomsToMatch.Count(), "<M>");
      FitAtoms();
      return true;
    }
    return false;
  }
  virtual void OnGraphicsDestroy()  {
    AtomsToMatch.Clear();
    TGlXApp::GetMainForm()->SetUserCursor( '0', "<M>");
  }
  virtual bool OnKey(int keyId, short shiftState)  {
    if( shiftState == 0 && keyId == WXK_ESCAPE )  {
      if( AtomsToMatch.IsEmpty() )  return false;
      AtomsToMatch.Delete( AtomsToMatch.Count()-1 );
      TGlXApp::GetMainForm()->SetUserCursor( AtomsToMatch.Count(), "<M>");
      return true;
    }
    return false;
  }
};

void TMatchMode::FitAtoms()  {
  if( (AtomsToMatch.Count() % 2) != 0 ||  AtomsToMatch.Count() < 2 )  return;
  TNetwork* netA = NULL, *netB = NULL;
  TSAtomPList atomsB;
  if( AtomsToMatch.Count() >= 2 )  {
    netA = &AtomsToMatch[0]->Atom().GetNetwork();
    netB = &AtomsToMatch[1]->Atom().GetNetwork();
    if( netA->GetLattice() != netB->GetLattice() )  {  //match lattices
      atomsB.SetCapacity( netB->GetLattice().AtomCount() );
      for( size_t i=0; i < netB->GetLattice().AtomCount(); i++ )
        atomsB.Add( &netB->GetLattice().GetAtom(i) );
    }
    else  {
      atomsB.SetCapacity( netB->NodeCount() );
      for( size_t i=0; i < netB->NodeCount(); i++ )
        atomsB.Add( &netB->Node(i) );
    }
  }
  if( netA == netB )  {
    TBasicApp::GetLog().Error("Atoms belong to the same fragment");
    AtomsToMatch.Clear();
    TGlXApp::GetMainForm()->SetUserCursor( AtomsToMatch.Count(), "<M>");
    return;
  }
  if( &netA->GetLattice() == &netB->GetLattice() )  {
    for( size_t i=2; i < AtomsToMatch.Count(); i+=2 )  {
      if( AtomsToMatch[i]->Atom().GetNetwork() != netA ||
          AtomsToMatch[i+1]->Atom().GetNetwork() != netB )  {

        if( AtomsToMatch[i]->Atom().GetNetwork() == netB &&
            AtomsToMatch[i+1]->Atom().GetNetwork() == netA )  {
          AtomsToMatch.Swap(i, i+1);
        }
        else  {
          AtomsToMatch.Clear();
          TGlXApp::GetMainForm()->SetUserCursor( AtomsToMatch.Count(), "<M>");
          return;
        }
      }
    }
  }
  else  {
    for( size_t i=2; i < AtomsToMatch.Count(); i+=2 )  {
      if( AtomsToMatch[i]->Atom().GetNetwork().GetLattice() != netA->GetLattice() ||
          AtomsToMatch[i+1]->Atom().GetNetwork().GetLattice() != netB->GetLattice() )  {

        if( AtomsToMatch[i]->Atom().GetNetwork().GetLattice() == netB->GetLattice() &&
            AtomsToMatch[i+1]->Atom().GetNetwork().GetLattice() == netA->GetLattice() )  {
          AtomsToMatch.Swap(i, i+1);
        }
        else  {
          AtomsToMatch.Clear();
          TGlXApp::GetMainForm()->SetUserCursor( AtomsToMatch.Count(), "<M>");
          return;
        }
      }
    }
  }

  if( AtomsToMatch.Count() == 2 )  {
    vec3d origin( AtomsToMatch[1]->Atom().crd() );
    for( size_t i=0; i < atomsB.Count(); i++ )
      (atomsB[i]->crd() -= origin) += AtomsToMatch[0]->Atom().crd();
    TNetPList na;
    TNetPList nb;
    if( netA->GetLattice() != netB->GetLattice() )  {
      for( size_t i=0; i < netA->GetLattice().FragmentCount(); i++ )
        na.Add( &netA->GetLattice().GetFragment(i) );
      for( size_t i=0; i < netB->GetLattice().FragmentCount(); i++ )
        nb.Add( &netB->GetLattice().GetFragment(i) );
    }
    else  {
      na.Add(netA);
      nb.Add(netB);
    }
    TGlGroup& ga = TGlXApp::GetGXApp()->GroupFragments(na, "FragmentA");
    TGlGroup& gb = TGlXApp::GetGXApp()->GroupFragments(nb, "FragmentB");
    TGlXApp::GetGXApp()->SelectAll(false);
    TGlMaterial nm;
    nm.SetFlags(sglmAmbientF);
    nm.AmbientF = 0x0000ff00;
    if( &ga != NULL && ga.IsDefaultColor() )
      ga.SetGlM(nm);
    nm.AmbientF = 0x000000ff;
    if( &gb != NULL && gb.IsDefaultColor() )
      gb.SetGlM(nm);

    TGlXApp::GetGXApp()->CenterView();
  }
  if( AtomsToMatch.Count() == 4 )  {
    vec3d orgn = AtomsToMatch[0]->Atom().crd();
    vec3d vec1 = AtomsToMatch[2]->Atom().crd() - orgn;
    vec3d vec2 = AtomsToMatch[3]->Atom().crd() - orgn;
    vec3d rv = vec1.XProdVec( vec2 ).Normalise();
    double ca = vec1.CAngle( vec2 );
    mat3d rm;
    CreateRotationMatrix(rm, rv, ca);

    for( size_t i=0; i < atomsB.Count(); i++ )
      atomsB[i]->crd() = rm * atomsB[i]->crd();

    orgn = AtomsToMatch[1]->Atom().crd();
    for( size_t i=0; i < atomsB.Count(); i++ )
      (atomsB[i]->crd() -= orgn) += AtomsToMatch[0]->Atom().crd();
    TGlXApp::GetGXApp()->UpdateBonds();
    TGlXApp::GetGXApp()->CenterView();
    //FXApp->CreateObjects();
  }
  if( AtomsToMatch.Count() == 6 )  {
    vec3d rv( (AtomsToMatch[0]->Atom().crd() - AtomsToMatch[2]->Atom().crd()).Normalise() );
    vec3d v1( (AtomsToMatch[4]->Atom().crd() - AtomsToMatch[2]->Atom().crd()) );
    vec3d v2( (AtomsToMatch[5]->Atom().crd() - AtomsToMatch[2]->Atom().crd()) );
    //v1 = rv.XProdVec(v1);
    //v2 = rv.XProdVec(v2);
    v1 = rv.Normal(v1);
    v2 = rv.Normal(v2);
    rv = v1.XProdVec(v2).Normalise();  // replacing the rotation vector for the one with correct orientation
    double ca = v1.CAngle( v2 );
    mat3d rm;
    CreateRotationMatrix(rm, rv, ca);
    for( size_t i=0; i < atomsB.Count(); i++ ) 
      atomsB[i]->crd() = rm * atomsB[i]->crd();

    vec3d orgn = AtomsToMatch[1]->Atom().crd();
    for( size_t i=0; i < atomsB.Count(); i++ )
      (atomsB[i]->crd() -= orgn) += AtomsToMatch[0]->Atom().crd();
    TGlXApp::GetGXApp()->UpdateBonds();
    TGlXApp::GetGXApp()->CenterView();
    //FXApp->CreateObjects();
  }
}

#endif
