#include "stdafx.h"
#include "CWindowControl.h"
#include "CButtonControl.h"
/* Constructor */
CButtonControl::CButtonControl() 
{
}
/* Destructor */
CButtonControl::~CButtonControl()
{
}
//----------------------------------------------------------
//Method @@@
//				CreateText(HWND hParent, const TCHAR *szCaption,
//                         int nID,	const Rect& rcBound)
//Description @@@
//				Creates a simple text button
//				
//------------------------------------------------------------
HRESULT CButtonControl::CreateText(HWND hParent, const TCHAR *szCaption, int nID, 
                               const Rect& rcBound)
{
    CREATESTRUCT crtStrsuct;
	ZeroMemory(&crtStrsuct, sizeof(CREATESTRUCT));
    crtStrsuct.x			= rcBound.left;
    crtStrsuct.y			= rcBound.top;
    crtStrsuct.cx			= rcBound.right - crtStrsuct.x;
    crtStrsuct.cy			= rcBound.bottom - crtStrsuct.y;
    crtStrsuct.hwndParent	= hParent;
    crtStrsuct.lpszName		= szCaption;
    crtStrsuct.hMenu		= (HMENU)(INT_PTR)nID;
    crtStrsuct.lpszClass	= TEXT("BUTTON");
    crtStrsuct.style		= BS_PUSHBUTTON | BS_FLAT;
    return CWindowControl::Create(crtStrsuct);
}
//----------------------------------------------------------
//Method @@@
//				CreateBitmap(HWND hParent, int nImgID, 
//							 int nID, const Rect& rcSize)
//Description @@@
//				 Creates a simple bitmap button
//				
//------------------------------------------------------------
HRESULT CButtonControl::CreateBitmap(HWND hParent, int nImgID, int nID, const Rect& rcSize)
{
    HRESULT hResult=CreateText(hParent,NULL,nID,rcSize);
    if(SUCCEEDED(hResult))
        SetImage(nImgID);
    return hResult;
}
//----------------------------------------------------------
//Method @@@
//				SetImage(WORD nImgId)
//Description @@@
//				 Set a bitmap for the button
//				
//------------------------------------------------------------
BOOL CButtonControl::SetImage(WORD nImgId)
{
    AddStyle(BS_BITMAP);
    HBITMAP hBitmap=SetBitmapImg(GetInstance(),nImgId,m_hwnd);
    return (hBitmap ? TRUE : FALSE);
}