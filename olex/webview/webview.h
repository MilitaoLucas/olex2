#ifndef __olx_webview_H
#define __olx_webview_H

#include "wx/webview.h"
#include "widgetcellext.h"
#include "estrlist.h"
#include "bapp.h"
#include "paramlist.h"
#include "actions.h"
#include "../wininterface.h"
#include "edict.h"
#include "../ctrls.h"

class TWebView : public wxWebView, public AEventsDispatcher, public AOlxCtrl
{
public:
  TWebView();
  virtual ~TWebView();

  virtual bool Create(wxWindow *parent, wxWindowID id = wxID_ANY,
                       const wxString &url = wxWebViewDefaultURLStr,
                       const wxPoint &pos = wxDefaultPosition,
                       const wxSize &size = wxDefaultSize, long style = 0,
                       const wxString &name = wxWebViewNameStr) override;

  // Pure virtual functions from wxWebView
  virtual wxString GetCurrentTitle() const override { return wxWebView::GetCurrentTitle(); }
  virtual wxString GetCurrentURL() const override { return wxWebView::GetCurrentURL(); }
  virtual bool IsBusy() const override { return wxWebView::IsBusy(); }
  virtual bool IsEditable() const override { return wxWebView::IsEditable(); }
  virtual void LoadURL(const wxString& url) override { wxWebView::LoadURL(url); }
  virtual void Print() override { wxWebView::Print(); }
  virtual void RegisterHandler(wxSharedPtr<wxWebViewHandler> handler) override { wxWebView::RegisterHandler(handler); }
  virtual void Reload(wxWebViewReloadFlags flags = wxWEBVIEW_RELOAD_DEFAULT) override { wxWebView::Reload(flags); }
  virtual void SetEditable(bool enable = true) override { wxWebView::SetEditable(enable); }
  virtual void Stop() override { wxWebView::Stop(); }
  virtual bool CanGoBack() const override { return wxWebView::CanGoBack(); }
  virtual bool CanGoForward() const override { return wxWebView::CanGoForward(); }
  virtual void GoBack() override { wxWebView::GoBack(); }
  virtual void GoForward() override { wxWebView::GoForward(); }
  virtual void ClearHistory() override { wxWebView::ClearHistory(); }
  virtual void EnableHistory(bool enable = true) override { wxWebView::EnableHistory(enable); }
  virtual wxVector<wxSharedPtr<wxWebViewHistoryItem> > GetBackwardHistory() override { return wxWebView::GetBackwardHistory(); }
  virtual wxVector<wxSharedPtr<wxWebViewHistoryItem> > GetForwardHistory() override { return wxWebView::GetForwardHistory(); }
  virtual void LoadHistoryItem(wxSharedPtr<wxWebViewHistoryItem> item) override { wxWebView::LoadHistoryItem(item); }
  virtual bool CanSetZoomType(wxWebViewZoomType type) const override { return wxWebView::CanSetZoomType(type); }
  virtual float GetZoomFactor() const override { return wxWebView::GetZoomFactor(); }
  virtual wxWebViewZoomType GetZoomType() const override { return wxWebView::GetZoomType(); }
  virtual void SetZoomFactor(float zoom) override { wxWebView::SetZoomFactor(zoom); }
  virtual void SetZoomType(wxWebViewZoomType zoomType) override { wxWebView::SetZoomType(zoomType); }
  virtual bool CanUndo() const override { return wxWebView::CanUndo(); }
  virtual bool CanRedo() const override { return wxWebView::CanRedo(); }
  virtual void Undo() override { wxWebView::Undo(); }
  virtual void Redo() override { wxWebView::Redo(); }
  virtual void* GetNativeBackend() const override { return wxWebView::GetNativeBackend(); }
  virtual void DoSetPage(const wxString& html, const wxString& baseUrl) override { wxWebView::DoSetPage(html, baseUrl); }

  // Pure virtual function from AEventsDispatcher
  virtual bool Dispatch(int MsgId, short MsgSubId, const esdl::IOlxObject* Sender,
    const esdl::IOlxObject* Data, esdl::TActionQueue *) override;

  // Mimic THtml interface
  void SetFonts(const olxstr &normal, const olxstr &fixed);
  void GetFonts(olxstr &normal, olxstr &fixed);
  bool GetShowTooltips() const;
  void SetShowTooltips(bool v, const olxstr &html_name = EmptyString());
  bool IsPageLoadRequested() const;
  void LockPageLoad(const IOlxObject* caller);
  void UnlockPageLoad(const IOlxObject* caller);
  bool IsPageLocked() const;
  bool ProcessPageLoadRequest();
  const olxstr& GetHomePage() const;
  void SetHomePage(const olxstr& hp);
  bool LoadPage(const wxString &File);
  bool UpdatePage(bool update_indices = true);
  void SetParentCell(THtmlWidgetCell *cell) { parentCell = cell; }
  THtmlWidgetCell *GetParentCell() { return parentCell; }
  // wxHtmlCell *GetRootCell() { return m_Cell; } // m_Cell is protected in wxHtmlWindow, need to find an alternative for wxWebView

  void RunScript(const wxString& javascript);
  void AddImageHotspot(const wxString& imageId, int x, int y, int width, int height, const wxString& href);
  void AddImageCircleHotspot(const wxString& imageId, int x, int y, int radius, const wxString& href);

  void SetImage(const wxImage& img);
  const olxstr& GetSource() const;
  void SetSource(const olxstr& src);

   // Event handlers
   void OnNavigationRequest(wxWebViewEvent& event);
  void OnDocumentLoaded(wxWebViewEvent& event);
  void OnError(wxWebViewEvent& event);

private:
  THtmlWidgetCell *parentCell; // Mimic THtml's parentCell

private:
  olxstr NormalFont, FixedFont;
  bool ShowTooltips;
  bool PageLoadRequested;
  olxstr PageRequested;
  olxdict<const IOlxObject*, int, TPointerComparator> Locks;
  olxstr FileName;
  // For THtmlImageCell replacement
  // We might need to store image map data here or handle it purely in JS

  DECLARE_EVENT_TABLE()
};

#endif // __olx_webview_H
