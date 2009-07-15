//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <FileCtrl.hpp>
#include <Registry.hpp>

#include "main.h"

#include "licence.h"

#include "efile.h"
#include "winhttpfs.h"
#include "winzipfs.h"
#include "settingsfile.h"
#include "shellutil.h"
#include "estrlist.h"
#include "updateapi.h"
#include "patchapi.h"
#include "egc.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "frame1"
#pragma resource "*.dfm"

TfMain *fMain;

class TProgress: public AActionHandler  {
public:
  TProgress(){}
  virtual ~TProgress(){}
  bool Exit(const IEObject *Sender, const IEObject *Data)  {  return true;  }
  bool Enter(const IEObject *Sender, const IEObject *Data)  {
    if( !EsdlInstanceOf( *Data, TOnProgress) )  return false;
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    fMain->pbProgress->Max = A->GetMax();
    return true;
  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    if( !EsdlInstanceOf( *Data, TOnProgress) )  return false;
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    fMain->pbProgress->Position = A->GetPos();
    if( Sender != NULL && EsdlInstanceOf(*Sender, TWinHttpFileSystem) )
      fMain->frMain->stAction->Caption = (olxstr("Downloading: ") << TEFile::ExtractFileName(A->GetAction().c_str())).c_str();
    else
      fMain->frMain->stAction->Caption = TEFile::ExtractFileName(A->GetAction()).c_str();
    Application->ProcessMessages();
    return true;
  }
};

olxstr TfMain::SettingsFile = "usettings.dat";

//---------------------------------------------------------------------------
__fastcall TfMain::TfMain(TComponent* Owner) :TForm(Owner)  {
  MouseDown = Dragging = false;
  Expanded = false;
  TEGC::Initialise();
  olxstr ip( TShellUtil::GetSpecialFolderLocation(fiProgramFiles) );
  TEFile::AddTrailingBackslashI( ip ) << "Olex2";
  frMain->eInstallationPath->Text = ip.c_str();
  OlexInstalled = CheckOlexInstalled( OlexInstalledPath );
  if( OlexInstalled )  {
    Bapp = new TBasicApp( TEFile::AddTrailingBackslash(OlexInstalledPath) );
    Bapp->OnProgress->Add( new TProgress );
    TEFile::AddTrailingBackslash( OlexInstalledPath );
    frMain->eInstallationPath->Text = OlexInstalledPath.c_str();
    olxstr sfile = OlexInstalledPath;
           sfile << TfMain::SettingsFile;
    if( TEFile::Exists( sfile ) )  {
      const TSettingsFile Settings( sfile );
      frMain->cbRepository->Text = Settings["repository"].c_str();
      frMain->eProxy->Text = Settings["proxy"].c_str();
      int updateInd = frMain->rgAutoUpdate->Items->IndexOfName( Settings["update"].c_str() );
      if( updateInd != -1 )
        frMain->rgAutoUpdate->ItemIndex = updateInd;
    }
    frMain->sbPickZip->Visible = false;
    frMain->cbRepository->Width = 278;
    frMain->bbInstall->Caption = "Update";
    frMain->bbUninstall->Visible = true;
    frMain->bbBrowse->Enabled = false;
    frMain->eInstallationPath->Enabled = false;
    frMain->cbCreateShortcut->Visible = false;
    frMain->cbCreateDesktopShortcut->Visible = false;
    frMain->cbRepository->Enabled = false;
    frMain->rgAutoUpdate->Enabled = false;
  }
  else
    Bapp = new TBasicApp( TBasicApp::GuessBaseDir(CmdLine) );
  frMain->bbInstall->Default = true;
}
//---------------------------------------------------------------------------
__fastcall TfMain::~TfMain()  {
  Bapp->OnIdle->Execute(NULL, NULL);
  delete Bapp;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::iSplashMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
  if( !MouseDown )  return;
  if( !Dragging )  {
    if( abs(X-MouseDownX) >= 3 || abs(Y-MouseDownY) >= 3 )
      Dragging = true;
  }
  if( !Dragging )  return;
  Left = Left + (X-MouseDownX);
  Top = Top + (Y-MouseDownY);
}
//---------------------------------------------------------------------------
void __fastcall TfMain::iSplashMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  MouseDownX = X;
  MouseDownY = Y;
  MouseDown = true;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::iSplashMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  MouseDown = Dragging = false;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::bbBrowseClick(TObject *Sender)
{
  this->Enabled = false;
  try  {
    olxstr dir( TShellUtil::PickFolder("Please select installation folder",
      TShellUtil::GetSpecialFolderLocation(fiPrograms), EmptyString) );
    if( dir.Length() != 0 )
      frMain->eInstallationPath->Text = dir.c_str();
    dir = TShellUtil::GetSpecialFolderLocation(fiProgramFiles);
  }
  catch( ... )  {  ;  }
  this->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::bbDoneClick(TObject *Sender)  {
  Application->Terminate();
}
//---------------------------------------------------------------------------
bool TfMain::DoInstall(const olxstr& zipFile, const olxstr& installPath)  {
  TBasicApp::SetBaseDir(TEFile::AddTrailingBackslash(installPath) << "installer.exe");
  TWinZipFileSystem zfs( zipFile );
  bool res = zfs.Exists("olex2.tag");
  if( res )  {
    zfs.OnProgress->Add( new TProgress );
    TEFile* lic_f = NULL;
    try  {  lic_f = zfs.OpenFileAsFile("licence.rtf");  }
    catch(...)  {  res = false;  }
    if( lic_f == NULL )  res = false;
    if( res )  {
      olxstr lic_fn = lic_f->GetName();
      delete lic_f;
      dlgLicence->reEdit->Lines->LoadFromFile( lic_fn.c_str() );
      if( dlgLicence->ShowModal() != mrOk )
        return false;
      try  {  zfs.ExtractAll(installPath);  }
      catch(...) {  res = false;  }
    }
  }
  if( !res )
    Application->MessageBoxA("Invalid installation archive.", "Installation failed", MB_OK|MB_ICONERROR);
  return res;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::bbInstallClick(TObject *Sender)  {
  olxstr reposPath, proxyPath, installPath;

  frMain->bbInstall->Enabled = false;
  pbProgress->Visible = true;
  if( frMain->eProxy->Enabled )
    proxyPath = frMain->eProxy->Text.c_str();

  reposPath = TEFile::UnixPath(frMain->cbRepository->Text.c_str());

  bool localInstall = TEFile::IsAbsolutePath( reposPath );

  if( !localInstall )
    if( reposPath.Length() && !reposPath.EndsWith('/') )
      reposPath << '/';

  installPath = frMain->eInstallationPath->Text.c_str();
  installPath = TEFile::WinPath( installPath );
  if( !installPath.IsEmpty() && !installPath.EndsWith('\\') )  installPath << '\\';

  if( !OlexInstalled )  {
    if( !TEFile::Exists(installPath) )  {
      if( !ForceDirectories( installPath.c_str() ) )  {
        Application->MessageBoxA("Could not create installation folder.\
 Please make sure the folder is not opened in any other programs and try again.", "Error", MB_OK|MB_ICONERROR);
        frMain->bbInstall->Enabled = false;
        return;
      }
    }
    else if( !TEFile::IsEmptyDir(installPath) ) {
      int res =Application->MessageBoxA("The instalaltion folder already exists.\nThe installer needs to empty it.\nContinue?",
            "Confirm", MB_YESNOCANCEL|MB_ICONWARNING);
      if( res == IDYES )  {
        if( !TEFile::DeleteDir(installPath, true) )  {
          Application->MessageBoxA("Could not clean up the instalaltion folder. Please try later.", "Error", MB_OK|MB_ICONERROR);
          frMain->bbInstall->Enabled = false;
          return;
        }
      }
      else if( res == IDCANCEL )  {
        frMain->bbInstall->Enabled = false;
        return;
      }
    }
  }
  if( OlexInstalled )  {
    TEFile::ChangeDir( OlexInstalledPath );
    LaunchFile(OlexInstalledPath + "olex2.exe", true);
    return;
  }

  try  {
    olxstr StartDir = TBasicApp::GetBaseDir();  // it will be changd after install!!
    if( !localInstall )  {
      TUrl url( reposPath );
      if( !proxyPath.IsEmpty() )
        url.SetProxy(proxyPath);
      TWinHttpFileSystem repos(url);
      repos.OnProgress->Add( new TProgress );
      TEFile* zipf = repos.OpenFileAsFile( url.GetPath() + "olex2.zip");
      if( zipf == NULL )  {
        frMain->stAction->Caption = "Failed...";
        Application->MessageBoxA("Could not locate the Olex2 archive.\nPlease try another repository.", "Zip file fetching error", MB_OK|MB_ICONERROR);
        frMain->bbInstall->Enabled = true;
        return;
      }
      olxstr zipName( zipf->GetName() );
      delete zipf;
      // the file gets deleted with the FS
      if( !DoInstall( zipName, installPath ) )  {
        return;
      }
    }
    else  {
      if( !DoInstall( frMain->cbRepository->Text.c_str(), installPath ) )
        return;
    }
//    frMain->pbProgress->Position = 0;
    // install MSVC redistributables
    olxstr redist_path( StartDir + "redist/vcredist_x86.exe");
    if( TEFile::Exists(redist_path) )
      LaunchFile( redist_path, false );

    updater::UpdateAPI api;
    if( localInstall )
      api.GetSettings().repository = "http://dimas.dur.ac.uk/olex2-distro/";
    else
      api.GetSettings().repository = api.TrimTagPart(reposPath);

    api.GetSettings().proxy = proxyPath;
    api.GetSettings().update_interval = frMain->rgAutoUpdate->Items->Strings[frMain->rgAutoUpdate->ItemIndex].c_str();
    api.GetSettings().Save();

    frMain->stAction->Caption = "Done";
    InitRegistry( installPath.c_str() );
    OSVERSIONINFO veri;
    memset(&veri, 0, sizeof(veri));
    veri.dwOSVersionInfoSize = sizeof(veri);
    GetVersionEx(&veri);
    // only after XP
    bool SetRunAs = veri.dwMajorVersion > 5;
    // create shortcuts
    if( frMain->cbCreateShortcut->Checked )
      TShellUtil::CreateShortcut(TShellUtil::GetSpecialFolderLocation(fiCommonStartMenu) + "Olex2.lnk",
                                 installPath + "olex2.exe", "Olex2 launcher", SetRunAs);
    if( frMain->cbCreateDesktopShortcut->Checked )
      TShellUtil::CreateShortcut(TShellUtil::GetSpecialFolderLocation(fiCommonDesktop) + "Olex2.lnk",
                                 installPath + "olex2.exe", "Olex2 launcher", SetRunAs);
  }
  catch( const TExceptionBase& exc )  {
    Application->MessageBox("The installation has failed. If using online installation please check, that\
your computers is online.", "Installation failed", MB_OK|MB_ICONERROR);
  }
  OlexInstalled = CheckOlexInstalled( OlexInstalledPath );
  if( OlexInstalled )  {
    frMain->bbInstall->Caption = "Run!";
    frMain->bbInstall->Enabled = true;
    pbProgress->Visible = false;
  }
}
//---------------------------------------------------------------------------
bool TfMain::CheckOlexInstalled(olxstr& installPath)  {
  TRegistry* Reg = new TRegistry();
  bool res = false;
  try  {
    Reg->RootKey = HKEY_CLASSES_ROOT;
    AnsiString cmdKey = "Applications\\olex2.dll\\shell\\open\\command";
    res = Reg->OpenKey(cmdKey, false);
    if( res )  {
      installPath = Reg->ReadString("").c_str();
      installPath = TEFile::ExtractFilePath(installPath.Trim('"'));
      Reg->CloseKey();
    }
  }
  catch( ... )  {    }
  delete Reg;
  if( res && !TEFile::Exists(installPath) )
    return false;
  return res;
}
//---------------------------------------------------------------------------
bool TfMain::InitRegistry(const AnsiString& installPath)  {
  TRegistry* Reg = new TRegistry();
  bool res = false;
  try  {
    Reg->RootKey = HKEY_CLASSES_ROOT;
    AnsiString cmdKey = "Applications\\olex2.dll\\shell\\open\\command";
    res = Reg->OpenKey(cmdKey, true);
    if( res )  {
      AnsiString val = '\"';
      val += installPath;
      val += "olex2.dll";
      val += "\" '%1'";
      Reg->WriteString("", val);
      Reg->CloseKey();
    }
    if( res )  {
      res = Reg->OpenKey(".ins\\OpenWithList\\olex2.dll", true);
      Reg->CloseKey();
    }
    if( res )  {
      res = Reg->OpenKey(".mol\\OpenWithList\\olex2.dll", true);
      Reg->CloseKey();
    }
    if( res )  {
      res = Reg->OpenKey(".res\\OpenWithList\\olex2.dll", true);
      Reg->CloseKey();
    }
    if( res )  {
      res = Reg->OpenKey(".cif\\OpenWithList\\olex2.dll", true);
      Reg->CloseKey();
    }
    if( res )  {
      res = Reg->OpenKey(".xyz\\OpenWithList\\olex2.dll", true);
      Reg->CloseKey();
    }
  }
  catch( ... )  {    }
  delete Reg;
  return res;
}
//---------------------------------------------------------------------------
bool TfMain::CleanRegistry()
{
  TRegistry* Reg = new TRegistry();
  bool res = false;
  try  {
    Reg->RootKey = HKEY_CLASSES_ROOT;
    AnsiString cmdKey = "Applications\\olex2.dll\\shell\\open\\command";
    res = Reg->KeyExists( cmdKey );
    if( res )  res = Reg->DeleteKey(cmdKey);
    if( res )  res = Reg->DeleteKey(".ins\\OpenWithList\\olex2.dll");
    if( res )  res = Reg->DeleteKey(".mol\\OpenWithList\\olex2.dll");
    if( res )  res = Reg->DeleteKey(".res\\OpenWithList\\olex2.dll");
    if( res )  res = Reg->DeleteKey(".cif\\OpenWithList\\olex2.dll");
    if( res )  res = Reg->DeleteKey(".xyz\\OpenWithList\\olex2.dll");
  }
  catch( ... )  {    }
  delete Reg;
  return res;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::bbUninstallClick(TObject *Sender)  {
  if( !OlexInstalled )  return;
  if( patcher::PatchAPI::IsOlex2Running() )  {
    Application->MessageBoxA("There are Olex2 instances are running or you do\
 not have sufficient rights to access the the instalaltion folder...", "Error", MB_OK|MB_ICONERROR);
    return;
  }
  olxstr indexFileName( OlexInstalledPath);
         indexFileName << "index.ind";

  frMain->bbUninstall->Enabled = false;
  frMain->bbInstall->Enabled = false;

  if( !TEFile::Exists(indexFileName) )  {
    Application->MessageBoxA("Could not locate installation database\nProcessing registry and shortcuts...",
      "Error",
      MB_OK|MB_ICONERROR);

    frMain->bbInstall->Enabled = true;
    frMain->bbInstall->Caption = "Install";
    frMain->bbUninstall->Visible = false;
    frMain->bbBrowse->Enabled = true;
    frMain->eInstallationPath->Enabled = true;
    frMain->cbCreateShortcut->Visible = true;
    frMain->cbCreateDesktopShortcut->Visible = true;
  }
  else  {
    int res = Application->MessageBoxA("Do you want to remove ALL Olex2 folders?",
          "Confirm", MB_YESNOCANCEL|MB_ICONQUESTION);
     if( res == IDCANCEL )
       return;
     if( res == IDYES )  {
      if( !TEFile::DeleteDir(OlexInstalledPath) )
          Application->MessageBoxA("Could not remove all Olex2 folders...", "Error", MB_OK|MB_ICONERROR);
      olxstr DataDir = TShellUtil::GetSpecialFolderLocation(fiAppData);
      TStrList dirs;
      dirs.Add( DataDir + "Olex2u");
      dirs.Add( DataDir + "Olex2");
      for( int i=0; i < dirs.Count(); i++ )  {
        if( TEFile::Exists(dirs[i]) )
          TEFile::DeleteDir(dirs[i]);
      }
    }
    else  {
      TOSFileSystem osFS(OlexInstalledPath);
      TFSIndex FSIndex(osFS);
      FSIndex.LoadIndex( indexFileName );
      CleanInstallationFolder( FSIndex.GetRoot() );
      TEFile::DelFile( indexFileName );
    }
  }
  if( !CleanRegistry() )  {
    Application->MessageBoxA("Could not remove registry entries", "Error", MB_OK|MB_ICONERROR);
    return;
  }
  // find and delete shortcuts
  try  {
    olxstr sf = TShellUtil::GetSpecialFolderLocation(fiStartMenu) + "Olex2.lnk";
    if( TEFile::Exists( sf ) )  TEFile::DelFile( sf );
    sf = TShellUtil::GetSpecialFolderLocation(fiDesktop) + "Olex2.lnk";
    if( TEFile::Exists( sf ) )  TEFile::DelFile( sf );
  }
  catch( const TExceptionBase& exc )  {
    Application->MessageBoxA("Could not remove shortcuts", "Error", MB_OK|MB_ICONERROR);
  }
  Application->Terminate();
}
//---------------------------------------------------------------------------
bool TfMain::CleanInstallationFolder(TFSItem& item)  {
  if( !item.IsFolder() && item.GetParent() != NULL )  return false;
  for( int i=0; i < item.Count(); i++ )  {
    if( item.Item(i).IsFolder() )
      CleanInstallationFolder( item.Item(i) );
    else
      TEFile::DelFile( OlexInstalledPath + item.Item(i).GetFullName() );
  }
  if( (item.GetParent() != NULL) && TEFile::ChangeDir( OlexInstalledPath + item.GetFullName() ) )  {
    TStrList leftItems;
    TEFile::ListCurrentDir(leftItems, "*.*", sefAll);
    if( !leftItems.Count() )  {
      TEFile::ChangeDir( OlexInstalledPath );
      TEFile::RmDir( item.GetFullName() );
    }
  }
  return true;
}
//---------------------------------------------------------------------------
bool TfMain::LaunchFile( const olxstr& fileName, bool do_exit )  {
  STARTUPINFO si;
  PROCESS_INFORMATION ProcessInfo;
  AnsiString Tmp;
  Tmp += fileName.c_str();
  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb = sizeof(STARTUPINFO);
  si.wShowWindow = SW_SHOW;
  si.dwFlags = STARTF_USESHOWWINDOW;

  // Launch the child process.
  if( !CreateProcess(
        Tmp.c_str(),
        NULL,
        NULL, NULL,   true,
        0, NULL,
        NULL,
        &si, &ProcessInfo))
  {
    Application->MessageBox( (AnsiString("Could not start ") += fileName.c_str()).c_str(),
      "Error",
      MB_OK|MB_ICONERROR);
      return false;
  }
  if( do_exit )
    Application->Terminate();
  return true;
}
//---------------------------------------------------------------------------

void __fastcall TfMain::cbProxyClick(TObject *Sender)  {
  frMain->eProxy->Enabled = frMain->cbProxy->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TfMain::FormPaint(TObject *Sender)  {
  Canvas->Rectangle(0, 0, Width, Height);
}
//---------------------------------------------------------------------------

void __fastcall TfMain::sbPickZipClick(TObject *Sender)  {
  if( frMain->dlgOpen->Execute() )  {
    frMain->cbRepository->Text = frMain->dlgOpen->FileName;
  }
}
//---------------------------------------------------------------------------


void __fastcall TfMain::FormShow(TObject *Sender)  {
  if( OlexInstalled )  return;
  try  {
    frMain->cbRepository->Items->Clear();
    frMain->cbRepository->Text = "";
    updater::UpdateAPI api;
    TStrList repos;
    api.GetAvailableRepositories(repos);
    if( repos.IsEmpty() )  return;
    for( int i=0; i < repos.Count(); i++ )
      frMain->cbRepository->Items->Add( repos[i].u_str());
    frMain->cbRepository->Text = repos[0].u_str();
  }
  catch(...)  {
    Application->MessageBox( "Could not discover any Olex2 repositories, only offline installation will be available",
      "Error",
      MB_OK|MB_ICONERROR);
  }
  olxstr zipfn( Bapp->GetBaseDir() + "olex2.zip" );
  if( TEFile::Exists(zipfn) )  {
    if( !TEFile::IsAbsolutePath(zipfn) )  {
      zipfn = TEFile::CurrentDir();
      zipfn << "\\olex2.zip";
    }
    frMain->cbRepository->Text = zipfn.c_str();
  }
}
//---------------------------------------------------------------------------

