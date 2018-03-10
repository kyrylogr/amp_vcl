//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "UnitAbout.h"
#include "version.hpp"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
__fastcall TFormAbout::TFormAbout(TComponent* Owner)
	: TForm(Owner)
{
	Memo->Lines->Add("Version: " + version().get_product_version());

}
//---------------------------------------------------------------------------









