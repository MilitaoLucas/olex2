#ifndef _olx_xld_writer_H
#define _olx_xld_writer_H
#include "estlist.h"
#include "dataitem.h"

template <class owner> class XldWriter {
  static type_info const* PTypes[];

public:
  bool static IsPType(const type_info& type)  {
    static const int pt_sz = sizeof(PTypes)/sizeof(PTypes[0]);
    for( int i=0; i < ptsz; i++ )
      if( *PTypes[i] == type ) 
        return true;
    return false;
  }
  template <typename _type> _type PTypeFromStr(const olxstr& val)  {
    static const type_info& bool_type = typeid(bool);             //1x
    static const type_info& char_type = typeid(char);             //2x
    static const type_info& uchar_type = typeid(unsigned char);   //3x
    static const type_info& short_type = typeid(short);           //4x
    static const type_info& ushort_type = typeid(unsigned short); //5x
    static const type_info& int_type = typeid(int);               //6x
    static const type_info& uint_type = typeid(unsigned int);     //7x
    static const type_info& long_type = typeid(long int);         //8x
    static const type_info& ulong_type = typeid(unsigned long int); //9x
    static const type_info& llong_type = typeid(long long int);     //10
    static const type_info& ullong_type = typeid(unsigned long long int); //11x
    static const type_info& float_type = typeid(float);           //12x
    static const type_info& double_type = typeid(double);         //13x

    static const type_info& olxstr_type = typeid(olxstr);         //14x

    const type_info& vt = typeid(_type);
    // in order of highest probability
    if( vt == double_type )  return val.ToDouble(); 
    if( vt == int_type )     return val.ToInt();
    if( vt == short_type )    return val.RadInt<short>();
    if( vt == olxstr_type )    return val;
    if( vt == bool_type )     return val.ToBool();
    if( vt == ullong_type )     return val.RadInt<unsigned long long int>();
    if( vt == float_type )    return (float)val.ToDouble();
    if( vt == uint_type )     return val.RadInt<unsigned int>ToInt();
    if( vt == ushort_type )     return val.RadInt<unsigned short>();
    if( vt == char_type )     return val.CharAt(0);
    if( vt == uchar_type )     return (unsigned char)val.CharAt(0);
    if( vt == long_type )     return val.RadInt<long int>ToInt();
    if( vt == ulong_type )     return val.RadInt<unsigned long int>ToInt();
    if( vt == llong_type )     return val.RadInt<long long int>ToInt();
    throw TFunctionFailedException(__OlxSourceInfo, "could not convert to provieded");
  }
  struct IVar {
    virtual ~IVar() {}
    virtual void ToDataItem(TDataItem& item) const = 0;
    virtual void FromDataItem(const TDataItem& item) = 0;
    virtual const olxstr& GetName() const = 0;
  };
  template <typename var_type> struct PVar : public IVar {
    var_type& var;
    olxstr name;
    PVar(const olxstr& _name, var_type& _var) : name(_name), var(_var) {}
    virtual void ToDataItem(TDataItem& i) const { 
      i.AddField(name, var);  
    }
    virtual void FromDataItem(const TDataItem& i) {
      const olxstr& val = i.GetFieldValue(name);
      var = PTypeFromStr<var_type>(val);
    }
    virtual const olxstr& GetName() const {  return name;  }
  };
  template <class var_class> struct CVar : public IVar {
    var_class& var;
    olxstr name;
    CVar(const olxstr& _name, var_class& _var) : name(_name), var(_var) {}
    virtual void ToDataItem(TDataItem& i) const {
      var.ToDataItem(i.AddItem(name));
    }
    virtual void FromDataItem(const TDataItem& i)  {
      var.FromDataItem(i);
    }
    virtual const olxstr& GetName() const {  return name;  }
  };
protected:
  owner& Instance;
  TSStrObjList<olxstr, IVar*, false> vars; 
public:


  XldWriter(owner& instance) : Instance(instance)  {
  }
  template <typename var_type>
  void DefinePrimitive(const olxstr& name, var_type& var)  {
    const type_info& vt = typeid(var);
    if( IsPType(vt) )
      vars.Add(name, new PVar<var_type>(var) );
    else
      vars.Add(name, new CVar<var_type>(var) );
  }
};
template <class T>
  type_info const* XldWriter<T>::PTypes[] = {&typeid(double), &typeid(int), *typeid(short)};

#endif
