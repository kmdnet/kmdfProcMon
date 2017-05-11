#pragma once


#define IOCTL_PROC_START CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PROC_STOP CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_INFO CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define PATH_LEN 512

NTSTATUS DispatchIoctl(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS DispatchCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp);
void UnloadDriver(PDRIVER_OBJECT DriverObject);
void ProcessCallback(HANDLE hParentId, HANDLE hProcessId, BOOLEAN bCreate);

void GetPathName(HANDLE hPid, WCHAR* PathName);

DRIVER_INITIALIZE DriverEntry;

// GLOBAL OBJECT;
PDEVICE_OBJECT gpDeviceObject;

// Event
PKEVENT pEvent;
HANDLE hEvent;

// Callback
typedef struct _ProcCallbackInfo {
	HANDLE hParentId;
	HANDLE hProcessId;
	BOOLEAN bCreate;

	WCHAR PathName[PATH_LEN];
}PROCCALLBACKINFO, *PPROCCALLBACKINFO;

typedef struct _LOCAL_DEVICE_EXTENSION {
	PDEVICE_OBJECT DeviceObject;
	PROCCALLBACKINFO ProcInfo;
}LOCAL_DEVICE_EXTENSION, *PLOCAL_DEVICE_EXTENSION;


typedef NTSTATUS(*ZWQUERYINFORMATIONPROCESS)(
	_In_      HANDLE           ProcessHandle,
	_In_      PROCESSINFOCLASS ProcessInformationClass,
	_Out_     PVOID            ProcessInformation,
	_In_      ULONG            ProcessInformationLength,
	_Out_opt_ PULONG           ReturnLength
	);


