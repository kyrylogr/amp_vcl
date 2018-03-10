//---------------------------------------------------------------------------

#ifndef amp_utilsH
#define amp_utilsH
//---------------------------------------------------------------------------
#include <classes.hpp>
#include <System.SyncObjs.hpp>

/**		Miscellaneous routines for amp.
*/

namespace amp {




PACKAGE String SApplicationName;

PACKAGE String SCompanyName;

PACKAGE String GetAppRegistryKey();

/**		Block level mutex sentry.
*/
class MtxLock {
	System::Syncobjs::TMutex* m;
public:
	MtxLock(System::Syncobjs::TMutex* _m);
	void UnLock();
	~MtxLock();
};

/**	Get number of seconds from time.
*/
int get_time_seconds(const TDateTime &tdiff);

/**	Get difference between two times in seconds.
*/
int get_diff_seconds(const TDateTime &tstart, const TDateTime& tend);

/**	increment seconds.
*/
TDateTime & increment_seconds(TDateTime &tstart, int nsecs);

void SaveRegistryIntValue(const String& sName, int value);
int  LoadRegistryIntValue(const String& sName, int def_value);
void SaveRegistryStrValue(const String& sName, const String& value);
String  LoadRegistryStrValue(const String& sName, const String& value = String());


}

#endif
