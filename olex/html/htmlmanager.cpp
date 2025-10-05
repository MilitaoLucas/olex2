/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "htmlmanager.h"
#include "htmlswitch.h"
#include "fsext.h"
#include "utf8file.h"
#include "olxstate.h"
#include "webview/twebview.h" // Include TWebView
#include "webview/webviewmanager.h" // Include TWebViewManager
// #include "imgcellext.h" // THtmlImageCell is replaced by TWebView functionality

#define this_InitFuncD(funcName, argc, desc) \
  (Library).Register(\
    new TFunction<THtmlManager>(\
      this, &THtmlManager::fun##funcName, #funcName, argc, desc))

THtmlManager::THtmlManager(wxWindow *mainWindow)
  : mainWindow(mainWindow),
    destroyed(false),
    main(0), // main will be a TWebView*
    OnLink(Actions.New("OnLink"))
{
  AActionHandler::SetToDelete(false);
}
//.............................................................................
THtmlManager::~THtmlManager() {
  Destroy();
}
//.............................................................................
void THtmlManager::Destroy() {
  if (destroyed) {
    return;
  }
  destroyed = true;
  if (main != 0) {
    // main->OnLink.Remove(this); // TWebView does not have OnLink directly
    main->Destroy();
  }
  ClearPopups();
}
//.............................................................................
void THtmlManager::InitialiseMain(long flags) {
  main = new TWebView(); // Create TWebView instance
  main->Create(mainWindow, wxID_ANY, "about:blank", wxDefaultPosition, wxDefaultSize, flags);
  // main->OnLink.Add(this); // TWebView does not have OnLink directly, events are handled differently
  // main->ShowScrollbars(wxSHOW_SB_ALWAYS, wxSHOW_SB_ALWAYS); // Not directly applicable to wxWebView
}
//.............................................................................
void THtmlManager::ProcessPageLoadRequests() {
  if (main != 0 && main->IsPageLoadRequested() && !main->IsPageLocked()) {
    main->ProcessPageLoadRequest();
  }
  try {
    for (size_t pi=0; pi < Popups.Count(); pi++) {
      if (Popups.GetValue(pi)->Html->IsPageLoadRequested() &&
        !Popups.GetValue(pi)->Html->IsPageLocked() )
      {
        Popups.GetValue(pi)->Html->ProcessPageLoadRequest();
      }
    }
  }
  catch(const TExceptionBase &e)  {
    TBasicApp::NewLogEntry(logExceptionTrace) << e;
  }
}
//.............................................................................
void THtmlManager::ClearPopups() {
  for (size_t i = 0; i < Popups.Count(); i++) {
    // Popups.GetValue(i)->Html->OnLink.Remove(this); // TWebView does not have OnLink directly
    Popups.GetValue(i)->Html->Destroy();
    Popups.GetValue(i)->Dialog->Destroy();
    delete Popups.GetValue(i);
  }
  Popups.Clear();
}
//.............................................................................
void THtmlManager::LockWindowDestruction(wxWindow* wnd,
  const IOlxObject* caller)
{
  if (wnd == main) {
    main->LockPageLoad(caller);
  }
  else  {
    for (size_t i=0; i < Popups.Count(); i++) {
      if (Popups.GetValue(i)->Html == wnd) {
        Popups.GetValue(i)->Html->LockPageLoad(caller);
        break;
      }
    }
  }
}
//.............................................................................
void THtmlManager::UnlockWindowDestruction(wxWindow* wnd,
  const IOlxObject* caller)
{
  if (wnd == main) {
    main->UnlockPageLoad(caller);
  }
  else {
    for (size_t i = 0; i < Popups.Count(); i++) {
      if (Popups.GetValue(i)->Html == wnd) {
        Popups.GetValue(i)->Html->UnlockPageLoad(caller);
        break;
      }
    }
  }
}
//.............................................................................
TWebView* THtmlManager::FindHtml(const olxstr& name) const {
  if (name.IsEmpty()) {
    return main;
  }
  TPopupData *pd = Popups.Find(name, 0);
  if (pd == 0) {
    // For now, we don't support finding TWebView instances within other TWebView instances
    // This functionality might need to be re-evaluated or re-implemented if required.
    return 0;
  }
  else {
    return pd->Html;
  }
}
//.............................................................................
bool THtmlManager::Enter(const IOlxObject *sender, const IOlxObject *data,
  TActionQueue *)
{
  if (sender != 0) {
    const wxWindow* wx = dynamic_cast<const wxWindow*>(sender);
    TWebView *html = dynamic_cast<TWebView*>(const_cast<wxWindow*>(wx)); // Changed from wx->GetParent()
    if (html != 0) {
      volatile THtmlManager::DestructionLocker dm =
        LockDestruction(html, this);
      OnLink.Enter(sender, data);
      return true;
    }
  }
  OnLink.Enter(sender, data);
  return true;
}

//.............................................................................
bool THtmlManager::Exit(const IOlxObject *sender, const IOlxObject *data,
  TActionQueue *)
{
  if (sender != 0) {
    const wxWindow *wx = dynamic_cast<const wxWindow*>(sender);
    TWebView *html = dynamic_cast<TWebView*>(const_cast<wxWindow*>(wx)); // Changed from wx->GetParent()
    if (html != 0) {
      volatile THtmlManager::DestructionLocker dm =
        LockDestruction(html, this);
      OnLink.Exit(sender, data);
      return true;
    }
  }
  OnLink.Exit(sender, data);
  return true;
}
//.............................................................................
bool THtmlManager::Execute(const IOlxObject *sender, const IOlxObject *data,
  TActionQueue *)
{
  olxstr dt;
  if (data != 0 && data->Is<olxstr>()) {
    dt = *(olxstr *)data;
    size_t ind = dt.FirstIndexOf('%');
    while (ind != InvalidIndex && ((ind + 2) < dt.Length()) &&
      olxstr::o_ishexdigit(dt[ind + 1]) &&
      olxstr::o_ishexdigit(dt[ind + 2]))
    {
      int val = dt.SubString(ind + 1, 2).RadInt<int>(16);
      dt.Delete(ind, 3);
      dt.Insert(val, ind);
      ind = dt.FirstIndexOf('%');
    }
    data = &dt;
  }
  if (sender != 0) {
    const wxWindow *wx = dynamic_cast<const wxWindow*>(sender);
    TWebView *html = dynamic_cast<TWebView*>(const_cast<wxWindow*>(wx)); // Changed from wx->GetParent()
    if (html != 0) {
      volatile THtmlManager::DestructionLocker dm =
        LockDestruction(html, this);
      olxstr cmd = *(const olxstr *)data;
      if (!cmd.IsEmpty()) {
        TBasicApp::PostAction(new olxCommandAction(cmd));
        //OnLink.Execute(sender, data);
      }
      return true;
    }
  }
  OnLink.Execute(sender, data);
  return true;
}
//.............................................................................
THtmlManager::TPopupData &THtmlManager::NewPopup(TDialog *owner,
  const olxstr &name, long flags)
{
  TPopupData *pd = Popups.Find(name, 0);
  if (pd == 0) {
    pd = Popups.Add(name, new TPopupData);
    pd->Dialog = owner;
    pd->Html = new TWebView(); // Create TWebView instance
    pd->Html->Create(owner, wxID_ANY, "about:blank", wxDefaultPosition, wxDefaultSize, flags);
    // pd->Html->OnLink.Add(this); // TWebView does not have OnLink directly
  }
  return *pd;
}
//.............................................................................
//.............................................................................
//.............................................................................
//.............................................................................
//.............................................................................
//.............................................................................
//.............................................................................
//.............................................................................
//.............................................................................
TLibrary *THtmlManager::ExportLibrary(const olxstr &name) {
  TLibrary &Library = *(new TLibrary(name));
  return &Library;
}

//.............................................................................
