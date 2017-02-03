#pragma once
#include <dshow.h>
#include <qedit.h>
#include <math.h>
#include "CAVIFileInfo.h"

enum CPlayBackState
{
	STATE_RUNNING,			// When Video is playing
	STATE_PAUSED,			// Wheb Video is paused
	STATE_STOPPED,			// When Video is stopped
	STATE_GRABBING,			// When Video is grabbing
	STATE_SETTING_AREA,		// When Area of Grabbing is set
	STATE_CLOSED
};
//-----------------------------------
// This struct containing pure virtual functions are termed "abstract"
// they cannot be instatntiated(action reply) subclasses of 
//-----------------------------------
struct GraphEventCallback
{
	/* Pure Virtual Function */
	virtual void OnGraphEvent(long lEventCode,LONG_PTR lptrParam,LONG_PTR lptrParam_)=0;
};
class CPlayBack
{
public:
	CPlayBack();
	CPlayBack(HWND hwndVideo_);
	~CPlayBack();
	HRESULT SetEventWindow(HWND hwnd, UINT uiMessage);
	CPlayBackState State() const {return pbState; }
	HRESULT OpenFile(const TCHAR* sFileName);
	HRESULT Play();
	HRESULT	Pause();
	HRESULT	Stop();
	HRESULT	GrabBitmaps();
	HRESULT	SetGrabTime(int* pSelectAreaCode);
	BOOL	HasVideo() const { return pWindowlessControl != NULL; }
	HRESULT	UpdateVideoWindow(const LPRECT prc);
	HRESULT	Repaint(HDC hdc);
	HRESULT	DisplayModeChanged();
	HRESULT	HandleGraphEvent(GraphEventCallback* pCB);
	BOOL	CanSeek() const;
	HRESULT	SetPosition(REFERENCE_TIME pos);
	HRESULT	GetDuration(LONGLONG* pDuration);
	HRESULT	GetCurrentPosition(LONGLONG* pTimeNow);
	void SetParentWnd(HWND h_wnd);
	int GetGrabRatio(void);
	void SetGrabRatio(int iSaveGrabRatio);
	void SetRegionWasSetFlag(BOOL);
	void SetVideoRect(RECT*,RECT*);
	void GetVideoRect(RECT*, RECT*);
private:
	HRESULT InitializeGraph();
	void TearDownGraph();
	HRESULT RenderStreams(IBaseFilter* pSource);
	IPin* GetOutPin(IBaseFilter* pBaseFilterOutPin,int iPin);
	IPin* GetInPin(IBaseFilter* pBaseFilterInPin,int iPin);
	HRESULT GetPin( IBaseFilter* pBaseFilterPin,PIN_DIRECTION dirRequired,int iNum,IPin **ppPin);
	void InitializeFlags();
	CPlayBackState pbState;	
	HWND hwndVideo;	
	HWND hwndEvent;				
	UINT uiEventMsg;			
	DWORD dwSeekCaps;				
	WCHAR tchFileName[MAX_PATH];
	IGraphBuilder* pGraphBuilder;	
	IMediaControl* pMediaControl;	
	IMediaEventEx* pMediaEvent;	
	IMediaSeeking* pMediaSeek;
	IBasicAudio* pBasicAudio;				
	/*	Grab Bitmaps */
	IBaseFilter* pBaseFilter;
	IBaseFilter* pSampleGrabberFilter;		
	VIDEOINFOHEADER* pVideoInfoHeader;
	AM_MEDIA_TYPE MediaType;
	/* VideoWindow :: Windowless mode */
	IVMRWindowlessControl* pWindowlessControl;	
	/* Video playback time */
	REFERENCE_TIME rCurrentTime;
	REFERENCE_TIME rStartTime;
	REFERENCE_TIME rEndTime;
	REFERENCE_TIME rDuration;
	/* Grab Area Set Information */
	BOOL bStartArea;
	BOOL bEndArea;
	int iKeepFileNumber;
	int m_iSaveGrabRatio;
	HWND Parent_hwnd;
	BOOL fRegionSet_;
	RECT rectBitmap;
	RECT rectSrcVideo;
	RECT rectDstVideo;
};