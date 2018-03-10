//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "UnitSettings.h"
#include "module_performer.h"
#include "UnitMain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormSettings *FormSettings;
//---------------------------------------------------------------------------
__fastcall TFormSettings::TFormSettings(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------

void TFormSettings::FromPerformer(amp::performer* p)
{
	EDefaultTimeout->Text = IntToStr(p->get_default_timeout());
	EDefaultTimeoutComplex->Text = IntToStr(p->get_default_timeout_complex());
	CBTimeoutWithoutWait->Checked = p->get_timeout_without_wait();
	ELogDirectory->Text = p->get_log_directory();
	ELogClearPeriod->Text = IntToStr(p->get_log_clear_period());
	RGEnabled->ItemIndex = p->get_disable_timeouts();

	CBWriteCSVLogs->Checked = p->get_write_csv_logs();

	CBLeaveWindowsAfterBreak->Checked = p->get_leave_nested_windows_after_break();
	CBDetailedLog->Checked = p->get_detailed_log();

	CBAppsExitCodesErrors->Checked = p->get_apps_exitcode_as_error();
	CBCommandExitCodeErrors->Checked = p->get_cmd_exitcode_as_error();
}

void TFormSettings::ToPerformer(amp::performer* p)
{
	p->set_default_timeout(StrToInt(EDefaultTimeout->Text));

	p->set_default_timeout_complex(StrToInt(EDefaultTimeoutComplex->Text));
	p->set_timeout_without_wait(CBTimeoutWithoutWait->Checked);
	p->set_log_directory(ELogDirectory->Text);
	p->set_log_clear_period(StrToInt(ELogClearPeriod->Text));

	p->set_apps_exitcode_as_error(CBAppsExitCodesErrors->Checked);
	p->set_cmd_exitcode_as_error(CBCommandExitCodeErrors->Checked);

	p->set_disable_timeouts((amp::ETimeoutOptions)RGEnabled->ItemIndex);

	p->set_write_csv_logs(CBWriteCSVLogs->Checked);

	p->set_leave_nested_windows_after_break(CBLeaveWindowsAfterBreak->Checked);
	p->set_detailed_log(CBDetailedLog->Checked);
}


void TFormSettings::FromForm(TFormMain* p)
{
	EFontSize->Text = IntToStr(p->FontSizePct);

	CBExecuteDefaultModule->Checked = p->ExecuteDefaultModule;

	EDefaultModule->Text = p->DefaultModule;
}

void TFormSettings::ToForm(TFormMain* p)
{
	p->FontSizePct = StrToInt(EFontSize->Text);

	p->ExecuteDefaultModule = CBExecuteDefaultModule->Checked;

	p->DefaultModule = EDefaultModule->Text;
}

void TFormSettings::SetEnabledEdit(bool v)
{
	BNOK->Visible = v;
	LDisabled->Visible = !v;
	if (!v)
		Caption = "Settings view (*)";
}

void __fastcall TFormSettings::BChooseFileClick(TObject *Sender)
{
  OpenDialog->FileName = EDefaultModule->Text;
  if (OpenDialog->Execute())
	EDefaultModule->Text = OpenDialog->FileName;
}
//---------------------------------------------------------------------------

