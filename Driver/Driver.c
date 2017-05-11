#include<ntifs.h>
#include<ntstrsafe.h>

#include"header.h"

#pragma warning(disable: 4100)


NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	UNICODE_STRING DeviceName, DosDevicesName, BaseName;
	PDEVICE_OBJECT pDeviceObject;

	RtlInitUnicodeString(&DeviceName, L"\\Device\\test");
	RtlInitUnicodeString(&DosDevicesName, L"\\DosDevices\\test");

	status = IoCreateDevice(DriverObject, sizeof(LOCAL_DEVICE_EXTENSION), &DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObject);
	if (!NT_SUCCESS(status)) {
		DbgPrint("IoCreateDevice Error");
		return status;
	}

	status = IoCreateSymbolicLink(&DosDevicesName, &DeviceName);
	if (!NT_SUCCESS(status)) {
		DbgPrint("IoCreateSymbolicLink Error");
		IoDeleteDevice(pDeviceObject);
		return status;
	}

	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchIoctl;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchCreateClose;
	DriverObject->DriverUnload = UnloadDriver;

	gpDeviceObject = pDeviceObject;

	RtlInitUnicodeString(&BaseName, L"\\BaseNamedObjects\\Proctest");

	pEvent = IoCreateNotificationEvent(&BaseName, &hEvent);
	if (pEvent == NULL) {
		DbgPrint("IoCreateNotificationEvent : %08x", status);
		IoDeleteDevice(pDeviceObject);
		return status;
	}
	KeClearEvent(pEvent);


	DbgPrint("DriverEntry OK\n");

	return STATUS_SUCCESS;
}


NTSTATUS DispatchIoctl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION pIoStackLocation;
	ULONG IoControlCode;

	PPROCCALLBACKINFO pProcCallbackInfo;
	PLOCAL_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;

	pIoStackLocation = IoGetCurrentIrpStackLocation(Irp);
	IoControlCode = pIoStackLocation->Parameters.DeviceIoControl.IoControlCode;

	//DbgPrint("DispatchIoctl\n");

	switch (IoControlCode) {
	case IOCTL_PROC_START:
		DbgPrint("IOCTL_PROC_START");
		status = PsSetCreateProcessNotifyRoutine(ProcessCallback, FALSE);
		if (!NT_SUCCESS(status)) {
			DbgPrint("PsSetCreateProcessNotifyRoutine : 0x%x", status);
			return status;
		}
		break;
	case IOCTL_PROC_STOP:
		DbgPrint("IOCTL_PROC_STOP");
		status = PsSetCreateProcessNotifyRoutine(ProcessCallback, TRUE);
		if (!NT_SUCCESS(status)) {
			DbgPrint("PsSetCreateProcessNotifyRoutine : 0x%x", status);
			return status;
		}
		break;
	case IOCTL_GET_INFO:
		//DbgPrint("IOCTL_GET_INFO");
		pProcCallbackInfo = Irp->AssociatedIrp.SystemBuffer;
		pProcCallbackInfo->hParentId = extension->ProcInfo.hParentId;
		pProcCallbackInfo->hProcessId = extension->ProcInfo.hProcessId;
		pProcCallbackInfo->bCreate = extension->ProcInfo.bCreate;
		wcscpy_s(pProcCallbackInfo->PathName, PATH_LEN, extension->ProcInfo.PathName);

		break;
	default:
		break;
	}

	Irp->IoStatus.Status = status;

	if (status == STATUS_SUCCESS) {
		Irp->IoStatus.Information = pIoStackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	}

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;
}


NTSTATUS DispatchCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	//DbgPrint("DispatchCreateClose");

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}


void UnloadDriver(PDRIVER_OBJECT DriverObject) {
	UNICODE_STRING DosDevicesName;

	//DbgPrint("UnloadDriver");
	IoDeleteDevice(DriverObject->DeviceObject);
	RtlInitUnicodeString(&DosDevicesName, L"\\DosDevices\\test");
	IoDeleteSymbolicLink(&DosDevicesName);
}


void GetPathName(HANDLE hPid, WCHAR* PathName) {
	HANDLE hProcess;
	ULONG ReturnLength;
	PVOID buf;
	UNICODE_STRING funcName;
	PUNICODE_STRING ImageFileName;
	PEPROCESS pEprocess;
	NTSTATUS status = STATUS_SUCCESS;

	ZWQUERYINFORMATIONPROCESS ZwQueryInformationProcess = NULL;

	status = PsLookupProcessByProcessId(hPid, &pEprocess);
	if (!NT_SUCCESS(status)) {
		DbgPrint("PsLookupProcessByProcessId");
		return;
	}

	RtlInitUnicodeString(&funcName, L"ZwQueryInformationProcess");
	ZwQueryInformationProcess = (ZWQUERYINFORMATIONPROCESS)MmGetSystemRoutineAddress(&funcName);
	if (ZwQueryInformationProcess == NULL) {
		DbgPrint("MmGetSystemRoutineAddress");
		return;
	}

	// Get Handle
	status = ObOpenObjectByPointer(pEprocess, 0, NULL, 0, 0, KernelMode, &hProcess);
	if (!NT_SUCCESS(status)) {
		DbgPrint("ObOpenObjectByPointer");
		return;
	}

	ObDereferenceObject(pEprocess);

	// Get PathLen
	// STATUS_INFO_LENGTH_MISMATCH = 0xC0000004
	status = ZwQueryInformationProcess(hProcess, ProcessImageFileName, NULL, 0, &ReturnLength);
	if (status != STATUS_INFO_LENGTH_MISMATCH) {
		DbgPrint("ZwQueryInformationProcess -1 0x%x", status);
		return;
	}

	buf = ExAllocatePool(NonPagedPool, ReturnLength);
	if (buf == NULL) {
		DbgPrint("ExAllocatebuf");
		return;
	}

	// Retrive buf
	status = ZwQueryInformationProcess(hProcess, ProcessImageFileName, buf, ReturnLength, &ReturnLength);
	if (!NT_SUCCESS(status)) {
		DbgPrint("ZwQueryInformationProcess -2 0x%x", status);
		ExFreePool(buf);
		return;
	}

	ImageFileName = (PUNICODE_STRING)buf;
	status = RtlStringCbCopyUnicodeString(PathName, PATH_LEN, ImageFileName);
	if (!NT_SUCCESS(status)) {
		DbgPrint("RtlStringCbCopyUnicodeString : 0x%x", status);
		return;
	}

	ExFreePool(buf);
}


void ProcessCallback(HANDLE hParentId, HANDLE hProcessId, BOOLEAN bCreate) {
	PLOCAL_DEVICE_EXTENSION extension;

	extension = gpDeviceObject->DeviceExtension;
	extension->ProcInfo.hParentId = hParentId;
	extension->ProcInfo.hProcessId = hProcessId;
	extension->ProcInfo.bCreate = bCreate;

	//DbgPrint("ProcessCallback\n");
	GetPathName(hProcessId, extension->ProcInfo.PathName);

	if (bCreate) {
		DbgPrint("PID : 0x%x %S Create \n", hProcessId, extension->ProcInfo.PathName);
	}
	else {
		DbgPrint("PID : 0x%x %S Delete \n", hProcessId, extension->ProcInfo.PathName);
	}

	KeSetEvent(pEvent, IO_NO_INCREMENT, FALSE);
	KeClearEvent(pEvent);
}
