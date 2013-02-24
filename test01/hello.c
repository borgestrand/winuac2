#include <ntddk.h>
 
void DriverUnload(PDRIVER_OBJECT pDriverObject)
{
    DbgPrint("hello.sys: Driver unloading\n");
}
 
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    DRIVER_UNLOAD DriverUnload;
	DriverObject->DriverUnload = DriverUnload;

    DbgPrint("hello.sys: Hello, World\n");
    return STATUS_SUCCESS;
}

