#pragma once
// Global Function
HRESULT			Progress_Init();
HRESULT			Progress_Create( HWND hParent, const Rect rc, DWORD_PTR id, HWND *pHwnd);
class ProgressBar : public CWindowControl
{
	public:
		//HRESULT Create(HWND hParent, const Rect& rcSize, DWORD_PTR id);
		HRESULT Create(HINSTANCE hInstance, HWND hParent, DWORD_PTR id, DWORD dwStyle = 0);
		HRESULT SetBackground(HBRUSH hBackground);
		HRESULT SetRange(int nLower,int nUpper);
		HRESULT GetRange(int& nLower, int& nUpper);
		void SetWnd(HWND h_wnd);
		void GetWnd(HWND* h_wnd);
		HWND ProgressBar_hwnd;
};