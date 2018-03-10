//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "MRList.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

MRList::MRList()
{
  FMaxItems = 5;
  FList = new TStringList();
  FBaseList = new TStringList();
  FOnClick = NULL;

}

MRList::~MRList()
{
  delete FList;
  delete FBaseList;
}

void MRList::Clear()
{
  FList->Clear();
  RebuildMenus();
}

void MRList::SetItem(const String& sDesc)
{
	int index = FList->IndexOf(sDesc);
	if (index>=0){
	  FList->Delete(index);
	}
	FList->Insert(0,sDesc);
	while (FList->Count>MaxItems){
		FList->Delete(FList->Count-1);
	}
	RebuildMenus();

}

void MRList::RebuildMenus()
{
  if (PopupMenu)
	FillMenu(PopupMenu->Items);
  FillMenu(MenuItem);
}

String GenHotString (int number)
{
  String result;
  if (number <= 9)
	result = IntToStr(number);
  else
	result = (char)('A' + (char)(number - 10));
  return result;
}

void MRList::AddItem(TMenuItem* item, String sD, int number, int iHotNumber, bool at_top)
{
  TMenuItem* mi = new TMenuItem(item);
  mi->AutoHotkeys = maManual;
  mi->Caption = "&" + GenHotString(iHotNumber) + "  "+sD;
  mi->Tag = number;
  mi->OnClick = FOnClick;
  if (at_top){
	 item->Insert(0, mi);
  }
  else{
	 item->Add(mi);
  }
}

void MRList::FillMenu(TMenuItem* item)
{
  if (!item)
	return;
  item->Clear();
  FillFromStringList(item, FList);
}

void MRList::FillFromStringList(TMenuItem* item, TStringList* list, int iStartHotNumber)
{
  for (int i=0;i<list->Count;i++){
	AddItem(item, list->Strings[i],(int)list->Objects[i], iStartHotNumber+i);
  }
}

void MRList::SetItems(TStrings* strings)
{
/*  FList->Clear();
  for (int i=0;i<strings->Count && i<FMaxItems;i++){
	 AddDesc(FList, strings->Strings[i]);
  }
  RebuildMenus();*/
}

TStrings* MRList::GetItems()
{
  return FList;
}

void MRList::SetTextData(String Value)
{
  FList->Text = Value;
  RebuildMenus();
}

String MRList::GetTextData()
{
  return FList->Text;
}

String MRList::ExtractName(TMenuItem* mi)
{
	if (mi->Caption.Length()>4) {
//	we form names of menu items in the form of "&0  filename"
		return mi->Caption.SubString(5,mi->Caption.Length()-4);
	}
	return "";
}
