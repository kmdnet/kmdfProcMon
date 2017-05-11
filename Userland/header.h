#pragma once

#define IOCTL_PROC_START CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PROC_STOP CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_INFO CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

// MAX_PATH : 260
#define PATH_LEN 512

void ioctl_event_init(void);
void ioctl_event(void);
void ioctl_event_stop(void);

typedef struct _ProcCallbackInfo {
	HANDLE hParentId;
	HANDLE hProcessId;
	BOOLEAN bCreate;

	WCHAR PathName[PATH_LEN];
}PROCCALLBACKINFO, *PPROCCALLBACKINFO;

