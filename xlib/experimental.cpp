#include "experimental.h"
#include "dataitem.h"
#include "pers_util.h"

//..................................................................................................
bool ExperimentalDetails::SetTemp(const olxstr& t)  {
  if( t.IsEmpty() )  return false;
  olxch last_d = olxstr::o_toupper(t.Last());
  if( last_d == 'F' || last_d == 'K' || last_d == 'C' )  {
    olxch scale = olxstr::o_toupper(t.Last());
    TempValue = t.SubStringTo(t.Length()-1);
    if( scale == 'F' )
      TempValue = (TempValue-32.0)*5./9.;
    else if( scale == 'K' )
      TempValue += 273.15;
  }
  else  {
    TempValue = t;
  }
  return true;
}
//..................................................................................................
bool ExperimentalDetails::SetSize(const olxstr& s)  {
  TStrList toks(s, 'x');
  if( toks.Count() != 3 )
    return false;;
  for( int i=0; i < 3; i++ )
    CrystalSize[i] = toks[i].ToDouble();
  return true;
}
//..................................................................................................
bool ExperimentalDetails::SetWL(const olxstr& wl)  {
  Radiation = wl.ToDouble();
  return true;
}
//..................................................................................................
void ExperimentalDetails::ToDataItem(TDataItem& item) const {
  item.AddField("radiation", Radiation);
  item.AddField("temperature", TempValue.ToString());
  item.AddField("crystal_size", PersUtil::VecToStr(CrystalSize));
}
//..................................................................................................
#ifndef _NO_PYTHON
PyObject* ExperimentalDetails::PyExport() {
  PyObject* main = PyDict_New();
  PythonExt::SetDictItem(main, "radiation", Py_BuildValue("d", Radiation) );
  PythonExt::SetDictItem(main, "temperature", Py_BuildValue("d", TempValue.GetV()) );
  PythonExt::SetDictItem(main, "size", Py_BuildValue("(ddd)", CrystalSize[0], CrystalSize[1], CrystalSize[2]) );
  return main;
}
#endif
//..................................................................................................
void ExperimentalDetails::FromDataItem(const TDataItem& item) {
  Radiation = item.GetRequiredField("radiation").ToDouble();
  TempValue = item.GetRequiredField("temperature").ToDouble();
  CrystalSize = PersUtil::FloatVecFromStr( item.GetRequiredField("crystal_size") );
}
//..................................................................................................
void ExperimentalDetails::LibTemperature(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )
    E.SetRetVal( IsTemperatureSet() ? TempValue.ToString() : olxstr("n/a"));
  else
    SetTemp(Params[0]);
}
//..................................................................................................
void ExperimentalDetails::LibRadiation(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )
    E.SetRetVal(olxstr(Radiation));
  else
    SetWL(Params[0]);
}
//..................................................................................................
void ExperimentalDetails::LibSize(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )
    E.SetRetVal(olxstr(CrystalSize[0]) << 'x' << CrystalSize[1] << 'x' << CrystalSize[2]);
  else
    SetSize(Params[0]);
}
//..................................................................................................
TLibrary* ExperimentalDetails::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("exptl") : name );
  lib->RegisterFunction<ExperimentalDetails>(
    new TFunction<ExperimentalDetails>(this,  &ExperimentalDetails::LibTemperature, "Temperature", fpNone|fpOne,
"Returns/sets experiment temperature. Returns value in C, accepts strings like 120K, 10F. Default scale is C") );
  lib->RegisterFunction<ExperimentalDetails>(
    new TFunction<ExperimentalDetails>(this,  &ExperimentalDetails::LibRadiation, "Radiation", fpNone|fpOne,
"Returns/sets experiment wavelength in angstrems") );
  lib->RegisterFunction<ExperimentalDetails>(
    new TFunction<ExperimentalDetails>(this,  &ExperimentalDetails::LibSize, "Size", fpNone|fpOne,
"Returns/sets crystal size. Returns/accepts strings line 0.5x0.5x0.5 (in mm)") );
  return lib;
}
//..................................................................................................
