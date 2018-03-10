//---------------------------------------------------------------------------

#pragma hdrstop

#include "process_helper.h"
#include "System.StrUtils.hpp"
#include <tlhelp32.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)



namespace amp {

BOOL CALLBACK base_window_enumerator::EnumCallbackStatic(HWND   hwnd, LPARAM lParam)
{
	base_window_enumerator* ph = reinterpret_cast<base_window_enumerator*>(lParam);
	return ph->EnumCallback(hwnd);
}

void base_window_enumerator::EnumerateWindows()
{
	EnumWindows(EnumCallbackStatic, (LPARAM)(this));
}

void base_window_enumerator::EnumerateChildWindows(HWND        hWndParent)
{
	EnumChildWindows(hWndParent, EnumCallbackStatic, (LPARAM)(this));
}

#define NAMECLASS_NAME_MAX_LEN 200

BOOL process_helper::EnumCallback(HWND   hwnd)
{
	TCHAR wndClassName[NAMECLASS_NAME_MAX_LEN];
	if (GetClassName (hwnd, wndClassName, NAMECLASS_NAME_MAX_LEN) &&
					_tcscmp(wndClassName, m_className)==0) {
		if (m_end_watch_stage) {
			if (m_handles.empty() || m_handles.find(hwnd)==m_handles.end()) {
				DWORD tid;
				if (GetWindowThreadProcessId(hwnd, &tid)) {
					m_process_handle = OpenProcess(PROCESS_TERMINATE, false, tid);
				}
				return false;
			}
		} else
			m_handles.insert(hwnd);
	}
	return true;
}

process_helper::process_helper(LPCTSTR className)
{
	m_className = NULL;
	m_process_handle = NULL;
	m_own_handle = false;
	if (className)
		prepare_watch(className);
}

process_helper::~process_helper()
{
	if (m_process_handle && m_own_handle) {
		CloseHandle(m_process_handle);
	}
}

void 	process_helper::prepare_watch(LPCTSTR className)
{
	m_handles.clear();
	m_className = className;
	m_end_watch_stage = false;
	EnumerateWindows();
}

HANDLE 	process_helper::end_watch(bool bOwnHandle)
{
	m_own_handle = bOwnHandle;
	m_end_watch_stage = true;
	m_process_handle = NULL;
	EnumerateWindows();
	return m_process_handle;
}

window_enumerator::window_enumerator()
{
	m_process_id = 0;
	m_className = NULL;
	m_caption = NULL;
}

HWND window_enumerator::find_top_level_window(HANDLE   _process_handle,
					LPCTSTR _className,
					LPCTSTR _caption)
{
	m_handles.clear();
	if (_process_handle)
		m_process_id= GetProcessId(_process_handle);
	m_className = _className;
	m_caption = _caption;
	EnumerateWindows();
	if (!m_handles.empty()) {
		return m_handles[0];
	} else return NULL;
}

HWND window_enumerator::find_child_window(HWND _hwndParent,
					LPCTSTR _className,
					LPCTSTR _caption)
{
	m_handles.clear();
	m_process_id = 0;
	m_className = _className;
	m_caption = _caption;
	EnumerateChildWindows( _hwndParent);
	if (!m_handles.empty()) {
		return m_handles[0];
	} else return NULL;
}

#define WINTEXT_MAXLEN 8000

BOOL window_enumerator::EnumCallback(HWND   hwnd)
{
	if (m_process_id  ) {
		DWORD pid;
		if (!GetWindowThreadProcessId(hwnd, &pid) || pid!=m_process_id) {
			return true;
		}
	}
	if (m_className && _tcslen(m_className)) {
		TCHAR wndClassName[NAMECLASS_NAME_MAX_LEN];
		if (!GetClassName (hwnd, wndClassName, NAMECLASS_NAME_MAX_LEN) ||
					_tcscmp(wndClassName, m_className))
			return true;
	}
	if (m_caption && _tcslen(m_caption)) {
		TCHAR wndCaption[WINTEXT_MAXLEN];
		if (!GetWindowText (hwnd, wndCaption, WINTEXT_MAXLEN) ||
					_tcscmp(wndCaption, m_caption))
			return true;
	}
	m_handles.push_back(hwnd);
	return true;
}

String get_managed_app_vba_error(HANDLE phandle, bool bClose)
{
	String result;
	window_enumerator we;
	HWND hdlg = we.find_top_level_window(phandle, _T("#32770"), _T("Microsoft Visual Basic"));
	if (hdlg) {
		HWND hbtn = we.find_child_window(hdlg, _T("Button"), _T("&End"));
		if (!hbtn) {
			we.find_child_window(hdlg, _T("Button"), NULL);
			if (we.m_handles.size()>2) {
				hbtn = we.m_handles[1];
			}
		}
		if (hbtn) {
			HWND hstatic = we.find_child_window(hdlg, _T("Static"), NULL);
			String sErr;
			if (hstatic) {
				TCHAR wndCaption[WINTEXT_MAXLEN];
				if (GetWindowText (hstatic, wndCaption, WINTEXT_MAXLEN) ) {
					result = wndCaption;
					result = "(VBA) " + ReplaceStr(result, "\n\n", "\t");
				}
			}
			if (bClose)
				SendMessage(hbtn, BM_CLICK, 0, 0);
		}
	}
	return result;
}

bool ProcessExists(String processName)
{
	bool bResult = false;

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry) == TRUE)
	{
		BOOL bContinue= TRUE;
		for (; bContinue==TRUE; bContinue = Process32Next(snapshot, &entry) ) {
			if (_tcsicmp(entry.szExeFile, processName.c_str()) == 0)	{
				bResult = true;
				break;
			}
		}
	}

	CloseHandle(snapshot);
	return bResult;
}

void TerminateChildProcesses(unsigned long threadId)
{
	PROCESSENTRY32 pe;

	memset(&pe, 0, sizeof(PROCESSENTRY32));
	pe.dwSize = sizeof(PROCESSENTRY32);

	HANDLE hSnap = :: CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (::Process32First(hSnap, &pe))
	{
		BOOL bContinue = TRUE;

		// kill child processes
		while (bContinue)
		{
			// only kill child processes
			if (pe.th32ParentProcessID == threadId)
			{
				HANDLE hChildProc = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);

				if (hChildProc)
				{
					TerminateChildProcesses(pe.th32ProcessID);
					::TerminateProcess(hChildProc, 1);
					::CloseHandle(hChildProc);
				}
			}

			bContinue = ::Process32Next(hSnap, &pe);
		}

	}
}

bool ATerminateProcess(String processName, bool bKillChild)
{
	bool bResult = false;

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry) == TRUE)
	{
		BOOL bContinue= TRUE;
		for (; bContinue==TRUE; bContinue = Process32Next(snapshot, &entry)) {
			if (_tcsicmp(entry.szExeFile, processName.c_str()) == 0) {
				if (bKillChild) {
					TerminateChildProcesses(entry.th32ProcessID);
				}
				DWORD dwDesiredAccess = PROCESS_TERMINATE;
				BOOL  bInheritHandle  = FALSE;
				HANDLE hProcess = OpenProcess(dwDesiredAccess, bInheritHandle, entry.th32ProcessID);
				if (hProcess == NULL)
					continue;
				UINT uExitCode;
				bResult = ::TerminateProcess(hProcess, uExitCode) || bResult;
				CloseHandle(hProcess);
			}
		}
	}

	CloseHandle(snapshot);
	return bResult;
}

bool LaunchProcess(String sCmd, bool bwait)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	BOOL bres = CreateProcess(NULL, sCmd.c_str(), NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS,NULL,NULL,  &si, &pi);
	if (bres==FALSE)
	{
		return false;
	}
	if (pi.hProcess==0)
	{
		return false;
	}

	if (bwait) {
		WaitForSingleObject(pi.hProcess, INFINITE);
	}

	if (pi.hProcess)
		CloseHandle(pi.hProcess);
	return true;
}

}


