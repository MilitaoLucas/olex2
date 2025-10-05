/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_htmlmanager_H
#define __olx_htmlmanager_H
#include "webview/twebview.h" // Use TWebView instead of THtml
#include "webview/webviewmanager.h" // Use TWebViewManager

class THtmlManager : public AActionHandler {
  wxWindow *mainWindow;
  TActionQList Actions;
  virtual bool Enter(const IOlxObject *, const IOlxObject *, TActionQueue *);
  virtual bool Exit(const IOlxObject *, const IOlxObject *, TActionQueue *);
  virtual bool Execute(const IOlxObject *, const IOlxObject *, TActionQueue *);
  bool destroyed;
protected:
  // library
  struct Control {
    TWebView *html; // Changed from THtml*
    AOlxCtrl *ctrl;
    wxWindow *wnd;
    olxstr ctrl_name;
    Control(TWebView *h, AOlxCtrl *c, wxWindow *w, const olxstr &cn) // Changed from THtml*
      : html(h), ctrl(c), wnd(w), ctrl_name(cn) {}
    };
  // needed = 0 - nothing, 1 - AOlxCtrl, 2 - wxWidow, 3 - both
  Control FindControl(const olxstr &name, TMacroData& me,
    short needed, const char* location);
  bool SetState(const TStrObjList &Params, TMacroData &E);
public:
  TWebView *main; // Changed from THtml*
  struct TPopupData  {
    TDialog *Dialog;
    TWebView *Html; // Changed from THtml*
  };
  struct DestructionLocker {
    THtmlManager &manager;
    wxWindow *wnd;
    IOlxObject *sender;
    DestructionLocker(THtmlManager &manager, wxWindow *wnd, IOlxObject *sender)
      : manager(manager), wnd(wnd), sender(sender)
    {
      manager.LockWindowDestruction(wnd, sender);
    }
    ~DestructionLocker() {
      manager.UnlockWindowDestruction(wnd, sender);
    }
  };
  THtmlManager(wxWindow *mainWindow);
  ~THtmlManager();
  void Destroy();
  void InitialiseMain(long flags);
  void ProcessPageLoadRequests();
  void ClearPopups();
  DestructionLocker LockDestruction(wxWindow *wnd, IOlxObject *sender) {
    return DestructionLocker(*this, wnd, sender);
  }
  void LockWindowDestruction(wxWindow* wnd, const IOlxObject* caller);
  void UnlockWindowDestruction(wxWindow* wnd, const IOlxObject* caller);
  TWebView* FindHtml(const olxstr& name) const; // Changed from THtml*
  TPopupData &NewPopup(TDialog *owner, const olxstr &name, long flags=4);
  TLibrary* ExportLibrary(const olxstr &name="html");

  olxstr_dict<TPopupData*, true> Popups;
  TActionQueue &OnLink;
};

#endif
