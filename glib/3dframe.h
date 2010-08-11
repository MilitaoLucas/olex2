// (c) O Dolomanov 2010
#ifndef __olx_gl_3dframe_H
#define __olx_gl_3dframe_H
#include "glmousehandler.h"
#include "glrender.h"
#include "styles.h"
#include "glprimitive.h"

class A3DFrameCtrl  {
public:
  virtual bool OnTranslate(size_t sender, const vec3d& t) = 0;
  virtual bool OnRotate(size_t sender, const vec3d& vec, double angle) = 0;
  virtual bool OnZoom(size_t sender, double zoom, bool inc) = 0;
  virtual void SetBasis() const = 0;
};
// frame 'face' - the control sphere of the face
class TFaceCtrl : public AGlMouseHandlerImp {
  vec3d &A, &B, &C, &D, &N;
  A3DFrameCtrl& ParentCtrl;
  TGlPrimitive* pPrimitive;
  size_t Id;
protected:
  virtual bool DoTranslate(const vec3d& t)  {
    ParentCtrl.OnTranslate(GetId(), t);
    return true;
  }
  virtual bool DoRotate(const vec3d& vec, double angle)  {
    return ParentCtrl.OnRotate(GetId(), vec, angle);
  }
  virtual bool DoZoom(double zoom, bool inc)  {
    return ParentCtrl.OnZoom(GetId(), zoom, inc);
  }
public:
  TFaceCtrl(TGlRenderer& prnt, const olxstr& cName, size_t _Id, vec3d& _A, vec3d& _B, vec3d& _C, vec3d& _D, vec3d& _N,
    A3DFrameCtrl& _ParentCtrl) : AGlMouseHandlerImp(prnt, cName), ParentCtrl(_ParentCtrl),
    Id(_Id), A(_A), B(_B), C(_C), D(_D), N(_N)
  {
    SetMoveable(true);
    SetRoteable(true);
  }
  size_t GetId() const {  return Id;  }
  TGlPrimitive& GetPrimitive() const {  return *pPrimitive;  }
  void Create(const olxstr& cName, const ACreationParams* cpar);
  virtual bool GetDimensions(vec3d&, vec3d&)  {  return false;  }
  virtual bool Orient(class TGlPrimitive&)  {
    ParentCtrl.SetBasis();
    olx_gl::translate((A+B+C+D)/4);
    return false;
  }
  vec3d& GetA()  {  return A;  }
  vec3d& GetB()  {  return B;  }
  vec3d& GetC()  {  return C;  }
  vec3d& GetD()  {  return D;  }
  const vec3d& GetN() const {  return N;  }
};
// the frame class itself...
class T3DFrameCtrl : public AGlMouseHandlerImp, public A3DFrameCtrl {
  TTypeList<TFaceCtrl> Faces;
  vec3d edges[8], norms[6], Center;
protected:
  virtual bool DoTranslate(const vec3d& t)  {
    Center += t;
    return true;
  }
  virtual bool DoRotate(const vec3d& vec, double angle);
  virtual bool DoZoom(double zoom, bool inc);
  virtual bool OnTranslate(size_t sender, const vec3d& t);
  virtual bool OnRotate(size_t sender, const vec3d& vec, double angle)  {
    return DoRotate(Faces[sender].GetN(), angle);
  }
  virtual bool OnZoom(size_t sender, double zoom, bool inc)  {
    return true;
  }
public:
  T3DFrameCtrl(TGlRenderer& prnt, const olxstr& cName);
  void Create(const olxstr& cName, const ACreationParams* cpar);
  virtual bool GetDimensions(vec3d& _mn, vec3d& _mx)  {
    for( int i=0; i < 8; i++ )
      vec3d::UpdateMinMax(edges[i], _mn, _mx);
    return true;
  }
  virtual void SetBasis() const {  olx_gl::translate(Center);  }
  virtual bool Orient(TGlPrimitive&)  {
    SetBasis();
    olx_gl::begin(GL_QUADS);
    for( int i=0; i < 6; i++ )  {
      olx_gl::normal(Faces[i].GetN());
      olx_gl::vertex(Faces[i].GetA());
      olx_gl::vertex(Faces[i].GetB());
      olx_gl::vertex(Faces[i].GetC());
      olx_gl::vertex(Faces[i].GetD());
    }
    olx_gl::end();
    return true;
  }
};

#endif
