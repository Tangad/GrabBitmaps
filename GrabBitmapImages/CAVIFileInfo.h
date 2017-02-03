
#pragma once

#include <vfw.h>
#include <string.h>
struct Rate
{
	long lRate;
	long lScale;
	bool bFileDirFlag;
	bool bGrabBitmapFlag;
	bool bAreaSetChanged;
	bool bTimeSetChanged;
};
const int	FILENUMBER=10;
class CAVIFileInfo
{
public:
	CAVIFileInfo();
	~CAVIFileInfo();
public:
	struct Rate FileData[FILENUMBER];
	int			iCurrentFileNumber;
	WCHAR		pSetDirectory[MAX_PATH];
	WCHAR		lpBuffer[MAX_PATH];
	WCHAR		pFileDirectory[MAX_PATH];
	HRESULT		AVIFileProcessing(const TCHAR* sFileName);
	int			SetData(AVISTREAMINFO* SetRate);
	int			GetData(Rate* GetRate);
	void		SetCurrentFileNumber(int iSetFileNumber);
	int			GetCurrentFileNumber(int* pGetFileNumber);
	void		SetCurrentFileDirectory(const WCHAR* pSetDirectoryName);
	int			GetCurrentFileDirectory(WCHAR* pGetDirectoryName);
	WCHAR		*CreateNewDirectory();
	void		SetDirectory(WCHAR* pCreatedDirectory);
	WCHAR		*GetDirectory();
};