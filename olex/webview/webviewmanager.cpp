#include "webviewmanager.h"

TWebViewManager::TWebViewManager()
{
}

TWebViewManager::~TWebViewManager()
{
}

TWebView* TWebViewManager::CreateWebView(wxWindow* parent, wxWindowID id)
{
  TWebView* webview = new TWebView();
  webview->Create(parent, id);
  return webview;
}
