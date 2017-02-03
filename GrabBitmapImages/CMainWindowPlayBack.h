#pragma once
#include "CWindowControl.h"
#include "CButtonControl.h"
#include "toolbar.h"
#include "slider.h"
#include "tooltip.h"
#include "textoutt.h"
#include "progressbar.h"
#include "CCreateWindow.h"
#include "CPlayBack.h"
#include "Resource.h"
#include <uuids.h>
const UINT WM_GRAPH_EVENT = WM_APP + 1;
/* public GraphEventCallback */
class CMainWindowPlayBack : public CCreateWindow, public GraphEventCallback
{
public:
	CMainWindowPlayBack();
	~CMainWindowPlayBack();
	LRESULT OnReceiveMessage(UINT uinMessage, WPARAM wParam, LPARAM lParam);
	void OnGraphEvent(long lEventCode, LONG_PTR lptrParam, LONG_PTR lptrParam_);
	int SetGrabArea(int iSetArea);	
public:
	int iAreaCode;
private:
	LPCTSTR ClassName() const {return TEXT("CPlayBack");}
	LPCTSTR	MenuName() const {return MAKEINTRESOURCE(IDC_PLAYBACK);}
	LPCTSTR	WindowName() const {return TEXT("PLAY BACK");}
	void SetVideoRect(RECT*,RECT*);
	void CalculateRegMinMax(void);
	/* HANDLING MESSAGE */
	HRESULT		Create_();
	void		Paint_();
	void		Timer_();
	void		Size_();
	/* HANDLING ACTION */
	void		OpenFile_();
	void		Play_();
	void		Stop_();
	void		Pause_();
	void		GrabBitmaps_();
	/* Notification: WM_NOTIFY */
	void		NotifyW(const NMHDR *pHdr);
	void		NotifySeekBar(const NMSLIDER_INFO *pInfo);
	void		NotifyListView(const NMHDR* pInfo);
	void		UpdateUI();
	void		UpdateSeekBar();
	void		StopTimer();
	void        SetPosProgressBar(void);
	Rebar				MWrebar;
	Toolbar				MWtoolbar;
	Slider 				MWseekbar;
    ToolTip				MWtoolTip;
	TextOutTime			MWTextOut;
	TextOutDuration		MWTextOutDuration;
	ProgressBar			MWProgressBar;
	HBRUSH				MWbrush;
	UINT_PTR			MWtimerID; 
	CPlayBack			*pMWPlayer;
	BOOL fRegionSet;
	RECT ptRegion;
	RECT ptSrcVideo;
	RECT ptDstVideo;
	LONG MinSX;
	LONG MinEX;
};