#include <ntifs.h>
#include <ntddk.h>
#include "ThreadBoosterCommon.h"

// Unload routine
void ThreadBoosterUnload(_In_ PDRIVER_OBJECT DriverObject)
{
	NTSTATUS status;
	// Delete symlink
	UNICODE_STRING symbolicLink = RTL_CONSTANT_STRING(L"\\??\\ThreadBooster");
	status = IoDeleteSymbolicLink(&symbolicLink);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("[!!!] Failed to delete symbolic link with code 0x%08X\n", status));
	}
	// Delete device object
	IoDeleteDevice(DriverObject->DeviceObject);

}

NTSTATUS ThreadBoosterCreateClose(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	// Approve the IRP request and return it.
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}
NTSTATUS ThreadBoosterDeviceControl(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	auto stack = IoGetCurrentIrpStackLocation(Irp);
	auto status = STATUS_SUCCESS;

	switch (stack->Parameters.DeviceIoControl.IoControlCode)
	{
		case IOCTL_THREAD_BOOSTER_SET_PRIORITY:
		{
			if (stack->Parameters.DeviceIoControl.InputBufferLength < sizeof(THREAD_DATA))
			{
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			PTHREAD_DATA threadData = (PTHREAD_DATA)(stack->Parameters.DeviceIoControl.Type3InputBuffer);

			// Is data actually a THREAD_DATA struct?
			if (threadData == NULL)
			{
				status = STATUS_INVALID_PARAMETER;
				break;
			}

			// Prio value valid?
			if (threadData->Priority < 1 || threadData->Priority > 31)
			{
				status = STATUS_INVALID_PARAMETER;
				break;
			}

			// Get pointer to kernel thread object by Thread ID
			PETHREAD pThread;
			status = PsLookupThreadByThreadId(ULongToHandle(threadData->ThreadId), &pThread);
			if (!NT_SUCCESS(status))
			{
				break;
			}

			// Update priority
			KeSetPriorityThread((PKTHREAD)pThread, threadData->Priority);

			// Dereference the thread, as it cannot die unless we do so.
			ObDereferenceObject(pThread);


			break;
		}
		default:
		{
			status = STATUS_INVALID_DEVICE_REQUEST;
			break;
		}
	}

	// Update Irp and return status
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

extern "C"
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
	NTSTATUS status;

	// Set unload routine
	DriverObject->DriverUnload = ThreadBoosterUnload;

	// Set dispatch routines (driver capabilities)
	// These functions should simply approve the create or close operations.
	DriverObject->MajorFunction[IRP_MJ_CREATE] = ThreadBoosterCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = ThreadBoosterCreateClose;
	// This is the DeviceIoControl driver-side major function! It should handle control codes and actual functions.
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ThreadBoosterDeviceControl;

	// Create our device object
	UNICODE_STRING deviceName = RTL_CONSTANT_STRING(L"\\Device\\ThreadBooster");
	PDEVICE_OBJECT DeviceObject;
	status = IoCreateDevice(DriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("Failed to create DeviceObject with code 0x%08X\n", status));
		return status;
	}

	// Create symbolic link for usermode callers
	UNICODE_STRING symbolicLink = RTL_CONSTANT_STRING(L"\\??\\ThreadBooster");
	status = IoCreateSymbolicLink(&symbolicLink, &deviceName);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("Failed to create symbolic link with code 0x%08X\n", status));
		IoDeleteDevice(DeviceObject);
		return status;
	}

	UNREFERENCED_PARAMETER(RegistryPath);

	return STATUS_SUCCESS;
}