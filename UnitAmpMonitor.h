//---------------------------------------------------------------------------

#ifndef UnitAmpMonitorH
#define UnitAmpMonitorH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Dialogs.hpp>
#include <System.Actions.hpp>
#include <Vcl.ActnList.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Menus.hpp>
#include <Vcl.AppEvnts.hpp>
#include <Vcl.ComCtrls.hpp>

#include <fstream>
//---------------------------------------------------------------------------

class TMonitorMainForm : public TForm
{
__published:	// IDE-managed Components
	TLabel *Label1;
	TLabel *Label2;
	TEdit *EInterval;
	TEdit *EArgument;
	TButton *BHide;
	TButton *BExit;
	TOpenDialog *OpenDialog;
	TButton *BChooseFileArgs;
	TButton *BEdit;
	TButton *BApply;
	TButton *BCancel;
	TActionList *ActionList1;
	TAction *AApply;
	TAction *ACancelEdit;
	TAction *AEdit;
	TAction *AHide;
	TAction *AExitApp;
	TTimer *Timer;
	TEdit *EExecutableFile;
	TButton *BChooseFileExec;
	TLabel *Label3;
	TPopupMenu *PopupMenu1;
	TMenuItem *Showform1;
	TMenuItem *ExitApplication1;
	TTrayIcon *TrayIcon1;
	TEdit *ELogInactivityLimit;
	TLabel *Label4;
	TPageControl *PageControl1;
	TTabSheet *TabSheet1;
	TTabSheet *TSRestarts;
	TMemo *MCheck;
	TMemo *MRestarts;
	TLabel *Label5;
	TEdit *EMonitorProcessName;
	TLabel *Label6;
	TEdit *EKillProcessesOnRestart;
	TLabel *Label7;
	TEdit *EBatchOnRestart;
	TButton *BBatchOnRestart;
	TStatusBar *StatusBar;
	void __fastcall BChooseFileArgsClick(TObject *Sender);
	void __fastcall AApplyUpdate(TObject *Sender);
	void __fastcall AApplyExecute(TObject *Sender);
	void __fastcall ACancelEditExecute(TObject *Sender);
	void __fastcall AEditUpdate(TObject *Sender);
	void __fastcall AEditExecute(TObject *Sender);
	void __fastcall TimerTimer(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
	void __fastcall AExitAppExecute(TObject *Sender);
	void __fastcall TrayIcon1DblClick(TObject *Sender);
	void __fastcall AHideExecute(TObject *Sender);
private:	// User declarations
	bool     BEditMode;
	String   SFileName;
	String   SInterval;
	String   SArgument;
	TDateTime FLastCheck;
	void     ResetTimer();
	void     SaveSettings();
	void     LoadSettings();
	void	 SetEditMode(bool v);
	void 	 RelaunchProcessIfNotAvailable();
	bool     BExiting;
	std::ofstream *m_p_flog;
	std::ofstream & get_log_file();
	String 	ExtractLogDirectory();
protected:
 /*	BEGIN_MESSAGE_MAP
	   MESSAGE_HANDLER(WM_TRAY_ICON_MESSAGE,TMessage,TrayIcon_HandleMessage)
	END_MESSAGE_MAP(TForm)
*/
	TDateTime CheckAmpLogLastActivity();
	void LaunchProcess();
	void AddMessage(String s, bool bRestart = false);
public:		// User declarations
	__fastcall TMonitorMainForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMonitorMainForm *MonitorMainForm;
//---------------------------------------------------------------------------
#endif
