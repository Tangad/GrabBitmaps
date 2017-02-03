#include "stdafx.h"
#include "CWindowControl.h"
#include "ToolTip.h"
//--------------------------------------------------------------------------------------
// Name: ToolTip::Create
// Description: Create an instance of the tool tip. Creating ToolTip on the Main Window
//--------------------------------------------------------------------------------------
HRESULT ToolTip::Create(HWND hParent)
{
    m_hwnd=CreateWindowEx(NULL,
						  TOOLTIPS_CLASS, 
						  NULL, 
						  WS_POPUP,
						  CW_USEDEFAULT, 
						  CW_USEDEFAULT,
						  CW_USEDEFAULT, 
						  CW_USEDEFAULT,
						  hParent, 
						  (HMENU)NULL, 
						  GetInstance(),
						  NULL
						);
    if(m_hwnd){
		SetWindowPos(m_hwnd,HWND_TOPMOST,0, 0, 0, 0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);        
        SendMessage(TTM_ACTIVATE, TRUE, 0);
    }
    if(m_hwnd==NULL)
        return __HRESULT_FROM_WIN32(GetLastError());
    else
        return S_OK;
}
//-----------------------------------------------------------------------------
// Name: ToolTip::AddTool
// Description: Add a new tool to the tool tip.
//-----------------------------------------------------------------------------
BOOL ToolTip::AddTool(HWND hControl,LPTSTR szText)
{
    TOOLINFO tinfo;
    ZeroMemory(&tinfo, sizeof(tinfo));
    tinfo.cbSize =	sizeof(tinfo);
    tinfo.uFlags =	TTF_IDISHWND | TTF_SUBCLASS;
    tinfo.hwnd	 =	hControl;  
    tinfo.uId	 =	(UINT_PTR)hControl;
    tinfo.lpszText=	szText;
    BOOL bResult=(BOOL)SendMessage(TTM_ADDTOOL,0,(LPARAM)&tinfo);
    DWORD count	=(DWORD)SendMessage(TTM_GETTOOLCOUNT, 0, 0); 
    return bResult;
}