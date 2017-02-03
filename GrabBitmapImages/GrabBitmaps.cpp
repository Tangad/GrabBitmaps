#include "stdafx.h"
#include "GrabBitmaps.h"
#include "CPlayBack.h"
#include "DshowUtil.h"
#include "CAVIFileInfo.h"
/* Constructor */
CGrabBitmaps::CGrabBitmaps() 
{
	lAccel=0;
	FileSaveDirectory[MAX_PATH - 1]=0;
	iKeepCurrentFileNumber=0;
	iDirNumber=0;
}
/* Destructor */
CGrabBitmaps::~CGrabBitmaps()
{
}