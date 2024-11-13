#include <ntddk.h>



// Unload function
void SampleUnload(_In_ PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);
	KdPrint(("Sample driver unloaded!"));
}

// Driver prototype
extern "C"
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);

	// Set unload function pointer
	DriverObject->DriverUnload = SampleUnload;
	KdPrint(("Sample driver loaded!"));
	RTL_OSVERSIONINFOW SysInfo;
	RtlGetVersion(&SysInfo);
	KdPrint(("=============================================="));
	KdPrint(("Major Version : %ld", SysInfo.dwMajorVersion));
	KdPrint(("Minor Version : %ld", SysInfo.dwMinorVersion));
	KdPrint(("Build : %ld", SysInfo.dwBuildNumber));
	KdPrint(("Unload Function : %p", &SampleUnload));
	KdPrint(("=============================================="));

	return STATUS_SUCCESS;
}