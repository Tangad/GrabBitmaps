#include "stdafx.h"
#include "CWindowControl.h"
#include "progressbar.h"
#include <windowsx.h>
HRESULT ProgressBar::Create(HINSTANCE hInstance,HWND hParent,DWORD_PTR id,DWORD dwStyle) 
{
	if(m_hwnd!=0)
		return E_FAIL;	
	HWND hwnd=CreateWindowEx(0,
			  				 PROGRESS_CLASS,
							 NULL,
							 WS_CHILD | WS_VISIBLE ,
							 420,
							 40,
							 300,
							 15,
							 hParent,
							 (HMENU)id,
							 hInstance,
							 NULL
							);
	if(hwnd==NULL){
		return E_FAIL;
	}else{
		m_hwnd=hwnd;
		/* Set Range */
		SendMessage(PBM_SETRANGE,(WPARAM)0,MAKELPARAM(0,100));
		/* Set step */
		SendMessage(PBM_SETSTEP,(WPARAM)1,0);
		/* Save handler to the member */
		SetWnd(m_hwnd);
	}
	return S_OK;
}
void ProgressBar::SetWnd(HWND h_wnd)
{
	ProgressBar_hwnd=h_wnd;
}
void ProgressBar::GetWnd(HWND* h_wnd)
{
	*h_wnd=ProgressBar_hwnd;
}