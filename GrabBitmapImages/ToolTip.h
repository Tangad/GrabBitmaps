#pragma once
class ToolTip : public CWindowControl
{
public:
    HRESULT Create(HWND hParent);
    BOOL AddTool(HWND hControl,LPTSTR szText);
};