#pragma once

#include <Windows.h>
#include <WinIoCtl.h>

#define BUF_LENGTH   0x200 * 0x800 //1M
#define PER_SECTORS  0x800
#define BYTES_PER_SECTOR 0x200

#define LODWORD(_qw)    ((DWORD)(_qw))
#define HIDWORD(_qw)    ((DWORD)(((_qw) >> 32) & 0xffffffff))

//
// IOCTLs to query and modify attributes
// associated with the given disk. These
// are persisted within the registry.
//

#define IOCTL_DISK_GET_DISK_ATTRIBUTES      CTL_CODE(IOCTL_DISK_BASE, 0x003c, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_SET_DISK_ATTRIBUTES      CTL_CODE(IOCTL_DISK_BASE, 0x003d, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define DISK_ATTRIBUTE_OFFLINE              0x0000000000000001
#define DISK_ATTRIBUTE_READ_ONLY            0x0000000000000002

//
// IOCTL_DISK_GET_DISK_ATTRIBUTES
//
// Input Buffer:
//     None
//
// Output Buffer:
//     Structure of type GET_DISK_ATTRIBUTES
//

typedef struct _GET_DISK_ATTRIBUTES {

	//
	// Specifies the size of the
	// structure for versioning.
	//
	ULONG Version;

	//
	// For alignment purposes.
	//
	ULONG Reserved1;

	//
	// Specifies the attributes
	// associated with the disk.
	//
	ULONGLONG Attributes;

} GET_DISK_ATTRIBUTES, *PGET_DISK_ATTRIBUTES;

//
// IOCTL_DISK_SET_DISK_ATTRIBUTES
//
// Input Buffer:
//     Structure of type SET_DISK_ATTRIBUTES
//
// Output Buffer:
//     None
//

typedef struct _SET_DISK_ATTRIBUTES {

	//
	// Specifies the size of the
	// structure for versioning.
	//
	ULONG Version;

	//
	// Indicates whether to remember
	// these settings across reboots
	// or not.
	//
	BOOLEAN Persist;

	//
	// Indicates whether the ownership
	// taken earlier is being released.
	//
	BOOLEAN RelinquishOwnership;

	//
	// For alignment purposes.
	//
	BOOLEAN Reserved1[2];

	//
	// Specifies the new attributes.
	//
	ULONGLONG Attributes;

	//
	// Specifies the attributes
	// that are being modified.
	//
	ULONGLONG AttributesMask;

	//
	// Specifies an identifier to be
	// associated  with  the caller.
	// This setting is not persisted
	// across reboots.
	//
	GUID Owner;

} SET_DISK_ATTRIBUTES, *PSET_DISK_ATTRIBUTES;

BOOL ReadSectors( HANDLE hDevice,
	ULONGLONG ullStartSector,
	DWORD dwSectors,
	DWORD dwBytesPerSector, 
	LPBYTE lpSectBuff, 
	LPOVERLAPPED lpOverlap,
	DWORD *pdwErrorCode );

BOOL WriteSectors(HANDLE hDevice,
	ULONGLONG ullStartSector,
	DWORD dwSectors,
	DWORD dwBytesPerSector, 
	LPBYTE lpSectBuff,
	LPOVERLAPPED lpOverlap, 
	DWORD *pdwErrorCode );

BOOL ReadFileAsyn( HANDLE hFile,
	ULONGLONG ullOffset,
	DWORD &dwSize,
	LPBYTE lpBuffer,
	LPOVERLAPPED lpOverlap,
	PDWORD pdwErrorCode );

BOOL WriteFileAsyn( HANDLE hFile,
	ULONGLONG ullOffset,
	DWORD &dwSize,
	LPBYTE lpBuffer,
	LPOVERLAPPED lpOverlap,
	PDWORD pdwErrorCode );

ULONGLONG GetDiskSectors(HANDLE hDevie);
ULONGLONG GetFileSectors(HANDLE hFile);

BOOL SetDiskAtrribute(HANDLE hDisk,BOOL bReadOnly,BOOL bOffline,PDWORD pdwErrorCode);

BOOL GetStorageDeviceNumber(HANDLE hStorageDevice,PSTORAGE_DEVICE_NUMBER pStorageDevNumber);

HANDLE GetHandleOnPhysicalDrive(int iDiskNumber,DWORD *pdwErrorCode);

int EnumDisk(int *disks);

BOOL GetDriveProperty(HANDLE hDevice, PSTORAGE_DEVICE_DESCRIPTOR pDevDesc);