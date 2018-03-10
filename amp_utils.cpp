//---------------------------------------------------------------------------

#pragma hdrstop

#include "amp_utils.h"
#include <Registry.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)


namespace amp {

MtxLock::MtxLock(System::Syncobjs::TMutex* _m)
{
	m = _m;
	m->Acquire();
}

MtxLock::~MtxLock()
{
	if (m)
		m->Release();
}

void MtxLock::UnLock()
{
	if (m)
		m->Release();
	m = NULL;
}


String SApplicationName = "AMP";

String SCompanyName = "Ansergy";

String GetAppRegistryKey()
{
	return "Software\\"+SCompanyName+"\\"+SApplicationName;
}

void SaveRegistryIntValue(const String& sName, int value)
{
	TRegistry *r = new TRegistry();
	try {
		r->RootKey = HKEY_CURRENT_USER;
		String SKEY_APP = GetAppRegistryKey();
		if (!r->OpenKey(SKEY_APP, true)) {
			delete r;
			throw Exception(String("failed to open/create registry key: ")+ SKEY_APP);
		}
		try {
			r->WriteInteger(sName, value);
		} catch (ERegistryException& e) {
		}
		r->CloseKey();
	}__finally {
		delete r;
	}

}

void SaveRegistryStrValue(const String& sName, const String& value)
{
	TRegistry *r = new TRegistry();
	try {
		r->RootKey = HKEY_CURRENT_USER;
		String SKEY_APP = GetAppRegistryKey();
		if (!r->OpenKey(SKEY_APP, true)) {
			delete r;
			throw Exception(String("failed to open/create registry key: ")+ SKEY_APP);
		}
		try {
			r->WriteString(sName, value);
		} catch (ERegistryException& e) {
		}
		r->CloseKey();

	}__finally {
		delete r;
	}
}

String  LoadRegistryStrValue(const String& sName, const String& value)
{
	String result;
	TRegistry *r = new TRegistry();
	try {
		r->RootKey = HKEY_CURRENT_USER;
		String SKEY_APP = GetAppRegistryKey();
		if (!r->OpenKey(SKEY_APP, true)) {
			throw Exception(String("failed to open/create registry key: ")+ SKEY_APP);
		}
		try {
			if (r->ValueExists(sName))
				result = r->ReadString(sName);
			else
				result = value;
		} catch (ERegistryException& e) {
			result = value;
		}
		r->CloseKey();
	}__finally {
		delete r;
	}
	return result;
}

int  LoadRegistryIntValue(const String& sName, int def_value)
{
	int result = def_value;
	TRegistry *r = new TRegistry();
	try {
		r->RootKey = HKEY_CURRENT_USER;
		String SKEY_APP = GetAppRegistryKey();
		if (!r->OpenKey(SKEY_APP, true)) {
			throw Exception(String("failed to open/create registry key: ")+ SKEY_APP);
		}
		try {
			result = r->ReadInteger(sName);
		} catch (ERegistryException& e) {
		}
		r->CloseKey();

	}__finally {
		delete r;
	}

	return result;
}


int get_time_seconds(const TDateTime &tdiff)
{
	return ((double)tdiff)*24*60*60;
}

int get_diff_seconds(const TDateTime &tstart, const TDateTime& tend){
	return get_time_seconds( tend - tstart);
}

TDateTime & increment_seconds(TDateTime &tstart, int nsecs)
{
	tstart += 1.0* nsecs/(24*60*60);
	return tstart;
}

}