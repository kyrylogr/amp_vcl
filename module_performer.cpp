//---------------------------------------------------------------------------

#pragma hdrstop

#include "module_performer.h"
#include "control_module_struct.h"
#include "execution_thread.h"

#include "zip_proces.h"
#include "auto_control"
#include "process_helper.h"
#include "amp_utils.h"
#include "UnitMain.h"
#include <memory>
#include "System.Win.ComObj.hpp"
//---------------------------------------------------------------------------
#pragma comment(lib, "Shlwapi.lib")
#pragma package(smart_init)


/**	if  item is passed then it is an skip specific element break
*/

class AmpBreakException : public Exception {
	amp::run_item* m_item;
public:
	__fastcall AmpBreakException(amp::run_item* item = NULL): Exception("Operation cancelled by user"),
		m_item(item)
	{}
	amp::run_item* item()
	{
		return m_item;
	}
};


namespace amp {


String BreakSignalDescription(EBreakSignalType bs)
{
	switch (bs) {
	case breakStop:
		return "stop";
	case breakTimeout:
		return "timeout";
	case breakSkipElement:
		return "skip element";
	}
	return "unknown";
}

String RunStatusDescription(ERunStatus  rs)
{
	switch (rs) {
	case rsNone:
		return "";
	case rsOk:
		return "OK";
	case rsFailed:
		return "FAIL";
	case rsTimeout:
		return "TIMEOUT";
	case rsSkipped:
		return "SKIPPED";
	}
	return "unknown";
}


void log_entry::init(performer& p)
{
	 status = rsNone;
	 timeout = 0;
	 loop =0;
	 runtime=-1;
	 waittime=-1;
	 nest_level = p.nested_level();
	 runid = p.get_next_log_entry_counter() ;
	 tstart = Now();
}

int log_entry::calc_run_time()
{
	TDateTime tn = Now();
	return tn - tstart;
}

void log_entry::set_error(String serr)
{
	set_error(serr, calc_run_time(), 0);
}

void log_entry::set_completion_time()
{
	set_completion_time(calc_run_time(),0);
}

log_entry::log_entry(performer& p, run_item       &item)
{
	init(p);
	action = "starting";
	type = "run";
	name = item.name;
	file = item.filename;
	macro = item.macroname;
	timeout = item.timeout;
	type_detail = run_string_by_type(item.type);
}

log_entry::log_entry(performer& p, run_item       &item, send_run_element &send_item)
{
	init(p);
	action = "copying";
	type = "run";
	name = item.name;
	file = send_item.source;
	macro = send_item.target;
}

log_entry::log_entry(performer& p, model          &m)
{
	init(p);
	action = "starting";
	type = "model";
	name = m.name;
	timeout = m.timeout;
}

log_entry::log_entry(performer& p, control_module &cm, int nLoop)
{
	init(p);
	action = "starting";
	type = "control_module";
	name = cm.filename;
	timeout = 0;
	loop = nLoop;
}

log_entry::log_entry(performer& p, EBreakSignalType bs)
{
	init(p);
	action = "execution break - " + BreakSignalDescription(bs);
	type = "signal";
	timeout = 0;
}

void log_entry::set_error(String serr, int ntimetotal, int ntimewait)
{
	action = "result";
	status = rsFailed;
	runtime=ntimetotal;
	waittime=ntimewait;
	error = serr;
}

void log_entry::set_completion_time(int ntimetotal, int ntimewait)
{
	action = "result";
	status = rsOk;
	runtime=ntimetotal;
	waittime=ntimewait;
}

void log_entry::set_timeout()
{
	set_timeout(calc_run_time(), 0);
}

void log_entry::set_timeout(int ntimetotal, int ntimewait)
{
	action = "result";
	status = rsTimeout;
	runtime=ntimetotal;
	waittime=ntimewait;
}

void processing_element::init(performer* _performer)
{
	time_total_wait=0;
	current_wait_interval=0;
	time_start_run=Now();
	success=false,
	loop_no=0;
	loop_count=0;
	if (_performer) {
		form = _performer->Form;
	}
	p_performer=_performer;
	p_child_performer = NULL;
}

processing_element::processing_element(element_with_timeout* e, String _caption, TTreeNode* n, performer* _performer, int _default_timeout, EExecutionNotification s):
	node(n),
	timeout_element(e),
	initial_caption(_caption),
	status(s),
	default_timeout(_default_timeout),
	p_run_item(NULL)
{
	init(_performer);
}

processing_element::processing_element(run_item* e, String _caption, TTreeNode* n, performer* _performer, int _default_timeout, EExecutionNotification s):
	node(n),
	p_run_item(e),
	timeout_element(e),
	initial_caption(_caption),
	status(s),
	default_timeout(_default_timeout)
{
	init(_performer);
}

void 	   processing_element::start_wait(int nCurrentWait)
{
	current_wait_interval = nCurrentWait;
	status = enWait;
	time_start_wait = Now();
}

void 	   processing_element::end_wait()
{
	status = enRunning;
	time_total_wait += Now() - time_start_wait;
}

void	   processing_element::end_run(bool bOk)
{
	success = bOk;
	status = enOther;
}

bool	   processing_element::check_timeout(bool bTimeoutWithoutWait)
{
	ETimeoutOptions to = p_performer->get_root_performer().get_disable_timeouts();
	if (to == toAllDisable ||
		to == toHighLevelDisabled &&  (!p_run_item || p_run_item->type==rtXML)	) {
		return false;
	}
	int itimeout = timeout_element && timeout_element->timeout ?
					timeout_element->timeout : default_timeout;
	if (timeout_element && itimeout ) {
		int nDiffSecondsTotal = get_diff_seconds(time_start_run, Now());
		return
			(!bTimeoutWithoutWait && nDiffSecondsTotal > itimeout ||
				bTimeoutWithoutWait && (nDiffSecondsTotal-get_actual_wait_time())> itimeout);
	}
	return false;
}

int		   processing_element::get_actual_run_time(const TDateTime& tnow)
{
	return get_time_seconds (time_start_run - tnow);
}

int		   processing_element::get_actual_wait_time(const TDateTime& tnow)
{
	TDateTime timeWaitInProgress=0;
	if (status==enWait)
	   timeWaitInProgress = tnow - time_start_wait;
	return get_time_seconds (time_total_wait + timeWaitInProgress);
}

String	   processing_element::get_node_caption()
{
//	generate final caption
	String result = initial_caption;
	TDateTime tnow = Now();
	int nActualRunTime = get_diff_seconds(time_start_run, tnow);
	TDateTime timeWaitInProgress=0;
	if (status==enWait)
	   timeWaitInProgress = tnow - time_start_wait;
	int nActualWaitTime = get_actual_wait_time(tnow);
	int nCurrentWaitTime = get_time_seconds(timeWaitInProgress);
	int nL  = loop_no;
	int nLC = loop_count;

	if (nL)
		result+= " loop #" + IntToStr(nL) + " of " + IntToStr(nLC);
	result += " - ";
	int itimeout = timeout_element && timeout_element->timeout ?
					timeout_element->timeout : default_timeout;
	if (p_performer->get_disable_timeouts())
		itimeout = 0;
	switch (status) {
	case enWait:
		result += String::Format("Waiting [%d/%d], total runtime %d (run: %d, wait %d)",
				ARRAYOFCONST( ( nCurrentWaitTime, current_wait_interval,
								nActualRunTime, nActualRunTime-nActualWaitTime, nActualWaitTime) ) );
		break;
	case enRunning:
		result += String::Format("Running [%d/%d] sec (run %d, wait %d)",
				ARRAYOFCONST( ( nActualRunTime,
						itimeout, nActualRunTime-nActualWaitTime, nActualWaitTime) ) );
		break;
	case enOther:
		result += String::Format("%s, total runtime %d sec (run %d, wait %d)",
				ARRAYOFCONST( (success?"OK":"Fail", nActualRunTime, nActualRunTime-nActualWaitTime, nActualWaitTime) ) );
		break;
	}
	return result;
}

performer::performer(TFormMain* f, TTreeView* tv, TMemo* mlog):
	m_is_running(false),
	Form(f),TreeView(tv),MemoLog(mlog),
	m_nested_level(0), m_execution_thread(NULL),
	m_saved_exception(NULL),
	m_root_parent(NULL),m_stop_at_error(false),
	m_show_managed_apps(false),
	m_current_node_run(NULL),
	m_current_node_model(NULL),
	m_current_node_control(NULL),
	m_current_run(NULL),
	m_current_model(NULL),
	m_current_control(NULL),
	m_ole_ext_app_process_handle(NULL),
	m_current_timeout_element (NULL),
	m_breakable(true),
	m_disable_timeouts(toAllEnabled),
	m_parent(NULL),
	m_log_clear_period(0),
	m_write_csv_logs(true),
	m_leave_nested_windows_after_break(false),
	m_parent_processing_element(0)
{
	m_apps_exitcode_as_error = false;
	m_cmd_exitcode_as_error = false;

	m_detailed_log = false;
	m_p_flog = NULL;
	m_p_csv_flog = NULL;
	m_p_ferrlog = NULL;
	m_show_nested_control_in_window = true;
	m_break_signal_received = 0;
	FMtxProcessingElements = new System::Syncobjs::TMutex(false);
	FMtxLogs 			   = new System::Syncobjs::TMutex(false);
	FMtxProcessHandle 	   = new System::Syncobjs::TMutex(false);
	m_p_flog = NULL;
	m_p_ferrlog = NULL;
	m_log_file_timestamp = Now();
	m_timeout_without_wait = false;
	m_default_timeout = 1500;
	m_default_timeout_complex = 7200;
	m_log_entry_counter = 0;
}

bool performer::get_detailed_log()
{
	return get_root_performer().m_detailed_log;
}


performer::~performer()
{
	delete_log_files();
	delete FMtxProcessingElements;
	delete FMtxLogs;
	delete FMtxProcessHandle;

}

void performer::delete_log_files()
{
	MtxLock l(FMtxLogs);

	delete m_p_flog;
	m_p_flog = NULL;

	delete m_p_ferrlog;
	m_p_ferrlog = NULL;

	delete m_p_csv_flog;
	m_p_csv_flog = NULL;
}

void performer::open_log_files()
{
	AnsiString s = "amp_" + m_log_file_timestamp.FormatString("yyyymmdd_hhnnss");
	AnsiString sNameLog = s+".log";
	AnsiString sNameErrLog = s + "_err.log";
	AnsiString sNameCSVLog = s + "_csv.log";
	AnsiString sDirectory = m_log_directory;
	if (sDirectory.IsEmpty()) {
		sDirectory = ExtractFilePath(Application->ExeName);
	}
	if (!sDirectory.IsEmpty()) {
		sDirectory = IncludeTrailingBackslash(sDirectory);
	}
	if (!m_p_flog) {
		sNameLog = sDirectory + sNameLog;
		m_p_flog = new std_ext::ofstream(sNameLog.c_str(), std::ios_base::app);
	}
	if (!m_p_ferrlog) {
		sNameErrLog = sDirectory + sNameErrLog;
		m_p_ferrlog = new std::ofstream(sNameErrLog.c_str(), std::ios_base::app);
	}
	if (!m_p_csv_flog) {
		sNameCSVLog = sDirectory + sNameCSVLog;
		m_p_csv_flog = new std_ext::ofstream(sNameCSVLog.c_str(), std::ios_base::app);
	}
}

std_ext::ofstream & performer::get_csv_log_file()
{
	if (!m_p_csv_flog) {
		open_log_files();
	}
	return *m_p_csv_flog;
}

std_ext::ofstream & performer::get_log_file()
{
	if (!m_p_flog) {
		open_log_files();
	}
	return *m_p_flog;
}

std::ofstream & performer::get_err_log_file()
{
	if (!m_p_ferrlog) {
		open_log_files();
	}
	return *m_p_ferrlog;
}

void 		performer::add_processing_element(processing_element& e)
{
	MtxLock l(get_root_performer().FMtxProcessingElements);
	get_root_performer().elements_in_process.push_back(&e);
}

void		performer::remove_processing_element()
{
	MtxLock l(get_root_performer().FMtxProcessingElements);
	const processing_element* pe = get_root_performer().elements_in_process.back();
	if (break_signal_pointer()==breakTimeout &&
			pe->timeout_element == get_root_performer().m_current_timeout_element) {
		break_signal_pointer()==breakNone;
		get_root_performer().m_current_timeout_element = NULL;
	}

	get_root_performer().elements_in_process.pop_back();
}

void		performer::notify_processing_wait(int n)
{
	performer& proot= get_root_performer();
	MtxLock l(proot.FMtxProcessingElements);
	for (t_processing_elements::iterator i=proot.elements_in_process.begin(); i != proot.elements_in_process.end(); i++) {
		if (n)
			(*i)->start_wait(n);
		else
			(*i)->end_wait();
	}
}

int  performer::nested_level()
{
	return m_nested_level;
}

void performer::set_nested_level(int level)
{
	m_nested_level = level;
}

void performer::raise_if_breaked()
{
	switch (is_breaked()) {
		case breakStop:
			break_signal_pointer() = breakNone;
			throw AmpBreakException();
		case breakTimeout: {
			MtxLock l(get_root_performer().FMtxProcessingElements);
			performer& rootp =get_root_performer() ;

			for (amp::t_processing_elements::iterator i=rootp.elements_in_process.begin();
							i!=rootp.elements_in_process.end(); i++) {
				amp::processing_element * pe = (*i);
				if (pe->timeout_element && pe->timeout_element == rootp.m_current_timeout_element) {
				//	we should break and throw only when we have that timeout element in processing.
					break_signal_pointer() = breakNone;
					throw AmpTimeoutException(pe->timeout_element);
				}
			}
			break;
		}
		case breakSkipElement:
			break_signal_pointer() = breakNone;
			throw AmpBreakException(
					Form ? m_current_run :
					get_root_performer().m_current_run);
	}
}

int performer::is_breaked()
{
	return break_signal_pointer();
}

bool&   performer::is_breakable()
{
	return get_root_performer().m_breakable;
}

performer&  performer::get_root_performer()
{
	return m_root_parent ? *m_root_parent : *this;
}

int& 	performer::break_signal_pointer()
{
	return get_root_performer().m_break_signal_received;
}

void performer::set_parent(performer* _p)
{
//	Form = _p->Form;
	m_parent = _p;
	set_default_timeout(_p->get_default_timeout());
	set_default_timeout_complex(_p->get_default_timeout_complex());
//	MemoLog = _p->MemoLog;
	set_nested_level(_p->nested_level()+1) ;
	m_execution_thread = _p->m_execution_thread;
	if (_p->m_root_parent) {
		m_root_parent = _p->m_root_parent;
	} else
		m_root_parent = _p;
}

bool performer::stop_at_error()
{
	return get_root_performer().m_stop_at_error;
}

void performer::set_stop_at_error(bool v)
{
	m_stop_at_error = v;
}

String TimeDiffStr(TDateTime tfrom)
{
	TDateTime tnow = Now();
	tnow = tnow-tfrom;
	String sres = tnow.FormatString("hh:nn:ss");
	if (sres.Pos("00:")==1) {
		sres = sres.SubString(4, sres.Length());
	}
	return sres;
}

const int DefaultSleepPause = 200;

bool performer::conscious_wait(int iwait, EControlLevel clevel, bool bnotify_log, bool bnotify_node, EExecutionNotification wait_type)
{
	TDateTime dBegin = m_time_start_run;
	if (wait_type == enWait) {
		dBegin = Now();
	}
	if ( bnotify_log && wait_type == enWait) {
		String smessage = "waiting "+(AnsiString)IntToStr(iwait) + " seconds...";
		switch (clevel) {
		case levelRun:
			add_run_detail_log_message(smessage);
			break;
		case levelModel:
			add_model_log_message(smessage);
			break;
		}
	}
	try {
		notify_processing_wait(iwait);
		for (;; ) {
			raise_if_breaked();
			Sleep(DefaultSleepPause);
			int nTimeDiff = get_diff_seconds(dBegin, Now());
			if (nTimeDiff>=iwait)
				break;
		}
	} __finally {
		notify_processing_wait(0);
	}
	return true;//	пока таумауты не считаем.
}

/**	Add root directory to a file name, if file name does not specify absolute path (with drive or from root folder).
*/
String performer::CombineFileNameWithRootPath(String nameFile, bool bCombineOnlyIfExists)
{
	String srootdir;
	control_module * ctl = m_current_control;
	if (!ctl) {
		if (m_current_model){
			ctl = &m_current_model->parent();
		} else if (m_current_run) {
			ctl = &m_current_run->parent().parent();
		}
	}
	if (ctl) {
		srootdir = ctl->rootfolder;
		if (srootdir.IsEmpty()) {
			srootdir = ExtractFilePath(ctl->filename);
		}
	}
	if (PathIsRelative(nameFile.c_str()) && !srootdir.IsEmpty()) {
		String sFileCombined  = IncludeTrailingBackslash(srootdir) + nameFile;
		if (!bCombineOnlyIfExists || FileExists(sFileCombined) )
			nameFile = sFileCombined;
	}
	return nameFile;
}

int performer::eval_param(const String& sparam, int def)
{
	String nameParam, nameFile;
	int nPosAt = sparam.Pos("@");
	if (nPosAt) {
		nameParam = sparam.SubString(1, nPosAt-1);
		nameFile  = sparam.SubString(nPosAt+1, sparam.Length()-nPosAt);
	} else {
		nameParam = sparam;
	}
	if (nameFile.IsEmpty()) {
		throw Exception("trying to read logic param without file name");
	}
	String sResult;
	{
		try{
			std::ifstream sfile;
			sfile.exceptions ( ifstream::failbit | ifstream::badbit );
			if (m_current_control) {
				nameFile = CombineFileNameWithRootPath(nameFile);
			}
			sfile.open( nameFile.c_str(), std::ios_base::in );

			TStringList* sl = new TStringList();			
			std::unique_ptr<TStringList>  lfree(sl);
			if (!nameParam.IsEmpty()) {
				while (!sfile.eof()) {
					std::string s;
					sfile >> s;
					if (!s.empty() && s.find("=")!=std::string::npos
							&& s.find("#")!=0) {
						sl->Add(s.c_str());
					}
				}
				sResult = sl->Values[nameParam];
			} else {
				std::string s;
				sfile >> s;
				sResult = s.c_str();
			}
		} catch (const ifstream::failure& e) {
		}
	}
	int res = def;
	if (!sResult.IsEmpty()) {
		if (!sResult.CompareIC("FALSE")) {
			res = 0;
		} else if (!sResult.CompareIC("TRUE")) {
			res = 1;
		} else {
			res = StrToInt(sResult);
		}
	}

	return res;
}



void performer::common_run_process(run_item& item)
{
	m_current_run = &item;
	m_current_node_run = node_for_run(item);

	String filterMessage;
	log_entry le(*this, item);
	m_time_start_run = Now();
	bool
		willRun = item.is_allowed_by_filter(m_time_start_run, filterMessage);
	if (willRun && !item.enabled_logic.IsEmpty()) {
		willRun = eval_param(item.enabled_logic, 1);
		filterMessage = "enabled logic - " + item.enabled_logic;
	}
	if (willRun) {
		processing_element pe(&item, item.name,m_current_node_run, this,
			item.type == amp::rtXML ? get_root_performer().get_default_timeout_complex() : get_root_performer().get_default_timeout()  );
		add_processing_element(pe);
		bool bOk= false;
		String sException;
		try {
			try {
				String logMsg = "starting run '" +item.name + "'";
				if (item.timeout) {
					logMsg+= " , timeout = "+IntToStr(item.timeout);
				} else if (pe.default_timeout) {
					logMsg+= " , default timeout = "+IntToStr(pe.default_timeout);
					le.timeout = pe.default_timeout;
				}
				add_run_log_message(logMsg);
				add_structured_log_message(le, logMsg);
			//	processing wait attribute
				if (item.wait) {
					conscious_wait(item.wait, levelRun, true, true);
//					add_structured_log_message(le, "waiting");
				}

				switch (item.type) {
				case amp::rtMDB:
					run_process_MDB(item);
					break;
				case amp::rtShell:
					run_process_Shell(item);
					break;
				case amp::rtExcel:
					run_process_Excel(item);
					break;
				case amp::rtCompact:
					run_process_Compact(item);
					break;
				case amp::rtXML:
					run_process_XML(item, pe);
					break;
				case amp::rtCopy:
					run_process_Copy(item);
					break;
				}
				bOk=true;
			} catch (Exception &e) {
				sException = e.Message;
				if (is_breaked()==breakTimeout) {
					sException = "process timeout";
					MtxLock l(get_root_performer().FMtxProcessingElements);
					break_signal_pointer() = breakNone;
					performer& rootp = get_root_performer();
					element_with_timeout* te = rootp.m_current_timeout_element;
					if (te) {
//                        sException += " " + te.
						rootp.m_current_timeout_element = NULL;
						if (te == static_cast<element_with_timeout*>(&item)) {
							if ( stop_at_error() )
								throw AmpTimeoutException(te);
						} else {
							for (t_processing_elements::iterator ipe=rootp.elements_in_process.begin();
									ipe!=rootp.elements_in_process.end(); ipe++)
								if ((*ipe)->timeout_element==te)
									throw AmpTimeoutException(te);
						}
					}
				}
				throw;
			}
		} __finally {
			add_run_extra_log_message("common_run_process final code");
			pe.end_run(bOk);
			add_run_extra_log_message("common_run_process pe.end_run() ok");
			String s_time = TimeDiffStr(m_time_start_run);
			if (bOk) {
				le.set_completion_time( pe.get_actual_run_time(), pe.get_actual_wait_time());
			} else {
				le.set_error(sException, pe.get_actual_run_time(), pe.get_actual_wait_time());
			}
			if (!bOk && !sException.IsEmpty()) {
				add_run_detail_log_message( "!!! Exception: " + sException , true) ;
			}
			add_run_extra_log_message("common_run_process about to display run status");
			String sresult = bOk ? "OK" : "with error";
			String sstatus_message = String("run '") + item.name + "' was completed " + sresult + " in " + s_time;
			add_run_log_message( sstatus_message, !bOk ) ;
			add_structured_log_message( le, sstatus_message);
			notify_node(m_current_node_run, false, pe.get_node_caption());
			remove_processing_element();
		}
	} else {
		String skipmsg = "execution of run '" + item.name + " ' was skipped: "+ filterMessage;
		add_run_log_message(skipmsg);
		le.status = rsSkipped;
		add_structured_log_message(le, skipmsg);
		notify_node(m_current_node_run, false, item.name, "skipped: " + filterMessage);
	}
}

void performer::prepare_managed_app()
{
	MtxLock l(get_root_performer().FMtxProcessingElements);
	get_root_performer().m_ole_ext_app_error_message = "";
}

void performer::run_process_MDB(run_item& item)
{
	if (item.filename.IsEmpty() || item.macroname.IsEmpty()) {
		add_run_detail_log_message("skipping element because some parameters are empty");
		return;
	}
	String sfname = CombineFileNameWithRootPath(item.filename);
	add_run_detail_log_message("executing access db file: " + sfname + ", macro: " + item.macroname);

	Variant vVarApp, vVarDoc, vRes1, vRes2, vVarActiveWindow, vCMD;

	process_helper ph(_T("OMain"));
	prepare_managed_app();

	vVarApp = Variant::CreateObject("Access.Application");
	{
		value_guard<HANDLE> vgh(get_root_performer().m_ole_ext_app_process_handle, ph.end_watch(), NULL);
		bool bWasOpen = false;
		bool bHasHandle = get_root_performer().m_ole_ext_app_process_handle!=NULL;
		try{
			vVarApp.OlePropertySet("Visible",show_managed_apps() );
			vVarApp.OlePropertySet("UserControl",false);
			vVarApp.OleProcedure("OpenCurrentDatabase",WideString(sfname));
			bWasOpen = true;
			bool bLegacyCode = true;//true ;
			try {
				if (bLegacyCode) {
					vCMD = vVarApp.OlePropertyGet("DoCmd");
					vCMD.OleProcedure("SetWarnings", false);
//					add_run_extra_log_message("about to run macro");
					vCMD.OleProcedure("RunMacro",WideString(item.macroname));
//					add_run_extra_log_message("macro run ok");
				} else
					vVarApp.OleProcedure("Run",WideString(item.macroname));
			} catch (EOleException &e) {
				MtxLock l(get_root_performer().FMtxProcessingElements);
				String svbaErr = get_root_performer().m_ole_ext_app_error_message;
				get_root_performer().m_ole_ext_app_error_message = "";
				if (!svbaErr.IsEmpty()) {
					throw Exception(svbaErr);
				}
				else
					throw e;
			}
			vVarApp.OleProcedure("CloseCurrentDatabase");
			bWasOpen = false;
			vVarApp.OleProcedure("Quit");
		}
		catch(Exception& e)
		{
			MtxLock l(get_root_performer().FMtxProcessHandle);
			if (bHasHandle && !get_root_performer().m_ole_ext_app_process_handle) {
				//	означает что был хендл и процесс прибили.
				throw Exception("Process killed");
			} else {
				try {
					if (bWasOpen) {
						vVarApp.OleProcedure("CloseCurrentDatabase");
					}
					vVarApp.OleProcedure("Quit");
					get_root_performer().m_ole_ext_app_process_handle = NULL;
				} catch (Exception& eInner) {
					l.UnLock();
					add_run_detail_log_message("cleanup exception: " + eInner.Message, true);
				}
				throw ;
			}
		}
	}
}

void performer::run_process_Shell(run_item& item)
{
	if (item.filename.IsEmpty()) {
		add_run_detail_log_message("filename empty, skipped");
	} else {
		add_run_detail_log_message("executing shell command " + item.filename + "...");

		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
		String sfname = CombineFileNameWithRootPath(item.filename);
		String sCmdLine = sfname;//item.filename;

		String sApp;
		const char* cBatExt= ".BAT";
		unsigned int batPos =sCmdLine.UpperCase().Pos(cBatExt);
		bool bIsCMD = false ;
		bool	bFileExists = true;
		if (batPos!=0) {
/*	todo:
*			there is a possible problem if batch file has parameters but filename contains spaces it will not be handled properly
*/
			if (batPos==sCmdLine.Length() - strlen(cBatExt)+1) {
				bFileExists = FileExists(sCmdLine);
				sCmdLine = "\"" + sCmdLine + "\"";
			}
			sCmdLine = "cmd.exe /c " +sCmdLine;
			bIsCMD = true;
		}

		TDateTime time_created_process = m_time_start_run;
		DWORD                 dwCreationFlags = NORMAL_PRIORITY_CLASS;
		if (!show_managed_apps()) {
			dwCreationFlags = dwCreationFlags  | CREATE_NO_WINDOW;
		}

		BOOL bres = CreateProcess(sApp.IsEmpty()?NULL:sApp.c_str(), sCmdLine.c_str(), NULL, NULL, TRUE, dwCreationFlags, NULL,NULL,  &si, &pi);
		if (bres==FALSE)
		{
			throw Exception("Failed to create process. Error code: " + IntToStr((int)GetLastError()) + ".");
		}
		if (pi.hProcess==0)
		{
			throw Exception("Handle for created process is NULL. Error code: " + IntToStr((int)GetLastError()) + ".");
		}

		try {
			unsigned int reswait;
			bool bTimeout = false;
			try {
				for (; ; ) {
					raise_if_breaked();
					reswait = WaitForSingleObject(pi.hProcess, DefaultSleepPause);
					if (reswait==WAIT_OBJECT_0) {
						break;
					}
					if (reswait==WAIT_FAILED) {
						throw Exception("Waiting for the process failed. Error code: " + IntToStr((int)GetLastError()) + ".");
					}
				}
			} catch (...) {
				if (!bTimeout || item.kill_on_timeout
						) {
					TerminateChildProcesses(pi.dwProcessId);
					bool bterminateres =
						::TerminateProcess(pi.hProcess, -1);
					add_run_log_message(String ("  killing process: ") + (bterminateres ? "OK": "failed") , true);
				}
				throw;
			}
			DWORD exitCode =0;
			if (reswait==WAIT_OBJECT_0) {
				bool bGetExitCodeStatus = GetExitCodeProcess(pi.hProcess, &exitCode);
				if (bGetExitCodeStatus) {
					add_run_detail_log_message("process ended, exit code = " + IntToStr((int)exitCode) );
					if (exitCode!=0) {
						if (get_cmd_exitcode_as_error() && bIsCMD || get_apps_exitcode_as_error() && !bIsCMD) {
							String sMessage = "Process exit code is non zero: "+ IntToStr((int)exitCode) + ".";
							if (!bFileExists)
								sMessage += " file was not found: " +sfname;
							throw Exception (sMessage);
						}
					}
				}
				else
					add_run_detail_log_message("process ended but getting result code failed. error code: " + IntToStr((int)GetLastError()) );

			}
		} __finally {
			if (pi.hProcess)
				CloseHandle(pi.hProcess);
		}
	}
}

bool performer::progress(ZIP_SIZE_TYPE pos_max, ZIP_SIZE_TYPE pos_current, String info )
{
//	do visualization
	if (Form && m_current_run) {
		if (m_execution_thread) {
			m_execution_thread->show_progress(Form, "Performing: " + m_current_run->name, pos_max, pos_current, "file: " + info);
		}
	}
	return !is_breaked();
}

void performer::copy_zip_operation(String source, String target, int izlevel, bool btimestamp)
{
	try {
		zip_operation(source, target, izlevel, btimestamp, NULL, this);
	}__finally {
		if (m_execution_thread)
			m_execution_thread->hide_progress(Form);
	}
}

void performer::run_process_Copy(run_item& item)
{
	for (int i=0; i < item.send_run_element_count(); i++) {
		send_run_element& send_item = item.get_send_run_element(i);
		log_entry le(*this, item, send_item);
		if (send_item.source.IsEmpty() || send_item.target.IsEmpty()) {
			le.status = rsSkipped;
			String sMsg = "send element skipped as source and/or target are missing. element #"+IntToStr(i);
			add_run_detail_log_message(sMsg);
			add_structured_log_message(le, sMsg);
			continue;
		}
		for (int n=0; n <= send_item.retries; n++) {
			le.loop = n;
			int izlevel = 0;
			if (!send_item.ziplevel.IsEmpty()) {
				izlevel = StrToInt(send_item.ziplevel);
			}
			if (n && send_item.lockwait) {
				conscious_wait(send_item.lockwait, levelRun, true, true);
			}
			String sMsg = String::Format("performing send element (%d) '%s' to '%s', ziplevel = %d, try #%d" ,
					ARRAYOFCONST((i, send_item.source , send_item.target, izlevel, n)) );
			add_run_detail_log_message( sMsg );
			add_structured_log_message(le, sMsg);

			bool bResultSuccessful = false;

			if (izlevel == 0 && !send_item.timestamp) {
				//	see the documentation on SHFileOperation
				int nLenSource = send_item.source.Length();
				String sSource = send_item.source + "  ";
				sSource[nLenSource+2] = '\0';
				sSource[nLenSource+1]   = '\0';
				int iStartFilePos = 1;
				for ( int z=1; z <= nLenSource; z++) {
					bool bIsSemicolon = sSource[z]==';';
					if (bIsSemicolon || z==nLenSource ) {
						String s = sSource.SubString(iStartFilePos, bIsSemicolon ? z- iStartFilePos : z- iStartFilePos + 1);
						if (!FileExists(s) &&! DirectoryExists(s)) {
							throw EFOpenError("Source file or directory is not found: %s", ARRAYOFCONST( ( s) ) );
						}
						if (bIsSemicolon) {
							sSource[z]= '\0';
						}
						iStartFilePos = z+1;
					}
				}
				if (!DirectoryExists(send_item.target)) {
						String sTargetDir = ExtractFilePath(send_item.target);
						if (!sTargetDir.IsEmpty() && !DirectoryExists(sTargetDir)) {
							throw EFOpenError("Target directory does not exist: %s", ARRAYOFCONST( (send_item.target) ));
						}
				}

				SHFILEOPSTRUCT od;
				ZeroMemory(&od, sizeof(SHFILEOPSTRUCT));

				od.hwnd = Form?Form->Handle : NULL;
				od.wFunc = FO_COPY;
				od.pFrom = sSource.c_str();
				od.pTo =  send_item.target.c_str();
				od.hNameMappings = NULL;
				od.lpszProgressTitle = NULL;
				od.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR |  FOF_NOERRORUI  ;
				value_guard<bool> bc(is_breakable(), false,true);
				int nres = SHFileOperation(&od);
				if (!nres) {
					if (od.fAnyOperationsAborted) {
						throw AmpBreakException();
					} else {
						bResultSuccessful = true;
					}
				} else {
					if (od.fAnyOperationsAborted) {
						throw AmpBreakException();
					} else {
						String smessage = "copy error code: " + IntToStr(nres);
						if (n == send_item.retries)
							throw Exception(smessage);
						else {
							le.set_error(smessage);
							add_run_detail_log_message(smessage);
							add_structured_log_message(le, smessage);
						}
					}
				}
			} else { //	zip type
				try {
					copy_zip_operation(send_item.source, send_item.target, izlevel, send_item.timestamp);
					bResultSuccessful = true;
				} catch (EFOpenError&e){
					throw;
				} catch (AmpBreakException&) {
					throw;
				} catch (AmpTimeoutException&) {
					throw;
				} catch (Exception &e) {
					if (n == send_item.retries) {
						throw;
					} else {
						String smessage = "copy zip error: " + e.Message;
						le.set_error(e.Message);
						add_run_detail_log_message( smessage, true);
						add_structured_log_message(le, smessage);
					}
				}
			}
			if (bResultSuccessful) {
				le.set_completion_time();
				String sMsg = "successful";
				add_run_detail_log_message(sMsg);
				add_structured_log_message(le, sMsg);
				break;
			}
		}
	}
}

void performer::run_process_Excel(run_item& item)
{
	if (item.filename.IsEmpty() || item.macroname.IsEmpty()) {
		add_run_detail_log_message("skipping element because some parameters are empty");
		return;
	}
	String sfname = CombineFileNameWithRootPath(item.filename);
	add_run_detail_log_message("executing excel file: " + sfname + ", macro: " + item.macroname);

	Variant vVarApp, vVarBooks,vVarDoc, vRes1, vRes2;


	process_helper ph(_T("XLMAIN"));
	prepare_managed_app();
	vVarApp = Variant::CreateObject("Excel.Application");
	{
		value_guard<HANDLE> vgh(get_root_performer().m_ole_ext_app_process_handle, ph.end_watch(), NULL);

		bool bWasOpen = false;
		bool bHasHandle = get_root_performer().m_ole_ext_app_process_handle!=NULL;
//		vRes1 = vVarApp.OlePropertyGet("Value");
		vVarApp.OlePropertySet("DisplayAlerts",false);
		try{
			vVarApp.OlePropertySet("Visible",show_managed_apps() );

			vVarBooks = vVarApp.OlePropertyGet("Workbooks");
			vVarBooks.OleProcedure("Open",WideString(sfname));
			vVarDoc = vVarBooks.OlePropertyGet("Item",1);
			bWasOpen = true;
			try {
				vVarApp.OleProcedure("Run",WideString(item.macroname));
			} catch (EOleException &e) {
				MtxLock l(get_root_performer().FMtxProcessingElements);
				String svbaErr = get_root_performer().m_ole_ext_app_error_message;
				get_root_performer().m_ole_ext_app_error_message = "";
				if (!svbaErr.IsEmpty())
					throw Exception(svbaErr);
				else
					throw e;
			}
			vVarDoc.OleProcedure("Save");
			vVarBooks.OleProcedure("Close");
			bWasOpen = false;
			vVarApp.OleProcedure("Quit");
		}
		catch(Exception& e)
		{
			MtxLock l(get_root_performer().FMtxProcessHandle);
			if (bHasHandle && !get_root_performer().m_ole_ext_app_process_handle) {
				//	означает что был хендл и процесс прибили.
				throw Exception("Process killed");
			} else {
				try {
					if  (bWasOpen)
						vVarBooks.OleProcedure("Close");
					vVarApp.OleProcedure("Quit");
					get_root_performer().m_ole_ext_app_process_handle = NULL;
				} catch (Exception& eInner) {
					l.UnLock();
					add_run_detail_log_message("cleanup exception: " + eInner.Message, true);
				}
				throw ;
			}
		}
	}
}

void performer::run_process_Compact(run_item& item)
{
	if (item.filename.IsEmpty() ) {
		add_run_detail_log_message("skipping element because filename parameter is empty");
		return;
	}
	String sfname = CombineFileNameWithRootPath(item.filename);
	String  tmpFile = sfname + ".compacted";

	add_run_detail_log_message("compacting access db file: " + sfname + " with temp file (" + tmpFile + ")" );

	Variant vVarApp, vVarDoc, vRes1, vRes2;

	process_helper ph(_T("OMain"));
	prepare_managed_app();
	vVarApp = Variant::CreateObject("Access.Application");
	{
		value_guard<HANDLE> vgh(get_root_performer().m_ole_ext_app_process_handle, ph.end_watch(), NULL);
		bool bHasHandle = get_root_performer().m_ole_ext_app_process_handle!=NULL;
		try{
			DeleteFile(tmpFile);
			vVarApp.OlePropertySet("Visible",show_managed_apps() );

			for (int n = 0; n <= item.retries; n++) {
				if (n &&item.lockwait)
					conscious_wait(item.lockwait, levelRun, true, true);
				try {
					vVarApp.OleProcedure("CompactRepair", WideString(sfname), WideString(tmpFile) );
					break;
				} catch (Exception & e) {
					if (n==item.retries) {
						throw;
					} else {
						add_run_detail_log_message("try #"+IntToStr(n) + " failed with exception: " + e.Message, true );
					}
				}
			}
			vVarApp.OleProcedure("Quit");
		}
		catch(Exception& e)
		{
			MtxLock l(get_root_performer().FMtxProcessHandle);
			if (bHasHandle && !get_root_performer().m_ole_ext_app_process_handle) {
				//	означает что был хендл и процесс прибили.
				throw Exception("Process killed");
			} else {
				try {
					vVarApp.OleProcedure("Quit");
					get_root_performer().m_ole_ext_app_process_handle = NULL;
				} catch (Exception& eInner) {
					l.UnLock();
					add_run_detail_log_message("cleanup exception: " + eInner.Message, true);
				}
				throw ;
			}
		}
		BOOL bRes = MoveFileEx(tmpFile.c_str(), sfname.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
		if (!bRes) {
			DeleteFile(tmpFile);
			throw Exception("Failed to rename compacted file to original. Error: " + IntToStr((int)GetLastError()));
		}
	}
}

void performer::run_process_XML(run_item& item, processing_element& pe)
{
	if (item.filename.IsEmpty()) {
		throw Exception("Atrribute filename is empty.");
	}
	String sfname = CombineFileNameWithRootPath(item.filename);
	add_run_detail_log_message("executing XML " + sfname);

	control_module * p_controlNested = NULL;
	performer      * p_performerNested = NULL;

	std::unique_ptr<control_module> xfree_module;
	std::unique_ptr<performer> 		xfree_performer;


	TFormMain* FNested = NULL;
	if ( get_root_performer().get_show_nested_control_in_window() ) {
		if (m_execution_thread)
			FNested = m_execution_thread->CreateForm(item.filename);
		else {
			FNested = new TFormMain (NULL);
			FNested->LoadFromFile(sfname);
			FNested->NestedMode();
		}
		p_controlNested = FNested->FControlModule;
		p_performerNested = FNested->FPerformer;
	} else {
		p_controlNested = new control_module();
		p_performerNested = new performer();

		p_controlNested->load_from_file(sfname);
		p_performerNested->Form = Form;

		xfree_module = std::unique_ptr<control_module>(p_controlNested);
		xfree_performer = std::unique_ptr<performer>(p_performerNested);
	}
	p_performerNested->m_parent_processing_element = &pe;

	p_performerNested->set_parent(this);
	pe.p_child_performer = p_performerNested;
	bool bWasBreaked = false;
	try{
		try{
			p_performerNested->do_process(*p_controlNested);
		} catch(AmpBreakException &e) {
			if (!e.item())
				bWasBreaked = true;
			throw ;
		}
	}
	__finally {
		//	so far we will close nested windows only if nested XML was completed successfully
		if (FNested && !(get_root_performer().m_leave_nested_windows_after_break && bWasBreaked	) ) {
			add_run_extra_log_message("run_process_XML before final clear");
			add_run_extra_log_message("run_process_XML in lock FMtxProcessingElements");
			if (m_execution_thread) {
				m_execution_thread->DeleteForm(FNested);
				add_run_extra_log_message("run_process_XML form deleted");
			} else {
				add_run_extra_log_message("BUG: not execution_thread");
			}
		}
	}
}

void performer::process(run_item& item)
{
	raise_if_breaked();
	try {
		common_run_process(item);
	}
	catch (	AmpBreakException & be) {
		if (be.item()==&item) {
			add_run_log_message("element skipped manually " + item.name );
		} else
			throw;
	} catch ( AmpTimeoutException & te) {
		if (te.timeout_element == static_cast<element_with_timeout*>(&item)) {
			if (stop_at_error()) {
				throw;
			}
		} else throw;
	} catch (   Exception & e ) {
		//	there can be situations when process is terminated because it was timeouted or breaked
		//	 in this case the initial exception could be of a different type
		if (is_breaked()) {
			switch (is_breaked()) {
			case breakStop:
				throw AmpBreakException();
			case breakSkipElement:
				break_signal_pointer() = breakNone;
				if (get_root_performer().m_current_run == &item)
					add_run_log_message("element skipped manually " + item.name, true );
				else
					throw AmpBreakException(get_root_performer().m_current_run);
			}
		} else
			if (stop_at_error())
				throw;
	}
}

void performer::process(model& m)
{
	m_current_model = &m;
	m_current_node_model = node_for_model(m);
	String filterMessage;
	m_time_start_model = Now();
	bool willRun = m.is_allowed_by_filter(m_time_start_model, filterMessage);
	if (willRun && !m.enabled_logic.IsEmpty()) {
		willRun = eval_param(m.enabled_logic, 1);
		filterMessage = "enabled logic - " + m.enabled_logic;
	}

	log_entry le(*this, m);
	if (willRun) {
		bool bOk= false;
		processing_element pe(&m, m.name,m_current_node_model, this);
		add_processing_element(pe);
		try{
			try {
				String logMsg = "starting model '" + m.name + "'";
				if (m.timeout) {
					logMsg+= " ";
					logMsg+= ", timeout = "+IntToStr(m.timeout);
				}
				add_model_log_message(logMsg);
				add_structured_log_message(le, logMsg);
				notify_node(m_current_node_model, true, m.name, "");
				for (int i=0; i < m.run_count(); i++) {
					run_item& ri = m.get_run(i);
					process (ri);
				}
				bOk=true;
			} __finally {
				pe.end_run(bOk);
				remove_processing_element();
				String s_time = TimeDiffStr(m_time_start_model);
				String sresult = bOk ? "OK" : "with error";
				String sMsg = "model '" + m.name + "' was completed " + sresult + " in " + s_time;
				add_model_log_message( sMsg ) ;
				if (bOk)
					le.set_completion_time();
				else
					le.set_error("");
				add_structured_log_message(le, sMsg);
				notify_node(m_current_node_model, false, pe.get_node_caption());
			}
		}  catch ( AmpTimeoutException & te) {
			if (te.timeout_element == static_cast<element_with_timeout*>(&m)) {
				if (stop_at_error()) {
					throw;
				}
			} else throw;
		}
	}	else {
		le.status = rsSkipped;
		String smsg = "execution of model '" + m.name + "' was skipped: " + filterMessage;
		add_model_log_message(smsg);
		add_structured_log_message(le, smsg);
		notify_node(m_current_node_model, false, m.name, "skipped: " + filterMessage);
	}
}

void performer::do_process(run_item& item)
{
	mark_new_story_in_log();
	m_break_signal_received = false;
	m_is_running = true;
	try{
		process(item);
	} __finally {
		m_is_running = false;
	}
}

void performer::do_process(model& m)
{
	mark_new_story_in_log();
	m_break_signal_received = false;
	m_is_running = true;
	try{
		process(m);
	} __finally {
		m_is_running = false;
	}
}

void performer::mark_new_story_in_log()
{
	add_log_message("");
	add_log_message("= = = = = = = = = = = = =");
	add_log_message("");
	m_log_entry_counter = 0;
}

void performer::do_process(control_module& module)
{
	if (is_running()) {
		throw Exception("process is already running");
	}
	if (nested_level()==0) {
		mark_new_story_in_log();
	}

	m_current_control = &module;
	m_break_signal_received = false;
	BoolController bc (m_is_running, false);

	log_entry le(*this, module,0);

	if (!module.enabled) {
		String sMsg = "control module is not enabled, skipping";
		add_log_message(sMsg);
		add_structured_log_message(le, sMsg);
		le.status = rsSkipped;
		return;
	}
	if (!module.enabled_logic.IsEmpty() && !eval_param(module.enabled_logic, 1)) {
		String sMsg = "control module is not enabled with logic, skipping. " + module.enabled_logic;
		le.status = rsSkipped;
		add_log_message(sMsg);
		add_structured_log_message(le, sMsg);
		return;
	}

	m_current_node_control = node_for_control();

	int nLoop = module.loop;
	if (!module.loop_logic.IsEmpty()) {
		nLoop = eval_param(module.loop_logic, nLoop);
	}

	processing_element pe((element_with_timeout*)NULL, module.def_node_caption(),m_current_node_control, this);
	add_processing_element(pe);

//	pe.loop_no = p_performerNested->loop_no();
//	pe.loop_count = p_controlNested->loop;

	bool bOk = false;
	try{
		m_loop_no = 1;
		pe.loop_count = nLoop;
		if (m_parent_processing_element) {
			m_parent_processing_element->loop_count = nLoop;
		}
		for (; m_loop_no <= nLoop; m_loop_no++) {
			le.loop = m_loop_no;
			m_time_start_control = Now();
			pe.loop_no = m_loop_no;
			if (m_parent_processing_element) {
				m_parent_processing_element->loop_no = m_loop_no;
			}
			AnsiString smessage = "starting control_module '" + module.filename+ "', loop #" + IntToStr(m_loop_no)+" of "+ nLoop;
			add_log_message( smessage);
			add_structured_log_message(le, smessage);
			for (int i=0; i < module.model_count(); i++) {
				model & m_current_model = module.get_model(i);
				process(m_current_model);
			}
			String sMsg = "control_module was completed in " + TimeDiffStr(m_time_start_control);
			add_log_message(sMsg) ;
			le.set_completion_time();
			add_structured_log_message(le, sMsg);
			if (m_loop_no == nLoop)
				break;
			Sleep(100);
			//	if there are a lot of loops or infinite loops and skipped alements because of time filters
			//	then there can be high processor load
		}
		add_run_extra_log_message("do_process(control_module) cycle ok");
		bOk = true;
	} __finally {
		pe.end_run(bOk);
		add_run_extra_log_message("do_process(control_module) end_run ok");
		remove_processing_element();
		add_run_extra_log_message("do_process(control_module) remove_processing_element ok");
		notify_node(m_current_node_control, false, pe.get_node_caption());
		add_run_extra_log_message("do_process(control_module) notify_node ok");
		m_is_running = false;
	}
}

void performer::break_signal(EBreakSignalType bs, amp::element_with_timeout* elTimeout)
{
	get_root_performer().m_current_timeout_element = elTimeout;
	if (break_signal_pointer() < bs) {
	// we only escalate break signal (if stop was sent and then timeout occurs then there
	// should be stop signal processed.
		break_signal_pointer() = bs;
		String sdesc = BreakSignalDescription(bs);
		add_log_message("break signal received: " + sdesc, true);
		{
			MtxLock l(get_root_performer().FMtxProcessHandle);
			HANDLE h = get_root_performer().m_ole_ext_app_process_handle;
			if (h) {
				get_root_performer().m_ole_ext_app_process_handle = NULL;
				TerminateProcess(h,-1);
			}
		}
	}
}

bool performer::check_managed_app_vba_error()
{
	performer& rootp = get_root_performer();
	if (rootp.m_ole_ext_app_process_handle && rootp.m_ole_ext_app_error_message.IsEmpty()) {
		rootp.m_ole_ext_app_error_message = get_managed_app_vba_error(get_root_performer().m_ole_ext_app_process_handle, true);
		if (!rootp.m_ole_ext_app_error_message.IsEmpty())
			return true;
	}
	return false;
}

bool performer::is_running()
{
	return m_is_running;
}

void performer::add_structured_log_message(log_entry& ple, String s)
{
	if (m_parent) {
		m_parent->add_structured_log_message(ple, s);
	}
	String sSeparator = ";";
	if (!nested_level() ) {
		TDateTime d = Now();
		String message =  d.FormatString("dd.mm.yy" + sSeparator + "hh:nn:ss" );

		message += sSeparator;
		message += ple.runid;

		message += sSeparator;
		message += ple.nest_level;

		message += sSeparator;
		message += ple.action;

		message += sSeparator;
		message += ple.type;

		message += sSeparator;
		message += ple.type_detail;



		message += sSeparator;
		message += ple.name;

		message += sSeparator;
		message += ple.file;

		message += sSeparator;
		message += ple.macro;

		message += sSeparator;
		message += IntToStr(ple.loop);

		message += sSeparator;
		if (ple.timeout)
			message += IntToStr(ple.timeout);

		message += sSeparator;
		message += RunStatusDescription(ple.status);

		message += sSeparator;
		if (ple.runtime >= 0) {
			message += IntToStr(ple.runtime);
		}
		message += sSeparator;
		if (ple.waittime >= 0) {
			message += IntToStr(ple.waittime);
		}

		message += sSeparator;
		message += ple.error;

		message += sSeparator;
		message += s;

		if (!nested_level()) {
			MtxLock l(FMtxLogs);
			std_ext::ofstream &of = get_csv_log_file();
			of << AnsiString(message).c_str() <<std::endl;
			of.commit_if_time_exceeded();
		}
	}
}

void performer::add_log_message(String s, bool bErr)
{
	if (nested_level()) {
		s = String::StringOfChar(' ',/*nested_level()* */ 8) + s;
	}

	if (MemoLog || !nested_level() ) {
		TDateTime d = Now();
		String message =  d.FormatString("dd.mm.yy hh:nn:ss") + " | ";
		message += s;

		if (!nested_level()) {
			MtxLock l(FMtxLogs);
			std_ext::ofstream &of = get_log_file();
			of << AnsiString(message).c_str() <<std::endl;
			of.commit_if_time_exceeded();

			if (bErr)
				get_err_log_file() << AnsiString(message).c_str() <<std::endl;
		}

		if (MemoLog) {
			if (m_execution_thread)
				m_execution_thread->notify_log(message, MemoLog);
			else
				MemoLog->Lines->Add(message);
		}
	}
	if (m_parent) {
		m_parent->add_log_message(s, bErr);
	}
}

/**	returns the node at level 0 with position = pos
*/

TTreeNode* GetTopNodeByIndex(TTreeView* tv, int pos)
{
	TTreeNode* pNode= tv->Items->GetFirstNode();
	for (; pNode && pos>0; pos--) {
		pNode = pNode->getNextSibling();
	}
	return pNode;
}

TTreeNode* performer::node_for_run(run_item& item)
{
	if (TreeView && TreeView->Items->Count) {
		TTreeNode* pModelNode = GetTopNodeByIndex(TreeView,item.parent().index()+1);
		return pModelNode->Item[item.index()];
	}
	return NULL;
}

TTreeNode* performer::node_for_model(model& item)
{
	if (TreeView && TreeView->Items->Count) {
		TTreeNode* pModelNode = GetTopNodeByIndex(TreeView,item.index()+1);
		return pModelNode;
	}
	return NULL;
}

TTreeNode* performer::node_for_control()
{
	if (TreeView && TreeView->Items->Count) {
		return TreeView->Items->Item[0];
	}
	return NULL;
}

void performer::set_running()
{
	m_is_running = true;
}

void performer::set_execution_thread( PerformerThread *_thread)
{
	m_execution_thread = _thread;
}

TTreeView* performer::get_treeview()
{
	return TreeView;
}

TMemo*  performer::get_memo_log()
{
	return MemoLog;
}

void performer::add_run_detail_log_message(String s, bool bErr)
{
	add_run_log_message("  " + s, bErr);
}

void performer::add_run_log_message(String s, bool bErr)
{
	add_log_message("    " + s, bErr);
}

void performer::add_run_extra_log_message(String s)
{
	if (get_detailed_log()) {
		add_run_log_message(s, false);
	}
}

void performer::add_model_log_message(String s, bool bErr)
{
	add_log_message("  " + s, bErr);
}

void performer::notify_node( TTreeNode* pnode, bool bRunning, const String& snodetext, const String& smessage)
{
	if (pnode) {
		String text_new = snodetext  ;
		if (!smessage.IsEmpty())
		  text_new += " - " + smessage;

		if (m_execution_thread)
			m_execution_thread->notify_node(pnode,text_new);
		else
			pnode->Text = text_new;
	}
}

bool 		performer::show_managed_apps()
{
	return get_root_performer().m_show_managed_apps;
}

void 		performer::show_managed_apps(bool v)
{
	m_show_managed_apps = v;
}

void 		performer::set_logfile_timestamp(TDateTime d)
{
	m_log_file_timestamp = d;
}

bool		performer::get_show_nested_control_in_window()
{
	return m_show_nested_control_in_window;
}

void		performer::set_show_nested_control_in_window(bool v)
{
	m_show_nested_control_in_window = v;
}

bool		performer::get_timeout_without_wait()
{
	return get_root_performer().m_timeout_without_wait;
}

void		performer::set_timeout_without_wait(bool v)
{
	get_root_performer().m_timeout_without_wait = v;
}

bool		performer::get_apps_exitcode_as_error()
{
	return get_root_performer().m_apps_exitcode_as_error;
}

void		performer::set_apps_exitcode_as_error(bool v)
{
	get_root_performer().m_apps_exitcode_as_error = v;
}

bool		performer::get_cmd_exitcode_as_error()
{
	return get_root_performer().m_cmd_exitcode_as_error;
}

void		performer::set_cmd_exitcode_as_error(bool v)
{
	get_root_performer().m_cmd_exitcode_as_error = v;
}

int			performer::get_default_timeout()
{
	return m_default_timeout;
}

void		performer::set_default_timeout(int v)
{
	m_default_timeout = v;
}

int			performer::get_default_timeout_complex()
{
	return m_default_timeout_complex;
}

void		performer::set_default_timeout_complex(int v)
{
	m_default_timeout_complex = v;
}

performer*  performer::get_parent()
{
	return m_parent;
}

String		performer::get_log_directory()
{
	return m_log_directory;
}

void		performer::set_log_directory(String v)
{
	if (m_log_directory!=v) {
		m_log_directory=v;
		delete_log_files();
		//	so far there is no extra protection on file structures.
		//	we hope that the logging methods will be used only within synchronize.
	}
}

int			performer::get_log_clear_period()
{
	return m_log_clear_period;
}

void		performer::set_log_clear_period(int v)
{
	m_log_clear_period = v;
}


ETimeoutOptions		performer::get_disable_timeouts()
{
	return m_disable_timeouts;
}

void		performer::set_disable_timeouts(ETimeoutOptions v)
{
	m_disable_timeouts = v;
}


bool		performer::get_write_csv_logs()
{
	return m_write_csv_logs;
}

void		performer::set_write_csv_logs(bool v)
{
	m_write_csv_logs = v;
}

int 		performer::get_next_log_entry_counter()
{
	return ++get_root_performer().m_log_entry_counter;
}

int			performer::loop_no()
{
	return m_loop_no;
}

bool		performer::get_leave_nested_windows_after_break()
{
	return m_leave_nested_windows_after_break;
}

void		performer::set_leave_nested_windows_after_break(bool v)
{
	m_leave_nested_windows_after_break = v;
}

void		performer::set_detailed_log(bool b)
{
	m_detailed_log = b;
}

}
