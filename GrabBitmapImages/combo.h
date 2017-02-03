#pragma once



// Global Functions
HRESULT				Combo_Init();
HRESULT				Combo_Create(HINSTANCE hInstance, HWND hParent, DWORD_PTR id, DWORD dwStyle, HWND *lvHwnd);

// Messages
const	UINT		WM_COMBO_SET_TIME				= WM_USER + 6;			
const	UINT		WM_COMBO_GET_TIME				= WM_USER + 7;			
const	UINT		WM_COMBO_SET_MIN_MAX			= WM_USER + 8;		

// Structure
typedef struct tagPETINFO
{
    WCHAR			szKind[10]	;
    WCHAR			szBreed[50]	;
    WCHAR			szPrice[20]	;

}PETINFO;

// Combo Class inherits Slider Class: Slider class inherits CWindowControl
class Combo	: public CWindowControl
{
public:

	HRESULT			Create(HINSTANCE hInstance, HWND hParent, DWORD_PTR id, DWORD dwStyle);
	HRESULT			SetPosToCombo(LONGLONG llPos);
	HRESULT			SetDurationToCombo(LONGLONG llPos);

	HRESULT			SetComboId(NMCOMBOBOXEX	*pComboId);
	NMCOMBOBOXEX*	GetComboId();



private:
	NMCOMBOBOXEX	*pIdCombo;
	HWND			hParentWindow;
	LONGLONG		llRecentTime;
	LONGLONG		llDuration;
	

};