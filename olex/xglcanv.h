/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xglCanvas_H
#define __olx_xglCanvas_H
#include "gxapp.h"
#include "wx/wx.h"
#include "wx/glcanvas.h"

class TGlCanvas: public wxGLCanvas  {
private:
  class TGXApp *FXApp;
  void OnMouseDown(wxMouseEvent& event);
  void OnMouseUp(wxMouseEvent& event);
  void OnMouseMove(wxMouseEvent& event);
  void OnMouseDblClick(wxMouseEvent& event);
  void OnMouseCaptureChange(wxMouseCaptureChangedEvent& event);
  void OnMouseWheel(wxMouseEvent& event);

  void OnKeyUp(wxKeyEvent& event);
  void OnKeyDown(wxKeyEvent& event);
  void OnChar(wxKeyEvent& event);

  int FMX{}, FMY{}; // mouse coordinates to process clicks
  short MouseButton;
  class TMainForm *FParent;
  wxGLContext* Context{};
  short EncodeEvent(const wxMouseState &evt, bool update_button=true);
public:
  TGlCanvas(TMainForm *parent, int* gl_attr, const wxWindowID id = -1,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize, long style = 0,
    const wxString& name = wxT("TGlCanvas"));
  ~TGlCanvas();

  void OnPaint(wxPaintEvent& event);
  void OnEraseBackground(wxEraseEvent& event);
  void OnMouse(wxMouseEvent& event);
  void XApp(TGXApp *XA);
  TGXApp *GetXApp() { return FXApp; }

  void Render();

  bool IsShownOnScreen();

  // static bool SwapBuffers();

  /* If default is true - NULL is returned,
  else if stereo is true - Olex2 will try to initialise stereo buffers
  */
  static olx_array_ptr<int> GetGlAttributes(bool _default, bool stereo,
    bool multisampling, short depth_bits);
  DECLARE_CLASS(wxGLCanvas)
};
#endif
