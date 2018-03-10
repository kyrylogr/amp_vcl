//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "UnitAmpMonitor.h"
#include "amp_utils.h"
#include "process_helper.h"
#include "TCHAR.H"
#include <auto_control>
#include "amp_ofstream.h"

using namespace amp;
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMonitorMainForm *MonitorMainForm;

#define WM_TRAY_ICON_ID 1997

//---------------------------------------------------------------------------
__fastcall TMonitorMainForm::TMonitorMainForm(TComponent* Owner)
	: TForm(Owner)
{
	m_p_flog = NULL;
	SApplicationName = "AmpMonitor";
	LoadSettings();

	SetEditMode(false);
	FLastCheck = Now();
	try {
		int nCheckInterval = StrToInt(EInterval->Text);
		if (nCheckInterval > 0) {

				amp::increment_seconds(FLastCheck, 20 - nCheckInterval);
		}
	} catch (...) {
	}


//	ResetTimer();

//  TrayIcon_Create ("Amp monitor", Icon->Handle);
	BExiting = false;

	AddMessage("monitor was started.", true);

}
//---------------------------------------------------------------------------

void __fastcall TMonitorMainForm::BChooseFileArgsClick(TObject *Sender)
{
  TEdit* eSrc = Sender == BChooseFileExec ? EExecutableFile :
				 (Sender == BChooseFileArgs ? EArgument : EBatchOnRestart);
  String sext = Sender == BChooseFileExec ? "Executable files (*.exe)|*.exe" :
				 (Sender == BChooseFileArgs ? "XML files (*.xml)|*.xml" : "Batch files (*.bat)|*.bat");
  OpenDialog->FileName = eSrc->Text;
  OpenDialog->Filter = sext;
  if (OpenDialog->Execute()) {
	eSrc->Text = OpenDialog->FileName;
	SaveSettings();
  }
}
//---------------------------------------------------------------------------
void     TMonitorMainForm::ResetTimer()
{
	Timer->Interval = 1000;
	Timer->Enabled = true;
}

const char* SKEY_FILE_EXEC = "ExecutableFile";
const char* SKEY_ARGUMENTS = "ExecutableArguments";
const char* SKEY_INTERVAL =  "Interval";
const char* SKEY_LOG_INACTIVITY_LIMIT =  "LogInactivityLimit";
const char* SKEY_BATCH_ON_RESTART =  "BatchOnRestart";
const char* SKEY_MONITOR_PROCESS  =  "MonitorProcess";
const char* SKEY_KILL_PROCESSES_ON_RESTART  =  "KillProcessesOnRestart";


void     TMonitorMainForm::SaveSettings()
{
	SaveRegistryIntValue(SKEY_INTERVAL, StrToInt(EInterval->Text) );
	SaveRegistryStrValue(SKEY_FILE_EXEC, EExecutableFile->Text );
	SaveRegistryStrValue(SKEY_ARGUMENTS, EArgument->Text );
	SaveRegistryIntValue(SKEY_LOG_INACTIVITY_LIMIT, StrToInt(ELogInactivityLimit->Text) );

	SaveRegistryStrValue(SKEY_BATCH_ON_RESTART, EBatchOnRestart->Text );
	SaveRegistryStrValue(SKEY_MONITOR_PROCESS, EMonitorProcessName->Text );
	String sKillP = EKillProcessesOnRestart->Text;
	if (sKillP.IsEmpty()) {
		sKillP = " ";
	}
	SaveRegistryStrValue(SKEY_KILL_PROCESSES_ON_RESTART, EKillProcessesOnRestart->Text );

}

void     TMonitorMainForm::LoadSettings()
{
	EInterval->Text = LoadRegistryIntValue(SKEY_INTERVAL, 60 );
	EExecutableFile->Text = LoadRegistryStrValue(SKEY_FILE_EXEC, "amp.exe" );
	EArgument->Text = 	LoadRegistryStrValue(SKEY_ARGUMENTS, "" );
	ELogInactivityLimit->Text = LoadRegistryIntValue(SKEY_LOG_INACTIVITY_LIMIT, 3600 );

	EBatchOnRestart->Text = LoadRegistryStrValue(SKEY_BATCH_ON_RESTART, "" );
	EMonitorProcessName->Text = LoadRegistryStrValue(SKEY_MONITOR_PROCESS, "" );

	EKillProcessesOnRestart->Text = LoadRegistryStrValue(SKEY_KILL_PROCESSES_ON_RESTART, "EXCEL.EXE MSACCESS.EXE" );
}
//---------------------------------------------------------------------------
void __fastcall TMonitorMainForm::AApplyUpdate(TObject *Sender)
{
	TAction* pa = dynamic_cast<TAction*>(Sender);
	if (pa) {
		pa->Enabled = BEditMode ;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorMainForm::AApplyExecute(TObject *Sender)
{
	SetEditMode(false);
	SaveSettings();
//	ResetTimer();
}
//---------------------------------------------------------------------------
void __fastcall TMonitorMainForm::ACancelEditExecute(TObject *Sender)
{
	SetEditMode(false);
}
//---------------------------------------------------------------------------
void SetEditRO(TEdit* e, bool v)
{
	e->ReadOnly = v;
	e->Color = v ? clBtnFace : clWindow;
}
//---------------------------------------------------------------------------
void	 TMonitorMainForm::SetEditMode(bool v)
{
	BEditMode = v;
	SetEditRO(EInterval, !v);
	SetEditRO(EExecutableFile, !v);
	SetEditRO(EArgument, !v);
	SetEditRO(ELogInactivityLimit, !v);

	SetEditRO(EBatchOnRestart, !v);
	SetEditRO(EMonitorProcessName, !v);

	SetEditRO(EKillProcessesOnRestart, !v);
	BChooseFileExec->Enabled = v;
	BChooseFileArgs->Enabled = v;
	BBatchOnRestart->Enabled = v;
}
//---------------------------------------------------------------------------
void __fastcall TMonitorMainForm::AEditUpdate(TObject *Sender)
{
	TAction* pa = dynamic_cast<TAction*>(Sender);
	if (pa) {
		pa->Enabled = !BEditMode ;
	}
}
//---------------------------------------------------------------------------

void __fastcall TMonitorMainForm::AEditExecute(TObject *Sender)
{
	SFileName = EExecutableFile->Text;
	SInterval = EInterval->Text;
	SArgument = EArgument->Text;
	SetEditMode(true);
}
//---------------------------------------------------------------------------

void __fastcall TMonitorMainForm::TimerTimer(TObject *Sender)
{
	TDateTime tnow = Now();
	int nDiffSecs = amp::get_diff_seconds(FLastCheck, tnow);
	int nCheckInterval = StrToInt(EInterval->Text);
	if (nDiffSecs > nCheckInterval) {
   	StatusBar->Panels->Items[1]->Text = "checking...";
		RelaunchProcessIfNotAvailable();
		const int NMaxScreenMemoLogLines = 15;
		while (MCheck->Lines->Count>=NMaxScreenMemoLogLines) {
			MCheck->Lines->Delete(0);
		}
	} else {
		StatusBar->Panels->Items[1]->Text = "next check in " + IntToStr(nCheckInterval - nDiffSecs) +" seconds";
	}
}
//---------------------------------------------------------------------------

void 	 TMonitorMainForm::RelaunchProcessIfNotAvailable()
{
	//	check if process exists.
	FLastCheck = Now();
	String sProcessName = EMonitorProcessName->Text;
	if (sProcessName.IsEmpty())
		sProcessName = ExtractFileName(EExecutableFile->Text);

	AddMessage("Check process");
	bool bNeedProcessLaunch = false;
	if (!sProcessName.IsEmpty()) {
		if (!amp::ProcessExists(sProcessName.c_str())) {
			AddMessage("process '" +sProcessName + "' was not found.", true );
			bNeedProcessLaunch = true;
		} else {

			int nInactivityLimit = StrToInt(ELogInactivityLimit->Text);
			String sLogActivityMsg;
			if (nInactivityLimit) {
				int nMinInactivity = std_ext::ofstream::default_commit_interval*2;
				if (nInactivityLimit <= nMinInactivity ) {
					nInactivityLimit = nMinInactivity;
				}
				TDateTime tmLog = CheckAmpLogLastActivity();
				if ((double)tmLog!=0) {
					int nTimeDiff = get_diff_seconds(tmLog, Now());
					sLogActivityMsg = "Last log activity was at " + tmLog.DateTimeString() + " (" +IntToStr(nTimeDiff) + " seconds ago). Inactivity limit is " + IntToStr(nInactivityLimit) + " seconds.";
					if (nTimeDiff > nInactivityLimit) {
						bNeedProcessLaunch = true;
					}
				}	else {
					sLogActivityMsg = "Log activity was not found.";
					bNeedProcessLaunch = true;
				}
			} else {
				sLogActivityMsg = "no checks on log activity because of zero parameter value.";
			}
			AddMessage("process '" +sProcessName + "' exists.", bNeedProcessLaunch );
			AddMessage(sLogActivityMsg, bNeedProcessLaunch);
			if (bNeedProcessLaunch)	{
				String sMsg = "killing process. " ;
				if (amp::ATerminateProcess(sProcessName, true))
					sMsg += "ok";
				else
					sMsg += "failed";
				AddMessage(sMsg, true);
			}
		}
		if ( bNeedProcessLaunch) {
			LaunchProcess();
		}
	} else {
		AddMessage("could not extract executable name.") ;
	}
}
//---------------------------------------------------------------------------

TDateTime FileTimeToDateTime(FILETIME ft)
{
	TDateTime res(0);

	FILETIME ftLoc;
	SYSTEMTIME st;

	if (FileTimeToLocalFileTime(&ft, &ftLoc) && FileTimeToSystemTime(&ftLoc, &st))
	   res = SystemTimeToDateTime(st);
	return res;
}

String TMonitorMainForm::ExtractLogDirectory()
{
	value_guard<String> appName(SApplicationName, "AMP");
	String sLogDir = LoadRegistryStrValue("LogDirectory", "");
	if (sLogDir.IsEmpty()) {
		String sExeDir = ExtractFilePath(EExecutableFile->Text);
		if (sExeDir.IsEmpty()) {
			sExeDir = ExtractFilePath(Application->ExeName);
		}
		sLogDir = sExeDir;
	}
	return sLogDir;
}

TDateTime TMonitorMainForm::CheckAmpLogLastActivity()
{
	TDateTime res(0);

	String sLogDir = ExtractLogDirectory();
	String sLogFileMask = "amp_*.log";
	sLogFileMask = IncludeTrailingBackslash(sLogDir)+sLogFileMask;

	WIN32_FIND_DATA findFileData;
	HANDLE hFind;

	hFind = FindFirstFile(sLogFileMask.c_str(), &findFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		res = FileTimeToDateTime(findFileData.ftLastWriteTime);
		for ( ; FindNextFile(hFind, &findFileData); ) {
			TDateTime tcurr = FileTimeToDateTime(findFileData.ftLastWriteTime);
			if (tcurr > res)
				res = tcurr;
		}
		FindClose(hFind);
	}
	return res;
}

void TMonitorMainForm::LaunchProcess()
{
	String sKillP = EKillProcessesOnRestart->Text;
	sKillP = sKillP.Trim();
	if (!sKillP.IsEmpty()) {
		AddMessage("killing processes: " + sKillP, true);
		for (int i=0; sKillP.Length(); ) {
			String sCurrentP;
			i = sKillP.LastDelimiter(" ");
			if (i>0) {
				sCurrentP = sKillP.SubString(i+1, sKillP.Length()-i);
				sKillP.SetLength(i-1);
			} else {
				sCurrentP = sKillP;
				sKillP.SetLength(0);
			}
			if (!sCurrentP.IsEmpty()) {
				amp::ATerminateProcess(sCurrentP);
			}
		}
	}
	if (!EBatchOnRestart->Text.IsEmpty()) {
		AddMessage("executing prestart batch: " + sKillP, true);
		String sCmd = "cmd.exe /c \"" + EBatchOnRestart->Text + "\"";
		amp::LaunchProcess( sCmd, true);
	}

	String sCmd;
		sCmd = "\"" + EExecutableFile->Text + "\"";
	if (!EArgument->Text.IsEmpty()) {
		sCmd += " ";
		sCmd += "\"" + EArgument->Text + "\"";
	}

	AddMessage("About to launch process with command line: " + sCmd, true);
	if (amp::LaunchProcess(sCmd)) {
		AddMessage("process launched at " + Now().DateTimeString() + ".", true) ;
	} else {
		AddMessage("ERROR: process launch failed. Error code:  " + IntToStr((int)GetLastError()) + "." , true) ;
	}
}

void __fastcall TMonitorMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
	Action = caHide;
}
//---------------------------------------------------------------------------

void __fastcall TMonitorMainForm::FormCloseQuery(TObject *Sender, bool &CanClose)
{
	if (!BExiting) {
		CanClose = false;
		Visible = false;
	}
}
//---------------------------------------------------------------------------


void __fastcall TMonitorMainForm::AExitAppExecute(TObject *Sender)
{
	BExiting = true;
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TMonitorMainForm::TrayIcon1DblClick(TObject *Sender)
{
	if (!Visible)
	  Show();
	else // already visible, so move it forward
	  Application->BringToFront();
}
//---------------------------------------------------------------------------

void __fastcall TMonitorMainForm::AHideExecute(TObject *Sender)
{
	Hide();
}
//---------------------------------------------------------------------------

void TMonitorMainForm::AddMessage(String s, bool bRestart)
{
	TDateTime d = Now();
	String message =  d.FormatString("dd.mm.yy hh:nn:ss") + " | ";

	message += s;
	MCheck->Lines->Add(message);

	if (bRestart) {
		MRestarts->Lines->Add(message);
		get_log_file() << AnsiString(message).c_str() << std::endl;
	}
}
//---------------------------------------------------------------------------

std::ofstream & TMonitorMainForm::get_log_file()
{
	if (!m_p_flog) {
		String s = "ampmonitor_" + Now().FormatString("yyyymmdd_hhnnss");
		String sNameLog = s+".log";

		String sDirectory = ExtractLogDirectory();
		if (!sDirectory.IsEmpty()) {
			sDirectory = IncludeTrailingBackslash(sDirectory);
			sNameLog = sDirectory + sNameLog;
		}
		m_p_flog = new std::ofstream(sNameLog.c_str(), std::ios_base::app);
	}
	return *m_p_flog;
}
//---------------------------------------------------------------------------
