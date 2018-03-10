//---------------------------------------------------------------------------
#ifndef MRListH
#define MRListH

//---------------------------------------------------------------------------
#include <menus.hpp>

/** MRU file list
*/
class MRList{
private:
  int        FMaxItems;
  TNotifyEvent FOnClick;
protected:
  TStringList* FList;
  TStringList* FBaseList;  
  TMenuItem* FMenuItem;
  TPopupMenu* FPopupMenu;
  void FillMenu(TMenuItem* item);
  void AddItem(TMenuItem* item, String sD, int number,
               int iHotNumber, bool at_top=false);
  void FillFromStringList(TMenuItem* item, TStringList* list, int iStartHotNumber = 0);
  void RebuildMenus();
  void SetItems(TStrings* strings);
  TStrings* GetItems();
  void SetTextData(String Value);
  String GetTextData();
  void __fastcall DrawItem(TObject *Sender, TCanvas *ACanvas,
          const TRect &ARect, bool Selected);

public:
  MRList();
  ~MRList();
  void SetItem(const String& sDesc);
  String ExtractName(TMenuItem* mi);
  void Clear();
  __property TMenuItem* MenuItem = {read = FMenuItem, write = FMenuItem};
  __property TPopupMenu* PopupMenu = {read = FPopupMenu, write = FPopupMenu};
  __property TNotifyEvent OnClick = {read = FOnClick, write = FOnClick};
  __property TStrings* Items = {read = GetItems, write = SetItems};
  __property String TextData = {read = GetTextData, write = SetTextData};
  __property int MaxItems = {read = FMaxItems, write = FMaxItems};
};

#endif
