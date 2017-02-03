#pragma once
/* Global function */
HRESULT TextOut_Init();
HRESULT TextOut_Init_();
/* Structure */
typedef struct tag_NMTEXTOUT_INFO {
    NMHDR hdr;
    LONGLONG llPosition;
} NM_TEXTOUT_INFO, *LPNMTEXTOUT_INFO;
//---------------------------------------------
// Postion(in time) of video is set
//---------------------------------------------
class TextOutTime:public CWindowControl
{
public:
	HRESULT Create(HINSTANCE hInstance,HWND hParent,DWORD_PTR id,DWORD dwStyle);
	HRESULT SetPosToTextOut(LONGLONG llPos,BOOL bInitializedFlag);
	void SetDurationToPrivate(LONGLONG llPos);
	LONGLONG* GetDurationOfPrivate();
	HRESULT SetTextOutId(NMHDDISPINFO *pIdTextOut_);
	NMHDDISPINFO* GetTextOutId();
	void SetPosition(const RECT rc);
	RECT* GetPosition(void);
	void NotifyPositionChange();
	RECT m_RectRec;
private:
	NMHDDISPINFO* pIdTextOut;
	LONGLONG llDuration;
};
//---------------------------------------------
// Duration of video is set
//---------------------------------------------
class TextOutDuration: public CWindowControl
{
public:
	HRESULT Create(HINSTANCE hInstance,HWND hParent,DWORD_PTR id,DWORD dwStyle);
	HRESULT	SetDurationToTextOut(LONGLONG llPos,BOOL bInitializedFlag);
	void SetDurationToPrivateD(LONGLONG llPos);
	LONGLONG* GetDurationOfPrivateD();
	HRESULT SetTextOutIdD(NMHDDISPINFO *pIdTextOut_);
	NMHDDISPINFO* GetTextOutIdD();
	void SetPosition(const RECT rc);
	RECT* GetPostition(void);
	RECT m_RectDur;
private:
	NMHDDISPINFO* pIdTextOutD;
	LONGLONG llDurationD;
};