//---------------------------------------------------------------------------

#ifndef UnitAboutH
#define UnitAboutH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Graphics.hpp>
//---------------------------------------------------------------------------

/**	About dialog.
*/

class TFormAbout : public TForm
{
__published:	// IDE-managed Components
	TMemo *Memo;
	TButton *Button1;
	TImage *Image1;
private:	// User declarations
public:		// User declarations
	__fastcall TFormAbout(TComponent* Owner);
};
//---------------------------------------------------------------------------
#endif
