
#pragma once
class CButtonControl : public CWindowControl
{
public:
	CButtonControl();
	~CButtonControl();
public:
	HRESULT	CreateText(HWND hParent, const TCHAR *szCaption, int nID, const Rect& rcBound);
	HRESULT	CreateBitmap(HWND hParent, int nImgID, int nID, const Rect& rcBound);
	BOOL	SetImage(WORD nImgId);
	void CBC_SetCheck(int nCheck){
		SendMessage(BM_SETCHECK, (WPARAM)nCheck, 0L);
	}
	BOOL CBC_IsChecked(){
		return SendMessage(BM_GETCHECK, 0, 0 )==BST_CHECKED;
	}
};