//---------------------------------------------------------------------------

#ifndef module_performerH
#define module_performerH
//---------------------------------------------------------------------------


#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.ComCtrls.hpp>
#include <System.SyncObjs.hpp>

#include <vector>
#include "amp_ofstream.h"
#include "zip_proces.h"

/**	Header file for performer and supporting class.
*	performer is central class that holds the execution logic of controlmodule.
*/


class PerformerThread;
class TFormMain;

namespace amp {

class element_with_timeout;
}


/**	Timeout exception
*/

class AmpTimeoutException: public Exception {
public:
	amp::element_with_timeout* timeout_element;
public:
	__fastcall AmpTimeoutException(amp::element_with_timeout* pe, String e=""): Exception( e.IsEmpty()? String("Process timeout") : e),
		timeout_element(pe)
	{}
};


namespace amp {


class control_module;
class run_item;
class model;
class performer;
class send_run_element;

enum EBreakSignalType { breakNone, breakSkipElement, breakTimeout, breakStop };

enum ERunStatus { rsNone, rsOk, rsFailed, rsTimeout, rsSkipped };

enum ETimeoutOptions {toAllEnabled, toHighLevelDisabled, toAllDisable };

/**	log entry for structured log.
*/

class log_entry {
	void init(performer& p);
	int calc_run_time();
	TDateTime tstart;
public:
	int runid;
	String action;
	String name;
	int nest_level;
	int loop;
	int runtime;
	int waittime;
	int timeout;
	String type;
	String type_detail;
	String file;
	String macro;
	ERunStatus status;
	String error;
	log_entry(performer& p, run_item       &item);
	log_entry(performer& p, run_item       &item, send_run_element &send_item);
	log_entry(performer& p, model          &m);

	log_entry(performer& p, control_module &cm, int nLoop);
	log_entry(performer& p, EBreakSignalType bs);
	void set_error(String serr, int ntimetotal, int ntimewait);
	void set_timeout();
	void set_timeout(int ntimetotal, int ntimewait);
	void set_completion_time(int ntimetotal, int ntimewait);
	void set_error(String serr);
	void set_completion_time();

};

enum EControlLevel {  levelControl, levelModel, levelRun };

enum EExecutionNotification {  enWait, enRunning, enOther };


/**		processing_element
*	Class defines proceessing infmation while element at any level that
*	is currently processing (module, model, run, nested modules, etc).
*	It holds timing and additional visual info and is used by main form
*	for displaying current processing status and checking timeouts.
*/

class processing_element {
	void init(performer* _performer);
public:
	TTreeNode* node;
	element_with_timeout* timeout_element;
	TForm*     form;
	performer* p_performer;
	performer* p_child_performer;
	run_item * p_run_item;
	int 	   default_timeout;
	int		   loop_no;
	int        loop_count;
	TDateTime  time_start_run;
	TDateTime  time_start_wait;
	TDateTime  time_total_wait;
	int        current_wait_interval;
	bool	   success;
	EExecutionNotification status;
	String 	   initial_caption;
	void 	   start_wait(int nwait);
	void 	   end_wait();
	void	   end_run(bool bOk);
	String 	   get_node_caption();
	bool	   check_timeout(bool bTimeoutWithoutWait);
	int		   get_actual_wait_time(const TDateTime& tnow = Now()) ;
	int		   get_actual_run_time(const TDateTime& tnow = Now()) ;
	processing_element(element_with_timeout* e, String _caption,
						TTreeNode* n, performer* _performer, int _default_timeout=0, EExecutionNotification s = enRunning);
	processing_element(run_item* e, String _caption,
						TTreeNode* n, performer* _performer, int _default_timeout=0, EExecutionNotification s = enRunning);

};

typedef std::vector<processing_element*> t_processing_elements;

/**  	Class for executing control models.
*   Contains all logic for execution of control, models and run elements, including nesting.
*		Execution can be breaked by call to break_signal().
*		performer controls execution of only one controlmodule, for nested controlmodules child
*	performer classes are created (that have reference to parent performer).
*		some methods provide the access not to the performer attributes but to the root performer
*	attributes.
*/
class performer: protected progress_interface {
friend class PerformerThread;
friend class processing_element;
	int 	m_break_signal_received;
	bool 	m_is_running;
	bool	m_stop_at_error;
	bool 	m_show_managed_apps;
	bool 	m_timeout_without_wait;
	bool 	m_leave_nested_windows_after_break;
	bool	m_detailed_log;
	ETimeoutOptions 	m_disable_timeouts;
	bool 	m_write_csv_logs;
	bool  m_apps_exitcode_as_error;
	bool  m_cmd_exitcode_as_error;

	int  	m_nested_level;
	int		m_default_timeout;
	int 	m_default_timeout_complex;
	int 	m_log_clear_period;
	int     m_log_entry_counter;
	int		m_loop_no;
	HANDLE  m_ole_ext_app_process_handle;
	bool    m_show_nested_control_in_window;
	Exception		*m_saved_exception;
	TDateTime		m_log_file_timestamp;
	String 	m_ole_ext_app_error_message;
	String	m_log_directory;

	TFormMain* Form;
	TTreeView* TreeView;
	TMemo* 	MemoLog;

	TDateTime m_time_start_control;
	TDateTime m_time_start_model;
	TDateTime m_time_start_run;
	PerformerThread *m_execution_thread;
	performer		*m_root_parent;
	performer		*m_parent;

	TTreeNode* m_current_node_control;
	TTreeNode* m_current_node_model;
	TTreeNode* m_current_node_run;

	run_item*	   	m_run_to_skip;
	run_item*	   	m_current_run;
	model*	   		m_current_model;
	control_module*   m_current_control;
	/**	the element that was timeouted.
	*/
	element_with_timeout* m_current_timeout_element;
	bool		m_breakable;
	int& 		break_signal_pointer();
	std_ext::ofstream *m_p_flog;
	std_ext::ofstream *m_p_csv_flog;
	std::ofstream *m_p_ferrlog;
	std_ext::ofstream & get_log_file();
	std_ext::ofstream & get_csv_log_file();
	std::ofstream & get_err_log_file();
	void open_log_files();
	/**	Add root directory to a file name, if file name does not specify absolute path (with drive or from root folder).
	*/
	String CombineFileNameWithRootPath(String nameFile, bool bCombineOnlyIfExists = true);
	/**	Closes and reopens stream if it time limit has passed since last "commit".
	* currently time difference is 120 seconds.
	*/
	void commit_log_by_time(std::ofstream & fstr, TDateTime& dt);
protected:
	int	 is_breaked();
	void raise_if_breaked();
	void add_run_detail_log_message(String s, bool bErr = false);
	void add_run_log_message(String s, bool bErr = false);
	void add_run_extra_log_message(String s);
	void add_model_log_message(String s, bool bErr = false);
	void add_log_message(String s, bool bErr = false);
	void add_structured_log_message(log_entry& ple, String s = "");
	void add_file_log_message(String s, bool bErr = false);
	void process(run_item& item);
	void process(model& m);
	bool conscious_wait(int iwait, EControlLevel clevel, bool bnotify_log, bool bnotify_node, EExecutionNotification wait_type = enWait);
	TTreeNode* node_for_run(run_item& item);
	TTreeNode* node_for_model(model& item);
	TTreeNode* node_for_control();

	void notify_node( TTreeNode* pnode, bool bRunning, const String& snodetext, const String& smessage="");
//	void notify_nodes_from_level(EControlLevel clevel, bool bFlag, const String& smessage);
	void common_run_process(run_item& item);
	void run_process_MDB(run_item& item);
	void run_process_Shell(run_item& item);
	void run_process_Copy(run_item& item);
	void run_process_Excel(run_item& item);
	void run_process_Compact(run_item& item);
	void run_process_XML(run_item& item, processing_element& pe);
	void copy_zip_operation(String source, String target, int izlevel, bool btimestamp);
	void set_parent(performer* _p);

	void 		set_execution_thread(	PerformerThread *_thread);

	void 		set_nested_level(int level);
	TTreeView* 	get_treeview();

	void 		add_processing_element(processing_element& e);
	void		remove_processing_element();
	void		notify_processing_wait(int n);
	void		mark_new_story_in_log();
	int 		eval_param(const String& sparam, int def);
/**	Implements progress callback of zip operations
*/
	bool progress(ZIP_SIZE_TYPE pos_max, ZIP_SIZE_TYPE pos_current, String info );
	void prepare_managed_app();
	processing_element* m_parent_processing_element;
public:
	System::Syncobjs::TMutex	*FMtxProcessingElements;
	System::Syncobjs::TMutex	*FMtxProcessHandle;
//	System::Syncobjs::TMutex	*FMtxNestedPerformer;
	System::Syncobjs::TMutex	*FMtxLogs;
	t_processing_elements elements_in_process;

//	void unbind_from_parent();
	bool 		check_managed_app_vba_error();
	void 		delete_log_files();
	int  		nested_level();
public:
	performer(TFormMain* f=NULL, TTreeView* tv=NULL, TMemo* mlog=NULL);
	~performer();

	void do_process(control_module& module);
	void do_process(run_item& item);
	void do_process(model& m);

	bool 		is_running();
	void 		set_running();

	Exception* 	get_exception();
	void       	clear_exception();

	TMemo*  	get_memo_log();

/**	available properties
*/
	bool 		stop_at_error();
	void	   	set_stop_at_error(bool v);

	bool 		show_managed_apps();
	void 		show_managed_apps(bool v);
	bool&   	is_breakable();
	void 		break_signal(EBreakSignalType bs, amp::element_with_timeout* elTimeout=NULL);
	void 		set_logfile_timestamp(TDateTime d);
	bool		get_show_nested_control_in_window();
	void		set_show_nested_control_in_window(bool v);
	bool		get_timeout_without_wait();
	void		set_timeout_without_wait(bool v);
	int			get_default_timeout();
	void		set_default_timeout(int v);
	int			get_default_timeout_complex();
	void		set_default_timeout_complex(int v);

	bool		get_apps_exitcode_as_error();
	void		set_apps_exitcode_as_error(bool v);

	bool		get_cmd_exitcode_as_error();
	void		set_cmd_exitcode_as_error(bool v);

	int			get_log_clear_period();
	void		set_log_clear_period(int v);


	String		get_log_directory();
	void		set_log_directory(String v);

	ETimeoutOptions		get_disable_timeouts();
	void				set_disable_timeouts(ETimeoutOptions v);

	bool		get_write_csv_logs();
	void		set_write_csv_logs(bool v);


	bool		get_leave_nested_windows_after_break();
	void		set_leave_nested_windows_after_break(bool v);
/**	returns root performer or this.
*/
	performer&  get_root_performer();

	performer*  get_parent();

	int 		get_next_log_entry_counter();

	int			loop_no();
	void		set_detailed_log(bool b);
	bool		get_detailed_log();
};

}
#endif
