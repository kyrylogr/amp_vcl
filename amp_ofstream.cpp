//---------------------------------------------------------------------------

#pragma hdrstop

#include "amp_ofstream.h"
#include <stdlib.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)


namespace std_ext {

const int ofstream::default_commit_interval = 60;

ofstream::ofstream(const char* c, std::ios_base::openmode n_modes) :
	std::ofstream(c, n_modes),
	m_sfname(c),
	m_commit_interval(default_commit_interval),
	m_modes(n_modes)
{
	m_last_commit = time(0);
}

void ofstream::commit_if_time_exceeded()
{
	time_t t_now = time(0) ;
	if (abs (t_now - m_last_commit) > m_commit_interval) {
		close();
		open( m_sfname.c_str(), m_modes & ~std::ofstream::trunc | std::ofstream::app);
		m_last_commit = t_now;
	}
}

void ofstream::set_commit_interval(int a_commit_interval)
{
	m_commit_interval = a_commit_interval;
}


}