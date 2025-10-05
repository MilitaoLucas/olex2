#ifndef __olx_webview_manager_H
#define __olx_webview_manager_H

#include "twebview.h"

class TWebViewManager {
public:
  TWebViewManager();
  ~TWebViewManager();

  TWebView* CreateWebView(wxWindow* parent, wxWindowID id = wxID_ANY);

private:
  // Add any necessary private members
};

#endif // __olx_webview_manager_H
