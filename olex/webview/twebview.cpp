#include "twebview.h"

BEGIN_EVENT_TABLE(TWebView, wxWebView)
  EVT_WEBVIEW_NAVIGATING(wxID_ANY, TWebView::OnNavigationRequest)
  EVT_WEBVIEW_LOADED(wxID_ANY, TWebView::OnDocumentLoaded)
  EVT_WEBVIEW_ERROR(wxID_ANY, TWebView::OnError)
END_EVENT_TABLE()

TWebView::TWebView()
    : AOlxCtrl(this), parentCell(nullptr)
{
  ShowTooltips = false;
  PageLoadRequested = false;
}

TWebView::~TWebView()
{
}


bool TWebView::Create(wxWindow *parent, wxWindowID id, const wxString &url,
                      const wxPoint &pos, const wxSize &size, long style,
                      const wxString &name) {
  return TWebView::Create(parent, id, url, pos, size, style, name);
}

void TWebView::SetFonts(const olxstr &normal, const olxstr &fixed)
{
  NormalFont = normal;
  FixedFont = fixed;
  // Inject JavaScript to set fonts
  wxString js = wxString::Format(
    "document.body.style.fontFamily = '%s';"
    "document.body.style.fontFamilyFixed = '%s';",
    (const wchar_t*)normal.c_str(), (const wchar_t*)fixed.c_str());
  RunScript(js);
}

void TWebView::GetFonts(olxstr &normal, olxstr &fixed)
{
  normal = NormalFont;
  fixed = FixedFont;
}

bool TWebView::GetShowTooltips() const
{
  return ShowTooltips;
}

void TWebView::SetShowTooltips(bool v, const olxstr &html_name)
{
  ShowTooltips = v;
}

bool TWebView::IsPageLoadRequested() const
{
  return PageLoadRequested;
}

void TWebView::LockPageLoad(const IOlxObject* caller)
{
  volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
  Locks.Add(caller, 0)++;
}

void TWebView::UnlockPageLoad(const IOlxObject* caller)
{
  volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
  const size_t pos = Locks.IndexOf(caller);
  if (pos == InvalidIndex) {
    throw TInvalidArgumentException(__OlxSourceInfo, "caller");
  }
  int lc = --Locks.GetValue(pos);
  if( lc < 0 ) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "not matching call to unlock");
  }
  if (lc == 0) {
    Locks.Delete(pos);
  }
}

bool TWebView::IsPageLocked() const
{
  volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
  return !Locks.IsEmpty();
}

bool TWebView::ProcessPageLoadRequest()
{
  if (PageLoadRequested) {
    PageLoadRequested = false;
    LoadPage(PageRequested.u_str());
    return true;
  }
  return false;
}

const olxstr& TWebView::GetHomePage() const
{
  return FileName;
}

void TWebView::SetHomePage(const olxstr& hp)
{
  FileName = hp;
}

bool TWebView::LoadPage(const wxString &File)
{
  if (IsPageLocked()) {
    PageRequested = (const wchar_t*)File.c_str();
    PageLoadRequested = true;
    return true;
  }
  TWebView::LoadURL(File);
  return true;
}

bool TWebView::UpdatePage(bool update_indices)
{
  TWebView::Reload();
  return true;
}

void TWebView::OnNavigationRequest(wxWebViewEvent& event)
{
  // For now, allow all navigation requests.
  // Later, we might want to filter or handle specific schemes/links.
  event.Skip();
}

void TWebView::OnDocumentLoaded(wxWebViewEvent& event)
{
  // Inject JavaScript to set fonts after the document is loaded
  SetFonts(NormalFont, FixedFont);
  event.Skip();
}

bool TWebView::Dispatch(int MsgId, short MsgSubId, const esdl::IOlxObject* Sender,
  const esdl::IOlxObject* Data, esdl::TActionQueue *)
{
  // Implement dispatch logic here if needed
  return false;
}

void TWebView::RunScript(const wxString& javascript)
{
  TWebView::RunScript(javascript);
}

void TWebView::AddImageHotspot(const wxString& imageId, int x, int y, int width, int height, const wxString& href)
{
  wxString js = wxString::Format(
    "var img = document.getElementById('%s');"
    "if (img) {"
    "  var map = img.useMap ? document.querySelector('map[name=\"' + img.useMap.substring(1) + '\"]') : null;"
    "  if (!map) {"
    "    var mapName = 'imagemap_' + imageId;"
    "    img.useMap = '#' + mapName;"
    "    map = document.createElement('map');"
    "    map.name = mapName;"
    "    document.body.appendChild(map);"
    "  }"
    "  var area = document.createElement('area');"
    "  area.shape = 'rect';"
    "  area.coords = '%d,%d,%d,%d';"
    "  area.href = '%s';"
    "  map.appendChild(area);"
    "}", imageId, x, y, x + width, y + height, href);
  RunScript(js);
}

void TWebView::AddImageCircleHotspot(const wxString& imageId, int x, int y, int radius, const wxString& href)
{
  wxString js = wxString::Format(
    "var img = document.getElementById('%s');"
    "if (img) {"
    "  var map = img.useMap ? document.querySelector('map[name=\"' + img.useMap.substring(1) + '\"]') : null;"
    "  if (!map) {"
    "    var mapName = 'imagemap_' + imageId;"
    "    img.useMap = '#' + mapName;"
    "    map = document.createElement('map');"
    "    map.name = mapName;"
    "    document.body.appendChild(map);"
    "  }"
    "  var area = document.createElement('area');"
    "  area.shape = 'circle';"
    "  area.coords = '%d,%d,%d';"
    "  area.href = '%s';"
    "  map.appendChild(area);"
    "}", imageId, x, y, radius, href);
  RunScript(js);
}

void TWebView::OnError(wxWebViewEvent& event)
{
  // This is where we will handle loading errors
  wxLogError("WebView error: %s", event.GetString());
}

void TWebView::SetImage(const wxImage& img)
{
  if (img.Ok()) {
    wxString base64;
    // Convert image to base64
    // This is a simplified example. In a real implementation, you would need a proper base64 encoding function.
    // For now, we'll assume one exists.
    // base64 = ConvertImageToBase64(img);

    wxString js = wxString::Format(
                                   "var img = document.getElementById('%s');"
                                   "if (img) {"
                                   "  img.src = 'data:image/png;base64,%s';"
                                   "}", GetName(), base64);
    RunScript(js);
  }
}

const olxstr& TWebView::GetSource() const
{
  return FileName;
}

void TWebView::SetSource(const olxstr& src)
{
  FileName = src;
  // In TWebView, changing the source might imply reloading the image
  // or updating the src attribute of an img element.
  wxString js = wxString::Format(
                                 "var img = document.getElementById('%s');"
                                 "if (img) {"
                                 "  img.src = '%s';"
                                 "}", GetName(), (const wchar_t*)src.c_str());
  RunScript(js);
}
