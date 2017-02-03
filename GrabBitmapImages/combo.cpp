#include "stdafx.h"
#include "CWindowControl.h"
#include "combo.h"
#include <windowsx.h>

namespace ComboControl
{
	LPCTSTR			ClassName		= TEXT("WC_COMBOBOX");
	LPCTSTR			InstanceData	= TEXT("COMBO_PROP");

	struct Combo_Info
	{
		// Logical units
		LONGLONG	lltimePos;

		// GDI
		HBRUSH		hBackGround;

	
	};
	// Window Procedure
	// hwnd is the handler to combo
	LRESULT			CALLBACK Combo_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// Message Handler
	LRESULT			OnCreate(HWND hwnd); 
	LRESULT			OnNcDestroy(HWND hwnd, Combo_Info *pInfo);
	LRESULT			OnPaint(HWND hwnd, Combo_Info *pInfo); 

	// Those are called in Window Procedure 
	LRESULT			OnSetTime( HWND hwnd, LONG lPos, const Combo_Info *pInfo);
	LRESULT			OnGetTime();
	LRESULT			OnSetMinMax();

	//@from OnCreate function 
	inline BOOL		SetInfo(HWND hwnd, Combo_Info *pInfo)
	{
		return SetProp(hwnd, InstanceData, pInfo);
	}

	inline Combo_Info * GetInfo(HWND hwnd)
	{
		return (Combo_Info*)GetProp(hwnd, InstanceData);
	}
	
	// List View wimdow procedure
	// hwnd  is the handler of the Combo Box
	LRESULT			CALLBACK Combo_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		Combo_Info * const pInfo = GetInfo(hwnd);
		switch (uMsg)
		{
		case WM_CREATE:
			return	OnCreate(hwnd);
		case WM_PAINT:
			return	OnPaint(hwnd, pInfo);
		case WM_NCDESTROY:
			return	OnNcDestroy(hwnd, pInfo);
		/*case WM_COMBO_SET_TIME:
			return		OnSetTime(hwnd,(LONG)wParam,pInfo);
		case WM_COMBO_GET_TIME:
			return 1;
		case WM_COMBO_SET_MIN_MAX:
			return 1;


		/*case WM_SLIDER_SET_BACKGROUND:
			return		OnSetBackground(hwnd, (HBRUSH)wParam, pInfo);

		case WM_SLIDER_SET_MIN_MAX:
			return		OnSetMinMax(hwnd, (LONG)wParam, (LONG)lParam, pInfo);

		case WM_SLIDER_SET_POSITION:
			return		OnSetPosition(hwnd, (LONG)wParam, pInfo);

		case WM_SLIDER_GET_POSITION:
			return		OnGetPosition(hwnd, pInfo);
		*/

		default:
			return		DefWindowProc(hwnd, uMsg, wParam, lParam);
		}

		return 0;
	};


	//--------------------------------------------------------------------------------------
    // NotifyParent
    // Description:	Send the parent window a WM_NOTIFY message with our current status.
    //				when an event has occurred or the control requires some information. 
    // hwnd:		Control window.
    // code:		WM_NOTIFY code. (One of the SLIDER_NOTIFY_xxx constants.)
    // pInfo:		Instance data.
    //--------------------------------------------------------------------------------------
	void		NotifyParent(HWND hwnd, UINT NotifyCode, Combo_Info *pInfo)
	{
		// hwnd	¨ Handler to the hwnd
		// code	¨ Notification message 
		// pInfo¨ Slider time position

		// Get Handler to the parent window
	/*	HWND	hParent = GetParent(hwnd);

		if (hParent)
		{	
			NM_DISP_INFO		ComboInfo;

			ComboInfo.hdr.hwndFrom		= hwnd;
			ComboInfo.hdr.code			= NotifyCode;
			ComboInfo.hdr.idFrom		= (UINT)GetMenu(hwnd);
			ComboInfo.llPosition		= pInfo->lltimePos;

			// Notify to Parent Window
			SendMessage( hParent, WM_NOTIFY, (WPARAM)ComboInfo.hdr.idFrom, (LPARAM)&ComboInfo);
		}
		*/
	}

	// Create Function 
	LRESULT		OnCreate(HWND hwnd)
	{
		Combo_Info	*pInfo = new Combo_Info();
		if(!pInfo)
		{
			return (LRESULT) -1;
		}
		ZeroMemory(pInfo, sizeof(Combo_Info));
		pInfo->hBackGround	= CreateSolidBrush(RGB(0xFF, 0x80, 0x80));
		
        if (SetInfo(hwnd, pInfo))
        {
            return 0;
        }
        else
        {
            delete pInfo;
            return -1;
        }
		
		return 0;		
	}
	LRESULT			OnNcDestroy(HWND hwnd, Combo_Info *pInfo)
	{
		//if pointer is not null
		if(pInfo)
		{
			DeleteObject(pInfo->hBackGround);
			//DeleteObject(pInfo->lltimePos );
			delete pInfo;
		}
		return 0;
	
	}
	LRESULT			OnPaint(HWND hwnd, Combo_Info *pInfo)
	{
		PAINTSTRUCT		ps;
		HDC				hdc;

		hdc = BeginPaint(hwnd, &ps);

		// Draw the background
		if (pInfo->hBackGround)
		{
			//FillRect(hdc, &ps.rcPaint, pInfo->hBackGround );
		}

		EndPaint(hwnd, &ps);
		return 0;
	}

	
}
// Initialize the Combo
HRESULT		Combo_Init()
{
	WNDCLASSEX		wndClass;
	ZeroMemory(&wndClass, sizeof(WNDCLASSEX));

	wndClass.cbSize				=	sizeof(WNDCLASSEX)					;
	wndClass.lpfnWndProc		=	ComboControl::Combo_WndProc			;
	wndClass.hInstance			=	GetInstance()						;
	wndClass.lpszClassName		=	ComboControl::ClassName 			;
	wndClass.cbWndExtra			=	sizeof(ComboControl::Combo_Info )	;

	ATOM	 a = RegisterClassEx(&wndClass);
	if(a == 0)
	{
		return __HRESULT_FROM_WIN32(GetLastError());
	}else{
		 return S_OK;
	}
	
}
HRESULT		Combo::Create(HINSTANCE hInstance, HWND hParent, DWORD_PTR id, DWORD dwStyle)
{
	if(m_hwnd != 0)
	{
		return E_FAIL;
	}
	HRESULT		hResult		=	S_OK; 
	//	Load the Common Control dll
	INITCOMMONCONTROLSEX		InitListBox;
	InitListBox.dwSize		=	sizeof(INITCOMMONCONTROLSEX);
	InitListBox.dwICC		=	ICC_USEREX_CLASSES;
	BOOL	bReturn			=	InitCommonControlsEx(&InitListBox);
	if(bReturn == FALSE)
	{
		hResult = E_FAIL;
		return hResult;
	}
	// Creating the Combo Box ¨ Control dll was loaded, WC_COMBOBOX ¨@commctrl.h
	HWND	hwnd = CreateWindowEx(	0, WC_EDIT, NULL,
									WS_CHILD |ES_MULTILINE | WS_VISIBLE |ES_OEMCONVERT       ,
									400,		
									0,			
									100,		 
									30,			
									hParent,
									(HMENU)id,
									hInstance,
									NULL
								);

	if(hwnd == NULL)
	{
		hResult = E_FAIL;
		return hResult;

	}else{	
		// Set the handler to the Combo, to CWindowControl`s m_hwnd
		m_hwnd	= hwnd;

		NMCOMBOBOXEX			IdDataCombo;
		IdDataCombo.hdr.code		= NULL;
		IdDataCombo.hdr.hwndFrom	= hwnd;
		IdDataCombo.hdr.idFrom		= id;

		SetComboId(&IdDataCombo);
		
	}

	return hResult;	

}


HRESULT		Combo::SetPosToCombo(LONGLONG llPos)
{
	HRESULT			hResult	= S_OK;

	HWND			hParent = GetParent(m_hwnd);
	if(hParent == NULL)
	{
		hResult = E_FAIL;
		return hResult;
	}

	NMCOMBOBOXEX*	pIdCombo_ = GetComboId();

	if(pIdCombo_ == NULL)
	{
		hResult = E_FAIL;
		return hResult;
	}
	
	// Text out method
	HDC	hdc = GetDC (m_hwnd) ;
	
	// rect must be validate, if there is BeginPaint(), retc is automatically validated
	ValidateRect (m_hwnd, NULL) ;
	

	ReleaseDC (m_hwnd, hdc) ;

	
	BOOL	bReturn = SetDlgItemInt(hParent, pIdCombo_->hdr.idFrom,(UINT)llPos, TRUE);
	if(bReturn != TRUE)
	{
		hResult = E_FAIL;
	}
	return hResult;
}

HRESULT		Combo::SetComboId(NMCOMBOBOXEX	*pComboId)
{
	pIdCombo = new NMCOMBOBOXEX;
	*pIdCombo = *pComboId;
	return  S_OK;
}

NMCOMBOBOXEX*	Combo::GetComboId() 
{
	return pIdCombo;
}