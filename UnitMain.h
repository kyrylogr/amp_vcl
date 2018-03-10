//---------------------------------------------------------------------------

#ifndef UnitMainH
#define UnitMainH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <System.Actions.hpp>
#include <Vcl.ActnList.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Menus.hpp>
#include <Vcl.StdActns.hpp>
#include <Vcl.ToolWin.hpp>
#include <Vcl.ImgList.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Xml.Win.msxmldom.hpp>
#include <Xml.XMLDoc.hpp>
#include <Xml.xmldom.hpp>
#include <Xml.XMLIntf.hpp>
#include "mrlist.h"
class PerformerThread;

namespace amp {
class control_module;
class performer;
};

/**		Main form class of the application.
*    Provides UI for loading and executing XML controlmodule files.
*		When nested xmls are executed new window is created.
*/

//---------------------------------------------------------------------------
class TFormMain : public TForm
{
friend class amp::performer;
__published:	// IDE-managed Components
	TMainMenu *MainMenu1;
	TMenuItem *MenuFile;
	TMenuItem *MIOpenXML;
	TMenuItem *MenuItem2;
	TMenuItem *MenuItem3;
	TMenuItem *MenuItem4;
	TMenuItem *MIExit;
	TMenuItem *MenuProcess;
	TMenuItem *MenuItem8;
	TMenuItem *MenuItem10;
	TMenuItem *MenuItem11;
	TMenuItem *MenuItem15;
	TMenuItem *MenuItem16;
	TMenuItem *MenuItem17;
	TMenuItem *MenuOptions;
	TMenuItem *MenuItem6;
	TMenuItem *MenuItem7;
	TMenuItem *MenuItem12;
	TMenuItem *MenuItem13;
	TMenuItem *MenuItem14;
	TMenuItem *MenuHelp;
	TMenuItem *MenuItem9;
	TActionList *ActionList1;
	TFileExit *FileExit1;
	TAction *AOpenXML;
	TAction *AReloadXML;
	TAction *AToolbar;
	TAction *AStatusBar;
	TAction *AAbout;
	TAction *AProcessRun;
	TAction *AProcessStop;
	TAction *AProcessStepOver;
	TAction *Action1;
	TAction *AShowDocumentAppOnRun;
	TAction *ACollapseAllNodes;
	TAction *AExpandAllModels;
	TAction *AStayOnTop;
	TToolBar *ToolBar;
	TStatusBar *StatusBar;
	TImageList *ImageListToolbar;
	TToolButton *TBOpen;
	TToolButton *ToolButton2;
	TToolButton *ToolButton3;
	TToolButton *ToolButton4;
	TToolButton *ToolButton5;
	TToolButton *ToolButton6;
	TToolButton *ToolButton7;
	TToolButton *ToolButton8;
	TToolButton *ToolButton9;
	TToolButton *ToolButton10;
	TToolButton *ToolButton11;
	TToolButton *TBStayOnTop;
	TPanel *PForm;
	TTreeView *TreeViewModels;
	TSplitter *SplitterTV;
	TImageList *ImageListTree;
	TAction *AStopAtError;
	TPopupMenu *PopupMenuTree;
	TMenuItem *Run1;
	TMenuItem *Stop1;
	TMenuItem *N1;
	TMenuItem *ReloadXML1;
	TMenuItem *N2;
	TMenuItem *ExpandAllNodes1;
	TMenuItem *CollapseAllNodes1;
	TToolButton *ToolButton13;
	TTimer *TimerProcessingElementsUpdate;
	TAction *ANestedXMLInWindow;
	TToolButton *ToolButton14;
	TAction *ATimeoutWithoutWait;
	TMenuItem *N3;
	TMenuItem *DoNotIncludeWaitTimeInTimeout1;
	TWindowClose *AWindowClose;
	TMenuItem *MIReopen;
	TPanel *PRight;
	TMemo *MemoLog;
	TPanel *PProgress;
	TLabel *LLongOperationCaption;
	TLabel *LLO_Element;
	TProgressBar *ProgressBar;
	TAction *ASetDefaultTimeout;
	TMenuItem *SetTimeout1;
	TAction *ACleaScreenLog;
	TAction *AClearFileLog;
	TMenuItem *ClearLogScreen1;
	TToolButton *ToolButton1;
	TMenuItem *ClearScreenLog1;
	TMenuItem *ClearFileLogs1;
	TAction *ASettings;
	TMenuItem *MISettings;
	TMenuItem *N4;
	TAction *ABringRunningWindowToFront;
	TToolButton *ToolButton15;
	void __fastcall AAboutExecute(TObject *Sender);
	void __fastcall AStatusBarExecute(TObject *Sender);
	void __fastcall AStatusBarUpdate(TObject *Sender);
	void __fastcall AToolbarExecute(TObject *Sender);
	void __fastcall AToolbarUpdate(TObject *Sender);
	void __fastcall AStayOnTopUpdate(TObject *Sender);
	void __fastcall AStayOnTopExecute(TObject *Sender);
	void __fastcall AShowDocumentAppOnRunExecute(TObject *Sender);
	void __fastcall AOpenXMLExecute(TObject *Sender);
	void __fastcall AReloadXMLUpdate(TObject *Sender);
	void __fastcall AReloadXMLExecute(TObject *Sender);
	void __fastcall AExpandAllModelsUpdate(TObject *Sender);
	void __fastcall ACollapseAllNodesExecute(TObject *Sender);
	void __fastcall AExpandAllModelsExecute(TObject *Sender);
	void __fastcall AProcessStopUpdate(TObject *Sender);
	void __fastcall AProcessRunUpdate(TObject *Sender);
	void __fastcall AProcessRunExecute(TObject *Sender);
	void __fastcall AProcessStopExecute(TObject *Sender);
	void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
	void __fastcall AStopAtErrorExecute(TObject *Sender);
	void __fastcall AStopAtErrorUpdate(TObject *Sender);
	void __fastcall AShowDocumentAppOnRunUpdate(TObject *Sender);
	void __fastcall AProcessStepOverExecute(TObject *Sender);
	void __fastcall AProcessStepOverUpdate(TObject *Sender);
	void __fastcall TimerProcessingElementsUpdateTimer(TObject *Sender);
	void __fastcall TreeViewModelsAdvancedCustomDrawItem(TCustomTreeView *Sender, TTreeNode *Node,
		  TCustomDrawState State, TCustomDrawStage Stage, bool &PaintImages,
		  bool &DefaultDraw);
	void __fastcall ANestedXMLInWindowUpdate(TObject *Sender);
	void __fastcall ANestedXMLInWindowExecute(TObject *Sender);
	void __fastcall ATimeoutWithoutWaitExecute(TObject *Sender);
	void __fastcall ATimeoutWithoutWaitUpdate(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall AOpenXMLUpdate(TObject *Sender);
	void __fastcall ASetDefaultTimeoutExecute(TObject *Sender);
	void __fastcall ACleaScreenLogExecute(TObject *Sender);
	void __fastcall ACleaScreenLogUpdate(TObject *Sender);
	void __fastcall AClearFileLogExecute(TObject *Sender);
	void __fastcall ASettingsUpdate(TObject *Sender);
	void __fastcall ASettingsExecute(TObject *Sender);
	void __fastcall AClearFileLogUpdate(TObject *Sender);
	void __fastcall ABringRunningWindowToFrontExecute(TObject *Sender);
	void __fastcall ABringRunningWindowToFrontUpdate(TObject *Sender);
	void __fastcall FormActivate(TObject *Sender);
	void __fastcall FormDeactivate(TObject *Sender);
	void __fastcall StatusBarResize(TObject *Sender);
public:
	static int FontSizePct;
	static int SplitterPosition;
	static bool ExecuteDefaultModule;
	static String DefaultModule;
private:	// User declarations
	static int FontInitialSize;
	amp::control_module *FControlModule;
	amp::performer		*FPerformer;
	String 				FCurrentFileName;
	MRList				FMRList;
//	TDateTime			FStartTime;
	void LoadTreeViewFromModel();

	void LoadOptions();
	void SaveOptions();

	void __fastcall AOpenMR(TObject *Sender);
	void ClearFileLogs(int nolderThanDays);
	void DoProcessRunExecute(TTreeNode* pnode);
	void SetFontSizeToUI();
	void SetSplitterSizeToUI();
    void add_ui_log_message(String s);
public:		// User declarations
	bool FStartAfterShow;
	__fastcall TFormMain(TComponent* Owner);
	__fastcall ~TFormMain();
	void __fastcall LoadFromFile(String sfname);
	void NestedMode();

	void ShowProgress(String s_caption, unsigned int pos_max, unsigned int pos_current, String info );
	void HideProgress();
	void set_control_performer(amp::control_module *cm, amp::performer		*cp);

};
//---------------------------------------------------------------------------
extern PACKAGE TFormMain *FormMain;
//---------------------------------------------------------------------------
#endif
