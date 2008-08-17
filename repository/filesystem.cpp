#ifdef  __BORLANDC__
  #pragma hdrstop
#endif

#include "filesystem.h"
#include "actions.h"
#include "bapp.h"
#include "efile.h"
#include "tptrlist.h"


olxstr TOSFileSystem::F__N;

TOSFileSystem::TOSFileSystem()  {
  OnRmFile   = &Events.NewQueue("rmf");
  OnRmDir    = &Events.NewQueue("rmd");
  OnMkDir    = &Events.NewQueue("mkd");
  OnChDir    = &Events.NewQueue("chd");
  OnAdoptFile = &Events.NewQueue("af");
  OnOpenFile  = &Events.NewQueue("of");
}
//..............................................................................
bool TOSFileSystem::DelFile(const olxstr& FN)  {
  F__N = FN;
  OnRmFile->Execute( this, &F__N);
  return true;
}
//..............................................................................
bool TOSFileSystem::DelDir(const olxstr& DN)  {
  F__N = DN;
  OnRmDir->Execute( this, &F__N);
  return true;
}
//..............................................................................
bool TOSFileSystem::AdoptFile(const TFSItem& Src)  {
  F__N = GetBase() + Src.GetFullName();
  OnAdoptFile->Execute( this, &F__N);

  olxstr DFN = GetBase() + Src.GetFullName();
  IInputStream* is = NULL;
  try  {  is = Src.GetFileSystem().OpenFile(Src.GetFileSystem().GetBase() + Src.GetFullName() );  }
  catch(const TExceptionBase& exc)  {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  if( is == NULL )  return false;

  try {
    olxstr path = TEFile::ExtractFilePath( DFN );
    if( !TEFile::FileExists(path) )
      if( !TEFile::MakeDir(path) )
        if( !TEFile::MakeDirs(path) )
          throw TFunctionFailedException(__OlxSourceInfo, olxstr("Mkdir \'") << path << '\'');
    TEFile destFile(DFN, "wb+");
    destFile << *is;
    delete is;
  }
  catch( const TExceptionBase& exc )  {
    delete is;
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  TEFile::SetFileTimes( DFN, Src.GetDateTime(), Src.GetDateTime() );
  return true;
}
//..............................................................................
bool TOSFileSystem::NewDir(const olxstr& DN)  {
  F__N = DN;
  OnMkDir->Execute(this, &F__N);
  return TEFile::MakeDir(DN);
}
//..............................................................................
bool TOSFileSystem::FileExists(const olxstr& FN)  {
  return TEFile::FileExists(FN);
}
//..............................................................................
IInputStream* TOSFileSystem::OpenFile(const olxstr& fileName)  {
  F__N = fileName;
  OnOpenFile->Execute(this, &F__N);
  return new TEFile( fileName, "rb");
}
//..............................................................................
bool TOSFileSystem::ChangeDir(const olxstr &DN)  {
  F__N = DN;
  OnChDir->Execute(this, &F__N);
  return TEFile::ChangeDir(DN);
}

//..............................................................................
//..............................................................................
//..............................................................................
TFSItem::TFSItem(TFSItem *parent, AFileSystem* FS, const olxstr& name) {
  Parent = parent;
  FileSystem = FS;
  Folder = false;
  Processed = false;
  DateTime = Size = 0;
  Name = name;
}
//..............................................................................
TFSItem::~TFSItem()  {
  Clear();
}
//..............................................................................
void TFSItem::Clear()  {
  for( int i=0; i < Items.Count(); i++ )
    delete Items.GetObject(i);
  Items.Clear();
  Name = EmptyString;
  DateTime = 0;
  Size = 0;
}
//..............................................................................
void TFSItem::operator >> (TStrList& S) const  {
  olxstr str = olxstr::CharStr('\t', GetLevel()-1 );
  S.Add( str + GetName() );
  str << DateTime;
  str << ',' << GetSize() << ',' << '{';

  for( int i=0; i < PropertyCount(); i++ )  {
    str << GetProperty(i);
    if( (i+1) < PropertyCount() )  str << ';';
  }
  str << '}';

  S.Add( str );
  for( int i=0; i < Items.Count(); i++ )
    Item(i) >> S;
}
//..............................................................................
int TFSItem::ReadStrings(int& index, TFSItem* caller, TStrList& strings, const TStrList* extensionsToSkip)  {
  TStrList toks, propToks;
  while( (index + 2) <= strings.Count() )  {
    int level = strings[index].LeadingCharCount( '\t' ), nextlevel = 0;
    olxstr name = strings[index].Trim('\t'), 
      ext = TEFile::ExtractFileExt(name);
    bool skip = false, folder = false;
    TFSItem* item = NULL;
    if( (index+2) < strings.Count() )  {
      nextlevel = strings[index+2].LeadingCharCount('\t');
      if( nextlevel > level )  
        folder = true;
    }
    if( !folder && extensionsToSkip != NULL && !ext.IsEmpty() )  {
      for( int i=0; i < extensionsToSkip->Count(); i++ )  {
        if( (*extensionsToSkip)[i].Comparei(ext) == 0 )  {
          skip = true;
          break;
        }
      }
    }
    if( !skip )  {
      item = &NewItem( name );
      item->SetFolder(folder);
      index++;
      toks.Strtok( strings[index], ',');
      if( toks.Count() < 2 )
        throw TInvalidArgumentException(__OlxSourceInfo, "token number");
      item->SetDateTime( toks[0].Trim('\t').RadInt<long>() );
      item->SetSize( toks[1].RadInt<long>() );
      for( int i=2; i < toks.Count(); i++ )  {
        if( toks[i].StartsFrom('{') && toks[i].EndsWith('}') )  {
          olxstr tmp = toks[i].SubString(1, toks[i].Length()-2);
          propToks.Clear();
          propToks.Strtok(tmp, ';');
          for( int j=0; j < propToks.Count(); j++ )
            item->AddProperty( propToks[j] );
        }
      }
      toks.Clear();
    }
    else
      index++;
    index++;
    if( index < strings.Count() )  {
      if( folder )  {
        int slevel = item->ReadStrings(index, this, strings);
        if( slevel != level )
          return slevel;
      }
      if( nextlevel < level )  return  nextlevel;
    }
  }
  return 0;
}
//..............................................................................
TFSItem& TFSItem::NewItem(const olxstr& name)  {
  TFSItem *I = new TFSItem(this, &GetFileSystem(), name);
  Items.Add(name, I);
  return *I;
}
//..............................................................................
olxstr TFSItem::GetFullName() const  {
// an alernative implementation can be done with olxstr::Insert ... to be considered
  TFSItem *FI = const_cast<TFSItem*>(this);
  olxstr Tmp;
  TPtrList<TFSItem> L;
  while( FI != NULL )  {
    L.Add(FI);
    FI = FI->GetParent();
    if( !FI->GetParent() )  break;  //ROOT
  }
  for( int i=L.Count()-1; i >= 0; i-- )  {
    Tmp << L[i]->GetName();
    if( i > 0 )  Tmp << '\\';
  }
  return Tmp;
}
//..............................................................................
int TFSItem::GetLevel()  const  {
  int level = 0;
  TFSItem *FI = const_cast<TFSItem*>(this);
  while( FI && FI->GetParent() )  {  FI = FI->GetParent();  level++;  }
  return level;
}
//..............................................................................
void TFSItem::DoSetProcessed(bool V)  {
  SetProcessed(V);
  for( int i=0; i < Count(); i++ )
    Item(i).SetProcessed(V);
}
//..............................................................................
int TFSItem::TotalItemsCount(int &cnt)  {
  cnt += Count();
  for( int i=0; i < Count(); i++ )  {
    TFSItem& FS = Item(i);
    if( FS.IsFolder() )
      FS.TotalItemsCount(cnt);
  }
  return cnt;
}
//..............................................................................
long int TFSItem::TotalItemsSize(long int &cnt)  {
  for( int i=0; i < Count(); i++ )  {
    TFSItem& FS = Item(i);
    if( FS.IsFolder() )
      FS.TotalItemsSize(cnt);
    else
      cnt += FS.GetSize();
  }
  return cnt;
}
//..............................................................................
int TFSItem::Synchronize(TFSItem* Caller, TFSItem& Dest, const TStrList& properties, bool Count)  {
  static TOnProgress Progress;
  static long int fc=0;
  if( Caller == NULL )  {
    fc = 0;
    Progress.SetMax( Synchronize(this, Dest, properties, true) );
    if( !fc )  return 0;  // nothing to do then ...
    fc = 0;
    Progress.SetPos( 0.0 );
    TBasicApp::GetInstance()->OnProgress->Enter(this, &Progress);
  }
  SetProcessed(false);
  /* check the repository files are at the destination - if not delete them
  (not implemented in TOSFileSystem, as a bit too dangerous)
  */
  for( int i=0; i < Dest.Count(); i++ )  {
    TFSItem& FI = Dest.Item(i);
    if( !Count )  {
      Progress.SetAction( FI.GetFullName() );
      Progress.IncPos( FI.GetSize() );
      TBasicApp::GetInstance()->OnProgress->Execute(this, &Progress);
    }                                        
    else
      fc += FI.GetSize();

    TFSItem* Res = FindByName( FI.GetName() );
    if( FI.IsFolder() && Res!= NULL )  {
      Res->Synchronize(this, FI, properties, Count);
      Res->SetProcessed(true);
      continue;
    }
    if( Res == NULL )  {
      if( !Count )
        FI.DelFile();
    }
    else  {
      Res->SetProcessed(true);
      if( !FI.IsFolder() && FI.Properties.Count() != 0 && properties.Count() != 0 )  {
        bool proceed = false;
        for(int j=0; j < properties.Count(); j++ )  {
          if( FI.HasProperty( properties.String(j) ) )  {
            proceed = true;  break;
          }
        }
        if( !proceed )  continue;
      }
      if( Res->GetDateTime() > FI.GetDateTime() ||
          !FI.GetFileSystem().FileExists(FI.GetFileSystem().GetBase() + FI.GetFullName()) )  {
        if( !Count )
          FI.UpdateFile(*Res);
      }
    }
  }
  /* add new files ans update the changed ones */
  for( int i=0; i < this->Count(); i++ )  {
    TFSItem& FI = Item(i);
    if( !FI.IsFolder() && FI.Properties.Count() != 0 && properties.Count() != 0 )  {
      bool proceed = false;
      for(int j=0; j < properties.Count(); j++ )  {
        if( FI.HasProperty( properties.String(j) ) )  {
          proceed = true;  break;
        }
      }
      if( !proceed )  continue;
    }
    if( !Count )  {
      Progress.SetAction( FI.GetFullName() );
      Progress.IncPos( FI.GetSize() );
      TBasicApp::GetInstance()->OnProgress->Execute(this, &Progress);
    }
    else
      fc += FI.GetSize();

    if( FI.IsProcessed() )  continue;
    if( FI.IsFolder() )  {
      if( !Count )  {
        TFSItem* Res = Dest.UpdateFile(FI);
        if( Res != NULL)
          FI.Synchronize(this, *Res, properties);
      }
      else
        FI.TotalItemsSize(fc);
    }
    else  {
      if( !Count )
        Dest.UpdateFile(FI);
    }
  }
  if( this->GetParent() == NULL )  {
    TBasicApp::GetInstance()->OnProgress->Exit(NULL, &Progress);
  }
  return fc;
}
//..............................................................................
TFSItem* TFSItem::UpdateFile(TFSItem& item)  {
  if( item.IsFolder() )  {
    olxstr FN = GetFileSystem().GetBase() + item.GetFullName();
    if( TEFile::FileExists(FN) || GetFileSystem().NewDir(FN) )    {
      TFSItem* FI = FindByName(item.GetName());
      if( FI == NULL )
        FI = &NewItem(item.GetName());
      *FI = item;
      return FI;
    }
    return NULL;
  }
  else  {
    // an error can be only caused by impossibility to open the file
    try  {
      GetFileSystem().AdoptFile(item);
      TFSItem* FI = const_cast<TFSItem*>(this);
      // if names are different - we are looking at the content of this
      if( this->GetName() != item.GetName() )  {
        FI = FindByName( item.GetName() );
        if( FI == NULL )
          FI = &NewItem( item.GetName() );
      }
      *FI = item;
      return FI;
    }
    catch( TExceptionBase& )  {  
      return NULL;
    }
  }
}
//..............................................................................
void TFSItem::DeleteItem(TFSItem* item)  {
  int ind = Items.IndexOfComparable( item->GetName() );
  if( ind != -1 )
    Items.Remove( ind );
  item->DelFile();
  delete item;
}
//..............................................................................
void TFSItem::Remove(TFSItem& item)  {
  if( &item == this )
    throw TInvalidArgumentException(__OlxSourceInfo, "cannot delete itself");
  if( item.GetParent() == NULL )
    throw TInvalidArgumentException(__OlxSourceInfo, "cannot delete ROOT");
  item.GetParent()->DeleteItem( &item );
}
//..............................................................................
void TFSItem::DelFile()  {
  if( IsFolder() )  {
    for( int i=0; i < Count(); i++ )
      Item(i).DelFile();
    GetFileSystem().DelDir( GetFileSystem().GetBase() + GetFullName() );
  }
  else
    GetFileSystem().DelFile( GetFileSystem().GetBase() + GetFullName());
}
//..............................................................................
TFSItem& TFSItem::operator = (const TFSItem& FI)  {
  Name = FI.GetName();
  Size = FI.GetSize();
  DateTime = FI.GetDateTime();
  Folder = FI.IsFolder();
  Properties.Clear();
  for( int i=0; i < FI.PropertyCount(); i++ )
    AddProperty( FI.GetProperty(i) );
  return *this;
}
//..............................................................................
void TFSItem::ListUniqueProperties(TCSTypeList<olxstr, void*>& uProps)  {
  for( int i=0; i < PropertyCount(); i++ )
    if( uProps.IndexOfComparable( GetProperty(i) ) == -1 )
      uProps.Add( GetProperty(i), NULL);
  for( int i=0; i < Count(); i++ )
    Item(i).ListUniqueProperties( uProps );
}
//..............................................................................
TFSItem* TFSItem::FindByFullName(const olxstr& Name)  const {
  TStrList toks(TEFile::UnixPath(Name), '/');
  if( toks.IsEmpty() )  return NULL;

  TFSItem* root = const_cast<TFSItem*>(this);
  for(int i=0; i < toks.Count(); i++ )  {
    root = root->FindByName( toks.String(i) );
    if( root == NULL )  return root;
  }
  return root;
}
//..............................................................................
TFSItem& TFSItem::NewItem(TFSItem* item)  {
  TPtrList<TFSItem> items;
  TFSItem* ti = item;
  while( item != NULL && item->GetParent() != NULL )  {
    items.Add( item );
    item = item->GetParent();
  }
// not in the items all path to the item
  ti = this;
  for( int i = items.Count()-1; i >= 0;  i-- )  {
    TFSItem* nti = ti->UpdateFile( *items[i] );
    if( nti == NULL )
      throw TFunctionFailedException( __OlxSourceInfo, "failed to update file");
    ti = nti;
  }
  return *ti;
}
//..............................................................................
void TFSItem::ClearNonexisting()  {
  for( int i=0; i < Count(); i++ )  {
    if( !Item(i).IsFolder() )  {
      if( !GetFileSystem().FileExists( GetFileSystem().GetBase() + Item(i).GetFullName()) )  {
        Items.Remove(i);
        i--;
      }
    }
    else  {
      Item(i).ClearNonexisting();
      if( Item(i).IsEmpty() )  {
        delete Items.Object(i);
        Items.Remove(i);
        i--;
      }
    }
  }
}
//..............................................................................
void TFSItem::ClearEmptyFolders()  {
  for( int i=0; i < Count(); i++ )  {
    if( Item(i).IsFolder() )
      if( Item(i).Count() > 0 )
        Item(i).ClearEmptyFolders();
  }
  for( int i=0; i < Count(); i++ )  {
    if( Item(i).IsFolder() )  {
      if( Item(i).IsEmpty() )  {
        delete Items.Object(i);
        Items.Remove(i);
        i--;
      }
    }
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
TFSIndex::TFSIndex(AFileSystem& fs)  {
  Root = new TFSItem(NULL, &fs, "ROOT");
}
//..............................................................................
TFSIndex::~TFSIndex()  {
  delete Root;
}
//..............................................................................
void TFSIndex::LoadIndex(const olxstr& IndexFile, const TStrList* extensionsToSkip)  {
  GetRoot().Clear();
  if( !GetRoot().GetFileSystem().FileExists(IndexFile) )
    throw TFileDoesNotExistException(__OlxSourceInfo, IndexFile);

  IInputStream* is = NULL;
  try {  is = GetRoot().GetFileSystem().OpenFile(IndexFile);  }
  catch( TExceptionBase &exc )  {
    throw TFunctionFailedException(__OlxSourceInfo, exc.GetException()->GetFullMessage() );
  }
  if( is == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "could not load index file" );
  TStrList strings;
  strings.LoadFromTextStream( *is );
  delete is;
  int index = 0;
  GetRoot().ReadStrings(index, NULL, strings, extensionsToSkip);
  Properties.Clear();
  GetRoot().ListUniqueProperties( Properties );
  GetRoot().ClearNonexisting();
}
//..............................................................................
void TFSIndex::SaveIndex(const olxstr &IndexFile)  {
  TStrList strings;

  GetRoot().ClearEmptyFolders();

  for( int i=0; i < GetRoot().Count(); i++ )
    GetRoot().Item(i) >> strings;
  TCStrList(strings).SaveToFile(IndexFile );
}
//..............................................................................
int TFSIndex::Synchronise(AFileSystem& To, const TStrList& properties, const TStrList* extensionsToSkip)  {
  TFSIndex DestI(To);
  olxstr SrcInd;   SrcInd << GetRoot().GetFileSystem().GetBase() << "index.ind";
  olxstr DestInd;  DestInd << To.GetBase() << "index.ind";
  FilesUpdated = 0;
  try  {
    LoadIndex(SrcInd, extensionsToSkip);
    if( To.FileExists(DestInd) )
      DestI.LoadIndex(DestInd);
    FilesUpdated = GetRoot().Synchronize(NULL, DestI.GetRoot(), properties );

    DestI.SaveIndex(DestInd);
  }
  catch( const TExceptionBase& exc )  {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }

  return FilesUpdated;
}
//..............................................................................
bool TFSIndex::UpdateFile(AFileSystem& To, const olxstr& fileName, bool Force)  {
  TFSIndex DestI(To);
  olxstr SrcInd;   SrcInd << GetRoot().GetFileSystem().GetBase() << "index.ind";
  olxstr DestInd;  DestInd << To.GetBase() << "index.ind";
  bool res = false;
  try  {
    LoadIndex(SrcInd);
    if( To.FileExists(DestInd) )
      DestI.LoadIndex(DestInd);

    TFSItem* src = GetRoot().FindByFullName(fileName);
    if( src == NULL )
      throw TFileDoesNotExistException( __OlxSourceInfo, fileName );
    TFSItem* dest = DestI.GetRoot().FindByFullName(fileName);
    if( dest != NULL )  {
      if( Force || (src->GetDateTime() > dest->GetDateTime()) ||
        !To.FileExists( To.GetBase() + dest->GetFullName()) )  {
          olxstr test = To.GetBase() + dest->GetFullName();
        if( To.AdoptFile( *src ) )  {
          *dest = *src;
          res = true;
        }
      }
    }
    else  {
      DestI.GetRoot().NewItem( src );
      //To.AdoptFile( *src );
      res = true;
    }
    if( res )
      DestI.SaveIndex( DestInd );
  }
  catch( const TExceptionBase& exc )  {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  return res;
}

