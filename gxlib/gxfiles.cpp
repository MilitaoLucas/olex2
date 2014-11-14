/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "gxfiles.h"
#include "gxapp.h"

TGXFile::TGXFile(XObjectProvider & op)
: TXFile(op)
{
  olxstr extra = (op.app.OverlayedXFileCount() == 0 ? EmptyString() :
    olxstr(op.app.OverlayedXFileCount()));
  DUnitCell = new TDUnitCell(op.app.GetRenderer(),
    olxstr("DUnitCell") << extra);
  DUnitCell->SetVisible(false);
  op.app.OnObjectsCreate.Add(this, GXFILE_EVT_OBJETSCREATE);
}
//..............................................................................
TGXFile::~TGXFile() {
  ((XObjectProvider &)GetLattice().GetObjects()).app
    .OnObjectsCreate.Remove(this);
  delete DUnitCell;
}
//..............................................................................
bool TGXFile::Dispatch(int MsgId, short MsgSubId, const IOlxObject *Sender,
  const IOlxObject *Data, TActionQueue *q)
{
  if (MsgId == GXFILE_EVT_OBJETSCREATE) {
    if (MsgSubId == msiExecute) {
      DUnitCell->Create();
    }
  }
  return TXFile::Dispatch(MsgId, MsgSubId, Sender, Data, q);
}
//..............................................................................
//..............................................................................
//..............................................................................
XObjectProvider::XObjectProvider(TGXApp &app) :
  ASObjectProvider(
  *(new TXObjectProvider<TSAtom, TXAtom>(app.GetRenderer())),
  *(new TXObjectProvider<TSBond, TXBond>(app.GetRenderer())),
  *(new TXObjectProvider<TSPlane, TXPlane>(app.GetRenderer()))),
  app(app)
{}
//..............................................................................

