//---------------------------------------------------------------------------

#pragma hdrstop

#include "execution_thread.h"
#include "UnitMain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)


__fastcall PerformerThread::PerformerThread(amp::performer& p):TThread(true),
	Performer(p),saved_exception(NULL)
{
	run_for_execute = NULL;
	model_for_execute = NULL;
	control_module_for_execute = NULL;
	notified_node = NULL;
	notify_memo = NULL;
	control_module_for_set = NULL;
	performer_for_set = NULL;

	Form = NULL;
	FreeOnTerminate = true;
	Performer.set_execution_thread(this);
}

__fastcall PerformerThread::~PerformerThread()
{
}

void __fastcall PerformerThread::internal_notify_node()
{
	if (notified_node )
		notified_node->Text = notify_message;
}

void __fastcall PerformerThread::internal_notify_log()
{
	if (notify_memo) {
		if (notify_memo->Lines->Count>10000)
			notify_memo->Lines->Delete(0);
		notify_memo->Lines->Add(notify_message.c_str());
	}
}

void __fastcall PerformerThread::show_exception()
{
	if (saved_exception) {
		Application->ShowException(saved_exception);
	}
}

void __fastcall PerformerThread::Execute()
{
	try {
		try {
			HRESULT cores = CoInitializeEx(0, COINIT_MULTITHREADED);
			if (cores!=S_OK) {
				throw Exception("COM initialization error.");
			}

			if (run_for_execute) {
				Performer.do_process(*run_for_execute);
			}
			if (model_for_execute) {
				Performer.do_process(*model_for_execute);
			}
			if (control_module_for_execute) {
				Performer.do_process(*control_module_for_execute);
			}
		} __finally {
			CoUninitialize();
		}
	} catch (Exception& e) {
		saved_exception = &e;
		Synchronize(show_exception);
	}
	run_for_execute = NULL;
	model_for_execute = NULL;
	control_module_for_execute = NULL;
}

void PerformerThread::execute(amp::run_item& r)
{
	if (Performer.is_running())
		throw Exception("performer is already running");
//	Performer.set_running();
	run_for_execute = &r;
	Start();
}

void PerformerThread::execute(amp::model& m)
{
	if (Performer.is_running())
		throw Exception("performer is already running");
//	Performer.set_running();
	model_for_execute = &m;
	Start();
}

void PerformerThread::execute(amp::control_module& m)
{
	if (Performer.is_running())
		throw Exception("performer is already running");
//	Performer.set_running();
	control_module_for_execute = &m;
	Start();
}

void PerformerThread::notify_node(TTreeNode* node, String smessage)
{
	notified_node = node;
	notify_message = smessage;
	Synchronize(internal_notify_node);
}

void PerformerThread::notify_log(String smessage, TMemo* memo)
{
	notify_memo = memo;
	notify_message = smessage;
	Synchronize(internal_notify_log);
}

TFormMain* PerformerThread::CreateForm(const String & sControlFile, bool bShow )
{
	s_file = sControlFile;
	do_show_form = bShow;
	Synchronize(internal_create_form);
	return Form;
//
}

void 	   PerformerThread::DeleteForm(TFormMain* f)
{
	Form = f;
	Synchronize(internal_delete_form);
}

void __fastcall PerformerThread::internal_create_form()
{
	Form = new TFormMain(NULL);
	Form->LoadFromFile(s_file);
	Form->NestedMode();
	if (do_show_form) {
		Form->Visible = true;
	}
}

void __fastcall PerformerThread::internal_delete_form()
{
	Form->Visible = false;
	delete Form;
	Form = NULL;
}

void __fastcall PerformerThread::internal_show_progress()
{
	if (Form) {
		Form->ShowProgress(notify_message, progress_max, progress_current, s_info);
	}
}

void __fastcall PerformerThread::internal_hide_progress()
{
	Form->HideProgress();
}

void PerformerThread::show_progress(TFormMain* f, String s_caption, ZIP_SIZE_TYPE pos_max, ZIP_SIZE_TYPE pos_current, String info )
{
	progress_max = pos_max;
	progress_current = pos_current;
	s_info = info;
	notify_message = s_caption;
	Form = f;
	Synchronize(internal_show_progress);
}

void PerformerThread::hide_progress(TFormMain* f)
{
	Form = f;
	Synchronize(internal_hide_progress);
}

void PerformerThread::SetFormControlPerformer(TFormMain* f, amp::control_module *cm, amp::performer		*cp)
{
	Form = f;
	control_module_for_set = cm;
	performer_for_set = cp;
}

void __fastcall PerformerThread::internal_set_form_control_performer()
{
	Form->set_control_performer(control_module_for_set, performer_for_set);
}
