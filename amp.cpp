//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <tchar.h>
#include "UnitMain.h"

//---------------------------------------------------------------------------
USEFORM("UnitSettings.cpp", FormSettings);
USEFORM("UnitMain.cpp", FormMain);
USEFORM("UnitAbout.cpp", FormAbout);
//---------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
	try
	{
		Application->Initialize();
		//Application->MainFormOnTaskBar = true;
		Application->CreateForm(__classid(TFormMain), &FormMain);
		if (ParamCount()>=1) {
			FormMain->LoadFromFile(ParamStr(1));
			FormMain->FStartAfterShow = true;
		}
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	catch (...)
	{
		try
		{
			throw Exception("");
		}
		catch (Exception &exception)
		{
			Application->ShowException(&exception);
		}
	}
	return 0;
}
//---------------------------------------------------------------------------
