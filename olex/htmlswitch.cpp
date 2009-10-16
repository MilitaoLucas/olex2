#include "htmlswitch.h"
#include "bapp.h"
#include "log.h"
#include "utf8file.h"
#include "htmlext.h"
#include "fsext.h"
#include "wxzipfs.h"

void THtmlSwitch::Clear()  {
  Switches.Clear();
  Strings.Clear();
  //Params.Clear();
}
//..............................................................................
void THtmlSwitch::SetFileIndex(short ind)  {
  FileIndex = ind;
  ParentHtml->SetSwitchState(*this, FileIndex);
}
//..............................................................................
void THtmlSwitch::UpdateFileIndex()  {
  Clear();
  if( FileIndex >= FileCount() || FileIndex < 0 )  return;
  olxstr FN = Files[FileIndex];
  IInputStream *is = TFileHandlerManager::GetInputStream(FN);
  if( is == NULL )  {
    TBasicApp::GetLog().Error( olxstr("THtmlSwitch::File does not exist: ") << FN );
    return;
  }
#ifdef _UNICODE
  TUtf8File::ReadLines(*is, Strings, false);
#else
  Strings.LoadFromTextStream(*is);
#endif
  delete is;
  for( int i=0; i < Strings.Count(); i++ )  {
    // replace the parameters with their values
    if( Strings[i].IndexOf('#') != -1 )  {
      // "key word parameter"
      Strings[i].Replace("#switch_name", Name);
      if( ParentSwitch != NULL )  {
        Strings[i].Replace("#parent_name", ParentSwitch->GetName()).\
                   Replace( "#parent_file", ParentSwitch->GetCurrentFile() );
      }

      for( int j=0; j < Params.Count(); j++ )
        Strings[i].Replace(olxstr('#') << Params.GetName(j), Params.GetValue(j) );
    } // end of parameter replacement
  }
  ParentHtml->CheckForSwitches(*this, TZipWrapper::IsZipFile(FN));
  for( int i=0; i < Switches.Count(); i++ )
    Switches[i].UpdateFileIndex();
}
//..............................................................................
bool THtmlSwitch::ToFile()  {
  if( Switches.IsEmpty() )  return true;
  if( GetCurrentFile().IsEmpty() )  return true;
  for( int i=0; i < Strings.Count(); i++ )  {
    if( Strings.GetObject(i) )  {
      THtmlSwitch *HO = Strings.GetObject(i);
      ParentHtml->UpdateSwitchState(*HO, Strings[i]);
      HO->ToFile();
    }
  }
#ifdef _UNICODE
  TUtf8File::WriteLines(GetCurrentFile(), Strings, false);
#else
  Strings.SaveToFile(GetCurrentFile());
#endif
  return true;
}
//..............................................................................
THtmlSwitch& THtmlSwitch::NewSwitch()  {
  return Switches.AddNew(ParentHtml, this);
}
//..............................................................................
int THtmlSwitch::FindSimilar(const olxstr& start, const olxstr& end, TPtrList<THtmlSwitch>& ret)  {
  int cnt = 0;
  for( int i=0; i < Switches.Count(); i++ )  {
    if( end.IsEmpty() )  {
      if( Switches[i].GetName().StartsFrom(start) )  {
        ret.Add(Switches[i]);
        cnt++;
      }
    }
    else if( start.IsEmpty() )  {
      if( Switches[i].GetName().EndsWith(end) )  {
        ret.Add(Switches[i]);
        cnt++;
      }
    }
    else {
      if( Switches[i].GetName().StartsFrom(start) && Switches[i].GetName().EndsWith(end) )  {
        ret.Add(Switches[i]);
        cnt++;
      }
    }
    cnt += Switches[i].FindSimilar(start, end, ret);
  }
  return cnt;
}
//..............................................................................
void THtmlSwitch::Expand(TPtrList<THtmlSwitch>& ret)  {
  for( int i=0; i < Switches.Count(); i++ )  {
    ret.Add(Switches[i]);
    Switches[i].Expand(ret);
  }
}
//..............................................................................
THtmlSwitch*  THtmlSwitch::FindSwitch(const olxstr &IName)  {
  for( int i=0; i < Switches.Count(); i++ )  {
    if( Switches[i].GetName().Equalsi(IName) )
      return &Switches[i];
    else  {
      THtmlSwitch* Res = Switches[i].FindSwitch(IName);
      if( Res != NULL )
        return Res;
    }
  }
  return NULL;
}
//..............................................................................
void THtmlSwitch::ToStrings(TStrList &List)  {
  if( FileIndex >= 0 && FileIndex < Files.Count() )
    List.Add("<SWITCHINFOS SRC=\"")<< Files[FileIndex] << "\">";

  for( int i=0; i < Strings.Count(); i++ )  {
    if( Strings.GetObject(i) != NULL )
      Strings.GetObject(i)->ToStrings(List);
    List.Add(Strings[i]);
  }

  if( FileIndex >= 0 && FileIndex < Files.Count() )
    List.Add("<SWITCHINFOE>");
}
