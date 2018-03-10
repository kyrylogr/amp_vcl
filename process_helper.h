//---------------------------------------------------------------------------

#ifndef process_helperH
#define process_helperH
//---------------------------------------------------------------------------

#include <classes.hpp>
#include <set>
#include <vector>



/**		Helper utilities for watching the spawned process execution.
*
*/



namespace amp {


/** 	Base class for window enumerators.
*	provide abstraction of winapin routines.
*/

class base_window_enumerator {
protected:
	static BOOL CALLBACK EnumCallbackStatic(HWND   hwnd, LPARAM lParam);
	virtual BOOL EnumCallback(HWND   hwnd)=0;
	void EnumerateWindows();
	void EnumerateChildWindows(HWND        hWndParent);
};


/** 	process_helper.
*	Class finds spawned process by main window class name.
*	It is a process that created a new window between prepare_watch and endwatch calls.
*/

class process_helper:  base_window_enumerator {
protected:
	std::set<HWND>  m_handles;
	LPCTSTR 		m_className;
	HANDLE          m_process_handle;
	bool			m_end_watch_stage;
	bool			m_own_handle;

	BOOL EnumCallback(HWND   hwnd);
public:
	process_helper(LPCTSTR className=NULL);
	virtual ~process_helper();
	void 	prepare_watch(LPCTSTR className);
	HANDLE 	end_watch(bool bOwnHandle= true);
};

typedef std::vector<HWND> t_v_window_handles;

/**		window_enumerator.
*   Helps to find top_level windows or child windows by possible parameters:
*			 process handle, caption, classname, parent handle.
*		FindXXX methods return first window that matches conditions.
*	All window handles are stored in m_handles member.
*/

class window_enumerator: base_window_enumerator {
	LPCTSTR 				m_className;
	LPCTSTR 				m_caption;
	DWORD					m_process_id ;
	HWND   					m_hwndParent;
	BOOL 					EnumCallback(HWND   hwnd);
public:
	t_v_window_handles  	m_handles;
public:
	window_enumerator();
	HWND find_top_level_window(HANDLE   _process_handle,
					LPCTSTR _className,
					LPCTSTR _caption);
	HWND find_child_window(HWND _hwndParent,
					LPCTSTR _className,
					LPCTSTR _caption);

};

/**	Finds VBA error message window, extract error and close the window.
*/
String get_managed_app_vba_error(HANDLE phandle,  bool bClose=true);

/** Checks if a process by name exists
*/

bool ProcessExists(String processName);

/** Kill all the processes by name
*/
void TerminateChildProcesses(unsigned long threadId);

/** Kill all the processes by name
*/
bool ATerminateProcess(String processName, bool bKillChild = false);

/**	Execute process with command line.
*/
bool LaunchProcess(String sCmd, bool bwait = false );

/**	Execute process with command line.
*/
bool ExecuteProcessWithWait(String sCmd);

}
#endif
