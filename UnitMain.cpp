//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "UnitMain.h"
#include "UnitAbout.h"
#include "UnitSettings.h"
#include "control_module_struct.h"
#include "module_performer.h"
#include "execution_thread.h"
#include <Xml.adomxmldom.hpp>
#include <System.IOUtils.hpp>

#include <Vcl.Dialogs.hpp>
#include "version.hpp"
#include "amp_utils.h"

using namespace amp;

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "Xml.adomxmldom"
#pragma resource "*.dfm"

TFormMain *FormMain;

bool TFormMain::ExecuteDefaultModule=false;

String TFormMain::DefaultModule;

int TFormMain::FontSizePct=100;

int TFormMain::FontInitialSize=0;

int TFormMain::SplitterPosition=0;

std::ofstream * p_UIThreadLog = NULL;

void TFormMain::add_ui_log_message(String s)
{
#ifdef EXTRA_DEBUG_LOGS
	bool bDo =false;
	if (FPerformer)
		bDo = FPerformer->get_root_performer().get_detailed_log();
	if (!bDo)
		return;
	TDateTime d = Now();
	String message =  d.FormatString("dd.mm.yy hh:nn:ss") + " | ";
	message += s;
	if (p_UIThreadLog) {
		(*p_UIThreadLog) << AnsiString(message).c_str() << std::endl;
	}
#endif
}

//---------------------------------------------------------------------------
__fastcall TFormMain::TFormMain(TComponent* Owner)
	: TForm(Owner), FControlModule(NULL),
	FPerformer ( new amp::performer(this, TreeViewModels, MemoLog))
{
#ifdef EXTRA_DEBUG_LOGS
	if 	(!p_UIThreadLog) {
		String sNameLog = "ampui.log";
		String sDirectory = ExtractFilePath(Application->ExeName);
		sDirectory = IncludeTrailingBackslash(sDirectory);
		sNameLog = sDirectory + sNameLog;

		p_UIThreadLog = new std::ofstream(sNameLog.c_str(), std::ios_base::app);
	}
#endif


	FStartAfterShow = false;
	FMRList.OnClick = AOpenMR;
	FMRList.MaxItems = 20;
	FMRList.MenuItem = MIReopen;

	if (StatusBar->Panels->Count>1) {
		StatusBar->Panels->Items[StatusBar->Panels->Count-1]->Text = "version " + version().get_product_version();
	}


}
//---------------------------------------------------------------------------
__fastcall TFormMain::~TFormMain()
{
	delete FControlModule;
	delete FPerformer;
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::AStatusBarUpdate(TObject *Sender)
{
	TAction* pa = dynamic_cast<TAction*>(Sender);
	if (pa) {
		pa->Checked = StatusBar->Visible;
	}
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::AStatusBarExecute(TObject *Sender)
{
	StatusBar->Visible = !StatusBar->Visible;
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::AToolbarUpdate(TObject *Sender)
{
	TAction* pa = dynamic_cast<TAction*>(Sender);
	if (pa) {
		pa->Checked = ToolBar->Visible;
	}
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::AToolbarExecute(TObject *Sender)
{
	ToolBar->Visible = !ToolBar->Visible;;
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::AAboutExecute(TObject *Sender)
{
	TFormAbout *f = new TFormAbout(NULL);
	f->PopupParent = this;

	TFormStyle fs = FormStyle;
	try {
		FormStyle = fsNormal;
		f->ShowModal();
	}__finally {
		if (fs==fsStayOnTop) {
			FormStyle=fs;
		}
		delete f;
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::AStayOnTopUpdate(TObject *Sender)
{
	TAction* pa = dynamic_cast<TAction*>(Sender);
	if (pa) {
		pa->Checked = FormStyle == fsStayOnTop;
		int nNewIndex = FormStyle == fsStayOnTop ? 12 : 10;
		if (nNewIndex!=pa->ImageIndex) {
			pa->ImageIndex = nNewIndex  ;
			TBStayOnTop->ImageIndex = nNewIndex  ;
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::AStayOnTopExecute(TObject *Sender)
{
	if (FormStyle == fsStayOnTop)
		FormStyle = fsNormal;
	else
		FormStyle = fsStayOnTop;
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::AShowDocumentAppOnRunExecute(TObject *Sender)
{
	FPerformer->show_managed_apps(!FPerformer->show_managed_apps());
}
//---------------------------------------------------------------------------

int ImageIndexForRunItem(amp::run_type rt)
{
	static int nImgIdx[amp::rtUndefined] = {5,6, 15 , 16 , 9 , 14};
	if (rt > amp::rtUndefined) {
		throw Exception("Run type out of range "+IntToStr(rt));
	}
	return nImgIdx [rt];
}

void TFormMain::LoadTreeViewFromModel()
{
	TreeViewModels->Items->Clear();
	TTreeNode* nodeHead = TreeViewModels->Items->AddChild(NULL,FControlModule->def_node_caption());
	nodeHead->ImageIndex = 0;

	for (int i=0; i < FControlModule->model_count(); i++) {
		amp::model &model = FControlModule->get_model(i);
		TTreeNode* nodeModel = TreeViewModels->Items->Add(nodeHead,model.name.c_str());
		nodeModel->ImageIndex = 1;
		nodeModel->SelectedIndex = 1;
		for (int k=0; k < model.run_count(); k++) {
			amp::run_item & run = model.get_run(k);
			TTreeNode* nodeRun = TreeViewModels->Items->AddChild(nodeModel,run.name.c_str());
			nodeRun->ImageIndex = ImageIndexForRunItem(run.type) ;
			nodeRun->SelectedIndex = nodeRun->ImageIndex;
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::LoadFromFile(String sfname)
{
	if (FControlModule) {
		delete FControlModule;
	}

	FControlModule = new amp::control_module();
	FControlModule->load_from_file(sfname.c_str());

	FCurrentFileName	 =   sfname;
	Caption = "Ansergy AMP - "+ FCurrentFileName;
	LoadTreeViewFromModel();
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::AOpenXMLExecute(TObject *Sender)
{
	TOpenDialog * dlg = new TOpenDialog(NULL);
	std::unique_ptr<TOpenDialog> _freelater(dlg);

	dlg->Filter = "XML files (*.xml)|*.XML";
	if (dlg->Execute()) {
		LoadFromFile(dlg->FileName);
		FMRList.SetItem(dlg->FileName);
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::AOpenMR(TObject *Sender)
{
	TMenuItem* mi = dynamic_cast<TMenuItem*>(Sender);
	if (mi) {
		String sData = FMRList.ExtractName(mi);
		LoadFromFile(sData);
		FMRList.SetItem(sData);
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::AReloadXMLUpdate(TObject *Sender)
{
	TAction* pa = dynamic_cast<TAction*>(Sender);
	if (pa) {
		pa->Enabled = !FCurrentFileName.IsEmpty() && !FPerformer->is_running();
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::AReloadXMLExecute(TObject *Sender)
{
	delete FControlModule;
	FControlModule = new amp::control_module();
	FControlModule->load_from_file(	FCurrentFileName);
	LoadTreeViewFromModel();
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::AExpandAllModelsUpdate(TObject *Sender)
{
	TAction* pa = dynamic_cast<TAction*>(Sender);
	if (pa) {
		pa->Enabled = !FCurrentFileName.IsEmpty();
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ACollapseAllNodesExecute(TObject *Sender)
{
	TreeViewModels->FullCollapse();
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::AExpandAllModelsExecute(TObject *Sender)
{
	TreeViewModels->FullExpand();
	if (TreeViewModels->Items->Count) {
		TreeViewModels->Items->Item[0]->MakeVisible();
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::AProcessStopUpdate(TObject *Sender)
{
	TAction* pa = dynamic_cast<TAction*>(Sender);
	if (pa) {
		pa->Enabled = FPerformer->is_running() && FPerformer->is_breakable();
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::AProcessRunUpdate(TObject *Sender)
{
	TAction* pa = dynamic_cast<TAction*>(Sender);
	if (pa) {
		pa->Enabled = !FPerformer->is_running() && !FCurrentFileName.IsEmpty() ;
	}
}
//---------------------------------------------------------------------------
void TFormMain::DoProcessRunExecute(TTreeNode* pnode)
{
	amp::performer& rootp = FPerformer->get_root_performer();
	amp::MtxLock l(rootp.FMtxProcessingElements);
	if (!rootp.elements_in_process.empty()) {
		throw Exception("There are some child processes running. Stop them and try again.");
	}
	PerformerThread * FPerformerThread = new PerformerThread(*FPerformer);
	if (!pnode || pnode->Parent == NULL && pnode->Index==0 ) {
		FPerformerThread->execute(*FControlModule);
	} else if (pnode) {
		if (pnode->Level==0 && pnode->Index>0) {
			amp::model &m = FControlModule->get_model(pnode->Index-1);
			FPerformerThread->execute(m);
		} else if (pnode->Level==1 ) {
			TTreeNode* parentNode = pnode->Parent;
			amp::model &m = FControlModule->get_model(parentNode->Index-1);
			amp::run_item &item = m.get_run(pnode->Index);
			FPerformerThread->execute(item);
		} else
			delete FPerformerThread;
	} else
		delete FPerformerThread;
}

//---------------------------------------------------------------------------
void __fastcall TFormMain::AProcessRunExecute(TObject *Sender)
{
	TTreeNode* pnode = TreeViewModels->Selected;
	DoProcessRunExecute(pnode);
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::AProcessStopExecute(TObject *Sender)
{
	if (FPerformer->is_running())
		FPerformer->break_signal(amp::breakStop);
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::FormCloseQuery(TObject *Sender, bool &CanClose)
{
	if (FPerformer->is_running()) {
		ShowMessage("Can not exit application when the process is running. Please stop the process.");
		CanClose = false;
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::AStopAtErrorExecute(TObject *Sender)
{
	FPerformer->set_stop_at_error(!FPerformer->stop_at_error());
}
//---------------------------------------------------------------------------


void __fastcall TFormMain::AStopAtErrorUpdate(TObject *Sender)
{
	TAction* pa = dynamic_cast<TAction*>(Sender);
	if (pa) {
		pa->Checked = FPerformer->stop_at_error() ;
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::AShowDocumentAppOnRunUpdate(TObject *Sender)
{
	TAction* pa = dynamic_cast<TAction*>(Sender);
	if (pa) {
		pa->Checked = FPerformer->show_managed_apps() ;
		int nNewImg = pa->Checked ? 14 : 9 ;
		if (pa->ImageIndex != nNewImg) {
			pa->ImageIndex = nNewImg;
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::AProcessStepOverExecute(TObject *Sender)
{
	if (FPerformer->is_running())
		FPerformer->break_signal(amp::breakSkipElement);
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::AProcessStepOverUpdate(TObject *Sender)
{
	TAction* pa = dynamic_cast<TAction*>(Sender);
	if (pa) {
		pa->Enabled = FPerformer->is_running() && FPerformer->is_breakable();
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::TimerProcessingElementsUpdateTimer(TObject *Sender)
{
	if (FPerformer ) {
		add_ui_log_message("in timer " + IntToStr(FPerformer->nested_level()));
		amp::performer& rootp = FPerformer->get_root_performer();
		amp::MtxLock l(rootp.FMtxProcessingElements);
		add_ui_log_message("in timer mutex acquired " + IntToStr(FPerformer->nested_level()));
		if (FPerformer->is_running()) {
			int z=0;
			for (amp::t_processing_elements::iterator i=rootp.elements_in_process.begin();
					i!=rootp.elements_in_process.end(); i++,z++) {
				amp::processing_element * pe = (*i);
				if (pe && pe->node && pe->form == this) {
					if (!pe->node->IsVisible) {
						if (pe->node->Parent && pe->node->Parent->Expanded) {
							pe->node->Parent->Expand(true);
						}
						pe->node->MakeVisible();
					}
					add_ui_log_message("in timer about get node caption " + IntToStr(z) + " " + IntToStr(FPerformer->nested_level()));
					pe->node->Text = pe->get_node_caption();
					add_ui_log_message("in timer node text updated " + IntToStr(z) + " " + IntToStr(FPerformer->nested_level()));
				}
			}
			add_ui_log_message("in timer about to check vba error " + IntToStr(FPerformer->nested_level()));
			bool b = FPerformer->check_managed_app_vba_error();
			add_ui_log_message("in timer about to check vba error checked " + IntToStr(FPerformer->nested_level()));
			if (b)
				return;
		}
		if (FPerformer->is_running() && !FPerformer->get_parent()) {
			for (amp::t_processing_elements::iterator i=rootp.elements_in_process.begin();
					i!=rootp.elements_in_process.end(); i++) {
				amp::processing_element * pe = (*i);
				if (pe->timeout_element
						  && pe->check_timeout(FPerformer->get_timeout_without_wait()) ) {
					rootp.break_signal(amp::breakTimeout, pe->timeout_element);
					break;
				}
			}
		}
		add_ui_log_message("in timer about to exit and release mtx " + IntToStr(FPerformer->nested_level()));


	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::TreeViewModelsAdvancedCustomDrawItem(TCustomTreeView *Sender,
		  TTreeNode *Node, TCustomDrawState State, TCustomDrawStage Stage,
		  bool &PaintImages, bool &DefaultDraw)
{

	if (Stage==cdPrePaint ) {
		String ntext = Node->Text;
		/*	As a temporary soultion will check node text for specific pattern
		*	in future can switch to using data attribute and set some state information to it.
		*/
		if (!State.Contains(cdsSelected)  ) {
			if (ntext.Pos(" - Fail")) {
				Sender->Canvas->Font->Color = clRed;
			}
			if (ntext.Pos(" - OK")) {
				Sender->Canvas->Font->Color = clBlue;
			}
			if (ntext.Pos(" - skipped")) {
				Sender->Canvas->Font->Color = clOlive;
			}
		}
		if (ntext.Pos(" - Running") || ntext.Pos(" - Waiting") ) {
			if (Node->Parent || Node->Index!=0) {
				Sender->Canvas->Font->Style = Sender->Canvas->Font->Style << fsBold;
			}
/*			if (!State.Contains(cdsSelected)  ) {
				Sender->Canvas->Font->Color = clGreen;
			}
*/
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ANestedXMLInWindowUpdate(TObject *Sender)
{
	TAction* pa = dynamic_cast<TAction*>(Sender);
	if (pa) {
		pa->Checked = FPerformer->get_show_nested_control_in_window();
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ANestedXMLInWindowExecute(TObject *Sender)
{
	FPerformer->set_show_nested_control_in_window( !FPerformer->get_show_nested_control_in_window());
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ATimeoutWithoutWaitExecute(TObject *Sender)
{
	FPerformer->set_timeout_without_wait( !FPerformer->get_timeout_without_wait() );
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ATimeoutWithoutWaitUpdate(TObject *Sender)
{
	TAction* pa = dynamic_cast<TAction*>(Sender);
	if (pa) {
		pa->Checked = FPerformer->get_timeout_without_wait();
	}
}
//---------------------------------------------------------------------------

const char* SKEY_TIMEOUT_WITHOUT_WAIT_TIME = "TimeoutWithoutWaitTime";
const char* SKEY_SHOW_WINDOW_FOR_NESTED_XML = "ShowWindowForNestedXML";
const char* SKEY_SHOW_MANAGED_APPS = "ShowManagedApps";
const char* SKEY_STAY_ON_TOP       = "StayOnTop";
const char* SKEY_STOP_ON_ERROR     = "StopOnError";
const char* SKEY_MRU_LIST     	   = "MRUList";
const char* SKEY_DEFAULT_TIMEOUT   = "DefaultTimeout";
const char* SKEY_DEFAULT_TIMEOUT_COMPLEX = "DefaultTimeoutComplex";
const char* SKEY_LOG_DIRECTORY     = "LogDirectory";
const char* SKEY_LOG_CLEAR_PERIOD = "LogClearPeriod";
const char* SKEY_DISABLE_TIMEOUTS = "DisableTimeous";
const char* SKEY_LEAVE_NESTED_WINDOW_AFTER_BREAK = "LeaveNestedWindowAfterBreak";
const char* SKEY_FONT_SIZE = "FontSize";
const char* SKEY_SPLITTER_POS = "SplitterPosition";
const char* SKEY_DETAILED_LOG = "DetailedLog"   ;
const char* SKEY_DEFAULT_MODULE = "DefaultModule" ;
const char* SKEY_EXECUTE_DEFAULT_MODULE = "ExecuteDefaultModule" ;
const char* SKEY_CMD_EXITCODE_AS_ERROR = "CmdExitcodeAsError" ;
const char* SKEY_APPS_EXITCODE_AS_ERROR = "AppsExitcodeAsError" ;


String SKEY_APP = "Software\\Ansergy\\AMP";

void TFormMain::LoadOptions()
{
	FPerformer->set_show_nested_control_in_window(
			LoadRegistryIntValue(SKEY_SHOW_WINDOW_FOR_NESTED_XML, FPerformer->get_show_nested_control_in_window()));

	FPerformer->set_timeout_without_wait(
			LoadRegistryIntValue(SKEY_TIMEOUT_WITHOUT_WAIT_TIME, FPerformer->get_timeout_without_wait()));

	FPerformer->show_managed_apps(
			LoadRegistryIntValue(SKEY_SHOW_MANAGED_APPS, FPerformer->show_managed_apps()));

	FPerformer->set_cmd_exitcode_as_error(
			LoadRegistryIntValue(SKEY_CMD_EXITCODE_AS_ERROR, FPerformer->get_cmd_exitcode_as_error()));

	FPerformer->set_apps_exitcode_as_error(
			LoadRegistryIntValue(SKEY_APPS_EXITCODE_AS_ERROR, FPerformer->get_apps_exitcode_as_error()));


//	FPerformer->set_stop_at_error(LoadRegistryIntValue(SKEY_STOP_ON_ERROR, FPerformer->stop_at_error())) ;
	FPerformer->set_stop_at_error( false );

	FPerformer->set_default_timeout(
				LoadRegistryIntValue(SKEY_DEFAULT_TIMEOUT, FPerformer->get_default_timeout()) );

	int to = LoadRegistryIntValue(SKEY_DISABLE_TIMEOUTS, FPerformer->get_disable_timeouts());
	if (to >= toAllEnabled && to <= toAllDisable)
		FPerformer->set_disable_timeouts( (ETimeoutOptions)to );

	FPerformer->set_default_timeout_complex(
				LoadRegistryIntValue(SKEY_DEFAULT_TIMEOUT_COMPLEX, FPerformer->get_default_timeout_complex()) );

	FPerformer->set_leave_nested_windows_after_break(
				LoadRegistryIntValue(SKEY_LEAVE_NESTED_WINDOW_AFTER_BREAK, FPerformer->get_leave_nested_windows_after_break()) );

	FontSizePct =
				LoadRegistryIntValue(SKEY_FONT_SIZE, FontSizePct );

	SplitterPosition =
				LoadRegistryIntValue(SKEY_SPLITTER_POS, 0 );

	ExecuteDefaultModule =
				LoadRegistryIntValue(SKEY_EXECUTE_DEFAULT_MODULE, 0 );

	DefaultModule =
				LoadRegistryStrValue(SKEY_DEFAULT_MODULE);

	FPerformer->set_log_directory(
				LoadRegistryStrValue(SKEY_LOG_DIRECTORY));

	FPerformer->set_log_clear_period(
				LoadRegistryIntValue(	SKEY_LOG_CLEAR_PERIOD, FPerformer->get_log_clear_period()));

	FPerformer->set_detailed_log(LoadRegistryIntValue(SKEY_DETAILED_LOG, FPerformer->get_detailed_log()));

	if (LoadRegistryIntValue(SKEY_STAY_ON_TOP, FormStyle == fsStayOnTop)) {
		FormStyle = fsStayOnTop;
	}

	FMRList.TextData = LoadRegistryStrValue(SKEY_MRU_LIST);
}


void TFormMain::SaveOptions()
{
	SaveRegistryIntValue(SKEY_SHOW_WINDOW_FOR_NESTED_XML, FPerformer->get_show_nested_control_in_window());

	SaveRegistryIntValue(SKEY_TIMEOUT_WITHOUT_WAIT_TIME, FPerformer->get_timeout_without_wait());

	SaveRegistryIntValue(SKEY_SHOW_MANAGED_APPS, FPerformer->show_managed_apps());

	SaveRegistryIntValue(SKEY_DETAILED_LOG, FPerformer->get_detailed_log());

	SaveRegistryIntValue(SKEY_STAY_ON_TOP, FormStyle == fsStayOnTop) ;

//	SaveRegistryIntValue(SKEY_STOP_ON_ERROR, FPerformer->stop_at_error() ) ;

	SaveRegistryStrValue(SKEY_MRU_LIST, FMRList.TextData ) ;

	SaveRegistryIntValue(SKEY_DEFAULT_TIMEOUT, FPerformer->get_default_timeout());

	SaveRegistryIntValue(SKEY_DEFAULT_TIMEOUT_COMPLEX, FPerformer->get_default_timeout_complex() );

	SaveRegistryStrValue(SKEY_LOG_DIRECTORY, FPerformer->get_log_directory());

	SaveRegistryIntValue(SKEY_LEAVE_NESTED_WINDOW_AFTER_BREAK, FPerformer->get_leave_nested_windows_after_break());

	SaveRegistryIntValue(SKEY_CMD_EXITCODE_AS_ERROR, FPerformer->get_cmd_exitcode_as_error());

	SaveRegistryIntValue(SKEY_APPS_EXITCODE_AS_ERROR, FPerformer->get_apps_exitcode_as_error());

//	SaveRegistryStrValue(SKEY_LOG_DIRECTORY, FPerformer->get_log_directory());

	SaveRegistryIntValue(SKEY_LOG_CLEAR_PERIOD, FPerformer->get_log_clear_period());

	SaveRegistryIntValue(SKEY_DISABLE_TIMEOUTS, FPerformer->get_disable_timeouts());

	SaveRegistryIntValue(SKEY_SPLITTER_POS, TreeViewModels->Width );

	SaveRegistryIntValue(SKEY_FONT_SIZE, FontSizePct );

	SaveRegistryIntValue(SKEY_EXECUTE_DEFAULT_MODULE, ExecuteDefaultModule );

	SaveRegistryStrValue(SKEY_DEFAULT_MODULE, DefaultModule );
}

void TFormMain::SetFontSizeToUI()
{
	if ( !FontInitialSize )
		FontInitialSize = TreeViewModels->Font->Size;

	if ( FontSizePct<=0  )
		FontSizePct = 100;

	TreeViewModels->Font->Size = FontInitialSize* FontSizePct / 100;
}

void TFormMain::SetSplitterSizeToUI()
{
	if (this==FormMain) {
		if (SplitterPosition>0) {
			TreeViewModels->Width = SplitterPosition;
		} else {
			TreeViewModels->Width = ClientWidth/2 - SplitterTV->Width;
		}
	} else {
			TreeViewModels->Width = FormMain->TreeViewModels->Width;
	}
}

void __fastcall TFormMain::FormClose(TObject *Sender, TCloseAction &Action)
{
	if (this==FormMain) {
		SaveOptions();
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::FormShow(TObject *Sender)
{
	if (this==FormMain) {
		LoadOptions();
		if (FPerformer->get_log_clear_period()>0) {
			ClearFileLogs(FPerformer->get_log_clear_period());
		}
		if (FStartAfterShow) {
			FStartAfterShow  = false;
			DoProcessRunExecute(NULL);
		} else {
			if (!DefaultModule.IsEmpty()) {
				LoadFromFile(DefaultModule);
				if (ExecuteDefaultModule)
					DoProcessRunExecute(NULL);
			}
		}
	} else {

	}
	SetSplitterSizeToUI();
	SetFontSizeToUI();
}
//---------------------------------------------------------------------------

void TFormMain::NestedMode()
{
	TBOpen->Visible = false;
	MIExit->Action = AWindowClose;
	MIOpenXML->Visible = false;
	ASettings->Visible = false;
	MISettings->Visible = false;
}

void __fastcall TFormMain::AOpenXMLUpdate(TObject *Sender)
{
	TAction* pa = dynamic_cast<TAction*>(Sender);
	if (pa) {
		pa->Enabled = !FPerformer->is_running() ;
	}
}
//---------------------------------------------------------------------------

void TFormMain::ShowProgress(String s_caption, unsigned int pos_max, unsigned int pos_current, String info )
{
	LLongOperationCaption->Caption = s_caption;
	ProgressBar->Max = pos_max;
	ProgressBar->Position = pos_current;
	PProgress->Visible = true;
	LLO_Element->Caption = info;
}

void TFormMain::HideProgress()
{
	PProgress->Visible = false;
}


void __fastcall TFormMain::ASetDefaultTimeoutExecute(TObject *Sender)
{
	int icurrent = FPerformer->get_default_timeout();

	String sDesc[2] = { "timeout value for simple element", "timeout value for XML element" };
	String sVals[2];
	sVals[0] = IntToStr(FPerformer->get_default_timeout());
	sVals[1] = IntToStr(FPerformer->get_default_timeout_complex());

	if (InputQuery("Set Default Timeout in seconds", sDesc, 1 , sVals , 1 )) {
		icurrent = StrToInt(sVals[0]);
		if (icurrent>=0)
			FPerformer->set_default_timeout(icurrent);
		icurrent = StrToInt(sVals[1]);
		if (icurrent>=0)
			FPerformer->set_default_timeout_complex(icurrent);
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ACleaScreenLogExecute(TObject *Sender)
{
	MemoLog->Lines->Clear();
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ACleaScreenLogUpdate(TObject *Sender)
{
	TAction* pa = dynamic_cast<TAction*>(Sender);
	if (pa) {
		pa->Enabled = 	MemoLog->Lines->Count!=0;
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::AClearFileLogExecute(TObject *Sender)
{
	if (Application->MessageBox(_T("Are you sure you want to delete all log files?"), _T("Log files deletion"), MB_YESNOCANCEL)==IDYES) {
		ClearFileLogs(0);
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ASettingsUpdate(TObject *Sender)
{
/*	TAction* pa = dynamic_cast<TAction*>(Sender);
	if (pa) {
		pa->Enabled = this==FormMain && FPerformer && !FPerformer->is_running();
	}
*/
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ASettingsExecute(TObject *Sender)
{
	TFormSettings *f = new TFormSettings(NULL);
	std::unique_ptr <TForm> _freelater(f);
	f->FromPerformer(FPerformer);
	f->FromForm(this);
	TFormStyle fs = FormStyle;
	bool bEnabledEdit = this==FormMain && FPerformer && !FPerformer->is_running();
	f->SetEnabledEdit(bEnabledEdit);
	try {
		FormStyle = fsNormal;
		if (f->ShowModal()==mrOk) {
			f->ToPerformer(FPerformer);
			f->ToForm(this);
			SaveOptions();
			for (int i=0; i < Screen->FormCount; i++) {
				TFormMain* fSet = dynamic_cast<TFormMain*>( Screen->Forms[i] );
				if (fSet)
					fSet->SetFontSizeToUI();
			}
		}
	}__finally {
		if (fs==fsStayOnTop) {
			FormStyle=fs;
		}
	}
}
//---------------------------------------------------------------------------
void TFormMain::ClearFileLogs(int nolderThanDays)
{
	if (!nolderThanDays && FPerformer)
		FPerformer->delete_log_files();
	WIN32_FIND_DATAW fd;
	String sRootFolder = FPerformer->get_log_directory();
	if (sRootFolder.IsEmpty()) {
		sRootFolder = GetCurrentDir();
	}
	String sMask = "amp*.log";
	if (!sRootFolder.IsEmpty())
		sMask = IncludeTrailingBackslash(sRootFolder) + sMask;

	HANDLE hFind = FindFirstFile(sMask.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			String sFile = fd.cFileName;
			if (!sRootFolder.IsEmpty())
				sFile = IncludeTrailingBackslash(sRootFolder) + sFile;
			if (nolderThanDays>0) {
				TDateTime tCreated = TFile::GetCreationTime(sFile);
				TDateTime tDiff = Now() - tCreated;
				if ((double)tDiff < (double)nolderThanDays)
					continue;
			}
			DeleteFile( sFile);
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::AClearFileLogUpdate(TObject *Sender)
{
	TAction* pa = dynamic_cast<TAction*>(Sender);
	if (pa) {
		pa->Enabled = this==FormMain && FPerformer && !FPerformer->is_running();
	}
}
//---------------------------------------------------------------------------

void TFormMain::set_control_performer(amp::control_module *cm, amp::performer		*cp)
{
	delete FControlModule;
	delete FPerformer;
	FControlModule = cm;
	FPerformer = cp;
	LoadTreeViewFromModel();
}

void __fastcall TFormMain::ABringRunningWindowToFrontExecute(TObject *Sender)
{
	amp::MtxLock l(FPerformer->get_root_performer().FMtxProcessingElements);
	TFormMain* mf_result = NULL;
	for (int i = 0; i < Screen->FormCount; i++) {
		TFormMain* mf = dynamic_cast<TFormMain*>(Screen->Forms[i]);
		if (mf && mf->FPerformer->is_running()) {
			if (!mf_result || mf_result->FPerformer->nested_level() < mf->FPerformer->nested_level())
				mf_result = mf;
		}
	}
	if (mf_result) {
		mf_result->BringToFront();
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ABringRunningWindowToFrontUpdate(TObject *Sender)
{
	TAction* pa = dynamic_cast<TAction*>(Sender);
	if (pa) {
		pa->Enabled = FPerformer->get_show_nested_control_in_window();
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::FormActivate(TObject *Sender)
{
//	if (FormStyle==fsStayOnTop) {
//	BringToFront();
//	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::FormDeactivate(TObject *Sender)
{
//	SendToBack();
}
//---------------------------------------------------------------------------



void __fastcall TFormMain::StatusBarResize(TObject *Sender)
{
	if (	StatusBar->Panels->Count>1) {
		const int nPanelGap = 10;
		int nRightWidth=0;
		for (int i=1; i< StatusBar->Panels->Count  ; i++) {
			nRightWidth+= StatusBar->Panels->Items[i]->Width + nPanelGap;

		}
		StatusBar->Panels->Items[0]->Width = StatusBar->Width - nRightWidth - nPanelGap;

	}

}
//---------------------------------------------------------------------------

