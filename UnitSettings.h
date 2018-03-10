//---------------------------------------------------------------------------

#ifndef UnitSettingsH
#define UnitSettingsH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Dialogs.hpp>
//---------------------------------------------------------------------------


namespace amp {
  class performer;
};

class TFormMain;

/**		Settings dialog
*/


class TFormSettings : public TForm
{
__published:	// IDE-managed Components
	TButton *BNOK;
	TButton *Cancel;
	TGroupBox *GBTimeout;
	TLabel *Label1;
	TEdit *EDefaultTimeout;
	TLabel *Label2;
	TEdit *EDefaultTimeoutComplex;
	TLabel *Label3;
	TLabel *Label4;
	TGroupBox *GBLogs;
	TLabel *Label5;
	TEdit *ELogDirectory;
	TLabel *Label6;
	TEdit *ELogClearPeriod;
	TLabel *Label7;
	TCheckBox *CBWriteCSVLogs;
	TRadioGroup *RGEnabled;
	TCheckBox *CBTimeoutWithoutWait;
	TGroupBox *GBUI;
	TEdit *EFontSize;
	TCheckBox *CBLeaveWindowsAfterBreak;
	TLabel *Label8;
	TLabel *Label9;
	TCheckBox *CBDetailedLog;
	TLabel *LDisabled;
	TGroupBox *GBDefaultXML;
	TLabel *Label10;
	TEdit *EDefaultModule;
	TCheckBox *CBExecuteDefaultModule;
	TButton *BChooseFile;
	TOpenDialog *OpenDialog;
	TGroupBox *GroupBox1;
	TCheckBox *CBCommandExitCodeErrors;
	TCheckBox *CBAppsExitCodesErrors;
	void __fastcall BChooseFileClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
	void FromPerformer(amp::performer* p);
	void ToPerformer(amp::performer* p);
	void SetEnabledEdit(bool v);

	void FromForm(TFormMain* p);
	void ToForm(TFormMain* p);

	__fastcall TFormSettings(TComponent*	 Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFormSettings *FormSettings;
//---------------------------------------------------------------------------
#endif
