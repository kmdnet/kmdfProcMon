#include<Windows.h>
#include<stdio.h>

#include"Header.h"


HANDLE hDriver;
HANDLE hEvent;

PROCCALLBACKINFO gProcCallBackInfo = { 0 };


void ProcMonitor(void) {
	BOOL bRet;
	DWORD BytesReturned;
	CHAR errmsg[256];

	PROCCALLBACKINFO ProcCallbackInfo;
	OVERLAPPED Overlapped;

	Overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (Overlapped.hEvent == NULL) {
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, errmsg, sizeof(errmsg), NULL);
		printf("CreateEvent : %hs\n", errmsg);
		return;
	}

	bRet = DeviceIoControl(
		hDriver,
		IOCTL_GET_INFO,
		0,
		0,
		&ProcCallbackInfo,
		sizeof(PROCCALLBACKINFO),
		&BytesReturned,
		NULL
	);
	if (bRet == FALSE) {
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, errmsg, sizeof(errmsg), NULL);
		printf("DeviceIoControl -IOCTL_GET_INFO- : %hs\n", errmsg);
		return;
	}


	if (BytesReturned > 0) {
		if (gProcCallBackInfo.bCreate != ProcCallbackInfo.bCreate ||
			gProcCallBackInfo.hParentId != ProcCallbackInfo.hParentId ||
			gProcCallBackInfo.hProcessId != ProcCallbackInfo.hProcessId) {

			if (ProcCallbackInfo.bCreate) {
				printf("hProcessId : 0x%x Create \n", ProcCallbackInfo.hProcessId);
			}
			else {
				printf("hProcessId : 0x%x Delete \n", ProcCallbackInfo.hProcessId);
			}

			printf("Path : %S \n", ProcCallbackInfo.PathName);

			gProcCallBackInfo.bCreate = ProcCallbackInfo.bCreate;
			gProcCallBackInfo.hParentId = ProcCallbackInfo.hParentId;
			gProcCallBackInfo.hProcessId = ProcCallbackInfo.hProcessId;
		}
	}

}


void ProcRun(void) {
	DWORD dRet;
	CHAR errmsg[256];

	printf("ProcRun start\n");

	while (TRUE) {
		dRet = WaitForSingleObject(hEvent, INFINITE);
		if (dRet == WAIT_FAILED) {
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, errmsg, sizeof(errmsg), NULL);
			printf("WaitForSingleObjec : %hs\n", errmsg);
			return;
		}
		ProcMonitor();
	}
}


void ioctl_event(void) {
	BOOL bRet;
	DWORD BytesReturned;
	CHAR errmsg[256];

	PROCCALLBACKINFO ProcCallbackInfo;

	hDriver = CreateFile(
		TEXT("\\\\.\\test"),
		GENERIC_ALL,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL
	);
	if (hDriver == INVALID_HANDLE_VALUE) {
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, errmsg, sizeof(errmsg), NULL);
		printf("CreateFile2 : %hs\n", errmsg);
		return;
	}


	hEvent = OpenEvent(SYNCHRONIZE, FALSE, TEXT("Global\\Proctest"));
	if (hEvent == NULL) {
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, errmsg, sizeof(errmsg), NULL);
		printf("OpenEvent : %hs\n", errmsg);
		return;
	}

	ProcRun();

}


void ioctl_event_init(void) {
	HANDLE hFile;
	BOOL bRet;
	DWORD BytesReturned;
	CHAR errmsg[256];

	printf("ioctl_event_start\n");

	hFile = CreateFile(
		TEXT("\\\\.\\test"),
		GENERIC_ALL,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		NULL,
		NULL
	);
	if (hFile == INVALID_HANDLE_VALUE) {
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, errmsg, sizeof(errmsg), NULL);
		printf("CreateFile1 : %hs\n", errmsg);
		return;
	}

	bRet = DeviceIoControl(
		hFile,
		IOCTL_PROC_START,
		NULL,
		0,
		NULL,
		0,
		&BytesReturned,
		NULL
	);
	if (bRet == FALSE) {
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, errmsg, sizeof(errmsg), NULL);
		printf("DeviceIoControl -IOCTL_PROC_START- : %hs\n", errmsg);
		return;
	}


}


void ioctl_event_stop(void) {
	HANDLE hFile;
	BOOL bRet;
	DWORD BytesReturned;
	CHAR errmsg[256];

	printf("ioctl_event_stop\n");

	hFile = CreateFile(
		TEXT("\\\\.\\test"),
		GENERIC_ALL,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		NULL,
		NULL
	);
	if (hFile == INVALID_HANDLE_VALUE) {
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, errmsg, sizeof(errmsg), NULL);
		printf("CreateFile1 : %hs\n", errmsg);
		return;
	}

	bRet = DeviceIoControl(
		hFile,
		IOCTL_PROC_STOP,
		NULL,
		0,
		NULL,
		0,
		&BytesReturned,
		NULL
	);
	if (bRet == FALSE) {
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, errmsg, sizeof(errmsg), NULL);
		printf("DeviceIoControl -IOCTL_PROC_STOP- : %hs\n", errmsg);
		return;
	}


}
