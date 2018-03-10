//---------------------------------------------------------------------------

#ifndef execution_threadH
#define execution_threadH
//---------------------------------------------------------------------------
#include <classes.hpp>

#include "control_module_struct.h"
#include "module_performer.h"

class TFormMain;

/**		Execution thread for performer.
*	Provides necessary public routines for safe UI updating via synchronize calls.
*/

class PerformerThread : public TThread {
	amp::performer& Performer;
protected:
	void __fastcall Execute();
	amp::run_item* 				run_for_execute;
	amp::model*    				model_for_execute;
	amp::control_module*		control_module_for_execute;
	amp::control_module*		control_module_for_set;
	amp::performer*				performer_for_set;
	TTreeNode*					notified_node;
	Exception*					saved_exception;
	String						notify_message;
	String						s_file;
	bool						do_show_form;
	TFormMain*					Form;
	TMemo*						notify_memo;
	int							progress_max;
	int							progress_current;
	String					    s_info;

	void __fastcall internal_notify_node();
	void __fastcall internal_notify_log();
	void __fastcall show_exception();
	void __fastcall internal_create_form();
	void __fastcall internal_delete_form();
	void __fastcall internal_show_progress();
	void __fastcall internal_hide_progress();
	void __fastcall internal_set_form_control_performer();
public:
   __fastcall PerformerThread(amp::performer& p);
   __fastcall ~PerformerThread();
	void execute(amp::run_item& r);
	void execute(amp::model& m);
	void execute(amp::control_module& m);

	void notify_node(TTreeNode* node, String smessage);
	void notify_log(String smessage, TMemo* memo);
	void show_progress(TFormMain* f , String s_caption, ZIP_SIZE_TYPE pos_max, ZIP_SIZE_TYPE pos_current, String info );
	void hide_progress(TFormMain* f);

	TFormMain* CreateForm(const String & sControlFile, bool bShow = true);
	void 	   		DeleteForm(TFormMain* f);
	void       SetFormControlPerformer(TFormMain* f, amp::control_module *cm, amp::performer		*cp);
};


#endif
