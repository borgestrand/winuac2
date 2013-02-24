/* hello.c
 *
 * Copyright (C) Borge Strand-Bergesen 
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Please refer to http://
 *
 */

#include <ntddk.h>
 
void DriverUnload(PDRIVER_OBJECT pDriverObject) {
	DbgPrint("hello.sys: Goodbye world\n");
}
 
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	DRIVER_UNLOAD DriverUnload;
	DriverObject->DriverUnload = DriverUnload;

	DbgPrint("hello.sys: Hello world\n");
	return STATUS_SUCCESS;
}

