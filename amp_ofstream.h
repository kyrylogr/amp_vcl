//---------------------------------------------------------------------------

#ifndef amp_ofstreamH
#define amp_ofstreamH
//---------------------------------------------------------------------------
#include <fstream>
#include <string>
#include <time.h>

/** ofstream with easy reopen.
*   (effort to fix server 2008 issue of not updating last write date if file is not closed).
*
*/

namespace std_ext {

class ofstream: public std::ofstream {
private:
 std::string m_sfname;
 time_t      m_last_commit;
 int         m_commit_interval;
 std::ios_base::openmode 				 m_modes;
public:
 ofstream(const char* c, std::ios_base::openmode n_modes);
 void commit_if_time_exceeded();
 void set_commit_interval(int a_commit_interval);
public:
 static const int default_commit_interval;

};

}

#endif
