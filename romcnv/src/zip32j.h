/*
	zip32j.h
		by Yoshioka Tsuneo(QWF00133@niftyserve.or.jp)

	Zip Compress Library using Unified Archiver Interface.
	This Program use ZIP32.DLL by Info-Zip
	
	  Unified Archiver Homepage is (http://www.csdinc.co.jp/archiver/)
	  InfoZip Homepage is (http://www.cdrom.com/pub/infozip/)

  ------------------------
	welcome any e-mail!!
	You can use this file as Public Domain Software.
	Copy,Edit,Re-distibute and for any purpose,you can use this file.
   -----------------------
*/

#ifndef ZIP32J_H
#define ZIP32J_H
#include <wtypes.h>

/* #define ZIP_VERSION 14 /* see resource */

WORD WINAPI ZipGetVersion(VOID);
BOOL WINAPI ZipGetRunning(VOID);
BOOL WINAPI ZipConfigDialog(const HWND _hwnd,LPSTR _lpszComBuffer,const int _iMode);
int WINAPI Zip(const HWND _hwnd,LPCSTR _szCmdLine,LPSTR _szOutput,const DWORD _dwSize);
BOOL WINAPI ZipQueryFunctionList(const int _iFunction);
BOOL WINAPI ZipQueryEncryption(VOID);

#if !defined(ISARC_FUNCTION_START)
#define ISARC_FUNCTION_START			0
#define ISARC							0
#define ISARC_GET_VERSION				1
#define ISARC_GET_CURSOR_INTERVAL		2
#define ISARC_SET_CURSOR_INTERVAL		3
#define ISARC_GET_BACK_GROUND_MODE		4
#define ISARC_SET_BACK_GROUND_MODE		5
#define ISARC_GET_CURSOR_MODE			6
#define ISARC_SET_CURSOR_MODE			7
#define ISARC_GET_RUNNING				8

#define ISARC_CHECK_ARCHIVE				16
#define ISARC_CONFIG_DIALOG				17
#define ISARC_GET_FILE_COUNT			18
#define ISARC_QUERY_FUNCTION_LIST		19
#define ISARC_HOUT						20
#define ISARC_STRUCTOUT					21
#define ISARC_GET_ARC_FILE_INFO			22

#define ISARC_OPEN_ARCHIVE				23
#define ISARC_CLOSE_ARCHIVE				24
#define ISARC_FIND_FIRST				25
#define ISARC_FIND_NEXT					26
#define ISARC_EXTRACT					27
#define ISARC_ADD						28
#define ISARC_MOVE						29
#define ISARC_DELETE					30

#define ISARC_GET_ARC_FILE_NAME			40
#define ISARC_GET_ARC_FILE_SIZE			41
#define ISARC_GET_ARC_ORIGINAL_SIZE		42
#define ISARC_GET_ARC_COMPRESSED_SIZE	43
#define ISARC_GET_ARC_RATIO				44
#define ISARC_GET_ARC_DATE				45
#define ISARC_GET_ARC_TIME				46
#define ISARC_GET_ARC_OS_TYPE			47
#define ISARC_GET_ARC_IS_SFX_FILE		48
#define ISARC_GET_FILE_NAME				57
#define ISARC_GET_ORIGINAL_SIZE			58
#define ISARC_GET_COMPRESSED_SIZE		59
#define ISARC_GET_RATIO					60
#define ISARC_GET_DATE					61
#define ISARC_GET_TIME					62
#define ISARC_GET_CRC					63
#define ISARC_GET_ATTRIBUTE				64
#define ISARC_GET_OS_TYPE				65
#define ISARC_GET_METHOD				66
#define ISARC_GET_WRITE_TIME			67
#define ISARC_GET_CREATE_TIME			68
#define ISARC_GET_ACCESS_TIME			69

#define ISARC_FUNCTION_END				69
#endif	/* ISARC_FUNCTION_START */


#endif
