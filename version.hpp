// version.hpp

#ifndef VERSION_HPP
#define VERSION_HPP

#pragma comment(lib, "version.lib")

#include <string>
#include <sstream>
#include <iomanip>
#include <exception>
#include <new>
#include <windows.h>
#include <tchar.h>

namespace version_nmsp
{
	struct language
	{
		WORD language_;
		WORD code_page_;

		language()
		{
			language_  = 0;
			code_page_ = 0;
		}
	};
}


class version
{
public:
	version()
	{
		{
			String sapp = ExtractFileName(Application->ExeName);
			DWORD h = 0;

			DWORD resource_size = ::GetFileVersionInfoSize(sapp.c_str(), &h);
			if(resource_size)
			{
				resource_data_ = new unsigned char[resource_size];
				if(resource_data_)
				{
					if(::GetFileVersionInfo(sapp.c_str(),
																	0,
																	resource_size,
																	static_cast<LPVOID>(resource_data_)) != FALSE)
					{
						UINT size = 0;

						// Get language information
						if(::VerQueryValue(static_cast<LPVOID>(resource_data_),
															 _T("\\VarFileInfo\\Translation"),
															 reinterpret_cast<LPVOID*>(&language_information_),
															 &size) == FALSE)
							throw Exception("Requested localized version information not available");
					}
					else
					{
						std::stringstream exception;
						exception << "Could not get version information (Windows error: " << ::GetLastError() << ")";
						throw Exception(exception.str().c_str());
					}
				}
				else
					throw Exception("Allocation failed");
			}
      else
      {
        std::stringstream exception;
        exception << "No version information found (Windows error: " << ::GetLastError() << ")";
        throw Exception(exception.str().c_str());
      }
    }
	}

	~version() { delete [] resource_data_; }
	String get_product_name() const { return get_value("ProductName"); }
	String get_internal_name() const { return get_value("InternalName"); }
	String get_product_version() const { return get_value("ProductVersion"); }
	String get_special_build() const { return get_value("SpecialBuild"); }
	String get_private_build() const { return get_value("PrivateBuild"); }
	String get_copyright() const { return get_value("LegalCopyright"); }
	String get_trademarks() const { return get_value("LegalTrademarks"); }
	String get_comments() const { return get_value("Comments"); }
	String get_company_name() const { return get_value("CompanyName"); }
	String get_file_version() const { return get_value("FileVersion"); }
	String get_file_description() const { return get_value("FileDescription"); }

private:
	unsigned char          *resource_data_;
	version_nmsp::language *language_information_;

	String get_value(const std::string &key) const
	{
		if(resource_data_)
		{
			UINT              size   = 0;
			std::stringstream t;
			LPVOID            value  = 0;

			// Build query string
			t << "\\StringFileInfo\\" << std::setw(4) << std::setfill('0') << std::hex
				<< language_information_->language_ << std::setw(4) << std::hex
				<< language_information_->code_page_ << "\\" << key;

			String 					  sFileInfo = t.str().c_str();

			if(::VerQueryValue(static_cast<LPVOID>(resource_data_),
												 sFileInfo.c_str(),
												 static_cast<LPVOID*>(&value),
												 &size) != FALSE)
        return String(static_cast<LPTSTR>(value));
    }

    return "";
  }
};

#endif