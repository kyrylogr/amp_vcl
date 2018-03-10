//---------------------------------------------------------------------------

#ifndef zip_procesH
#define zip_procesH
//---------------------------------------------------------------------------

#include "ZipArchive.h"
#include <classes.hpp>

/**	zip archiving operations
*/


/**	callback to be passed in zip_operation so that the caller can be notiofied
* about the progress and cancel the process. amp::performer implements this interface.
*/

class progress_interface {
protected:
public:
/**	return false to continue
*/
	virtual bool progress(ZIP_SIZE_TYPE pos_max, ZIP_SIZE_TYPE pos_current, String info )=0;
};

/** callback passed to ziparchive routines
*/
class CAmpZipCallback : public CZipActionCallback
{
	int*				m_p_abort_flag;
	progress_interface* m_pi;
	bool				m_was_breaked;
protected:
	bool Callback(ZIP_SIZE_TYPE  iProgress);
public:
	CAmpZipCallback(int* b_abort_flag, progress_interface* pi =NULL);
};

/**	facade method for performing zipping operations.
*   	btimestamp   - include timestamp in file name
*		b_abort_flag - pointer to abort flag
*		pi			 - pointer to progress interface
*/

void zip_operation(String source, String target, int izlevel, bool btimestamp=false, int* b_abort_flag=NULL, progress_interface* pi=NULL);

#endif
