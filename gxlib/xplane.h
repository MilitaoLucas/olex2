#ifndef xplaneH
#define xplaneH
#include "gxbase.h"

#include "glrender.h"
#include "gdrawobject.h"

#include "splane.h"

BeginGxlNamespace()

class TXPlane: public AGDrawObject  {
private:
  TSPlane *FPlane;
public:
  TXPlane(TGlRenderer& Render, const olxstr& collectionName, TSPlane *Plane);
  void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);
  virtual ~TXPlane()  {}

  inline TSPlane& Plane() const {  return *FPlane; }

  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &Max, vec3d &Min)  {  return false;  }
  void ListPrimitives(TStrList &List) const;

  bool OnMouseDown(const IEObject *Sender, const TMouseData *Data){  return true; }
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data)  {  return false; }
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data){  return false; }

  inline bool IsDeleted()  const {  return AGDrawObject::IsDeleted(); }
  void SetDeleted(bool v){  AGDrawObject::SetDeleted(v);  FPlane->SetDeleted(v); }
};

EndGxlNamespace()
#endif
 
