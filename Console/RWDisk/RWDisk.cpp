// RWDisk.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Windows.h>
#include <assert.h>
#include "ConsoleColor.h"
#include <fstream>
#include <strsafe.h>
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
BOOL ReadDiskToFile(HANDLE hDevice,HANDLE hFile,ULONGLONG ullStartSector, ULONGLONG ullSectors);
BOOL WriteDiskFromFile(HANDLE hDevice,HANDLE hFile,ULONGLONG ullStartSector, ULONGLONG ullSectors);
BOOL VerifyDiskAndFile(HANDLE hDevice,HANDLE hFile,ULONGLONG ullStartSector, ULONGLONG ullSectors);
BOOL EraseDisk(HANDLE hDevice,ULONGLONG ullStartSector, ULONGLONG ullSectors);

void Usage();
void ValidateArgs(int argc, TCHAR **argv);

BOOL SetDiskAtrribute(HANDLE hDisk,BOOL bReadOnly,BOOL bOffline,PDWORD pdwErrorCode);


TCHAR g_szDeviceName[255] = {NULL};
TCHAR g_szFileName[1024] = _T("phiyo.bin");
BOOL  g_bRead = FALSE;
BOOL  g_bWrite = FALSE;
BOOL  g_bVerify = FALSE;
BOOL  g_bErase = FALSE;
ULONGLONG g_ullStartSector = 0;
ULONGLONG g_ullSectors = -1;
int   g_nDiskNum = 0;
OVERLAPPED g_OverlapedDisk;
OVERLAPPED g_OverlapedFile;

int _tmain(int argc, _TCHAR* argv[])
{
	ValidateArgs(argc,argv);

	HANDLE hDisk = CreateFile(g_szDeviceName,
								GENERIC_READ | GENERIC_WRITE,
								FILE_SHARE_WRITE|FILE_SHARE_READ,
								NULL,
								OPEN_EXISTING,
								FILE_FLAG_OVERLAPPED,
								NULL);

	if (hDisk == INVALID_HANDLE_VALUE)
	{
		DWORD dwError = GetLastError();
		std::cout << red << "Open Disk Error---" << dwError << white << std::endl;
		return 2;
	}

	HANDLE hFile = INVALID_HANDLE_VALUE;

	if (g_bRead)
	{
		hFile = CreateFile(g_szFileName,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_WRITE|FILE_SHARE_READ,
			NULL,
			CREATE_ALWAYS,
			FILE_FLAG_OVERLAPPED,
			NULL);
	}
	else if (g_bWrite)
	{
		hFile = CreateFile(g_szFileName,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_WRITE|FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED,
			NULL);

		DWORD dwErrorCode = 0;
		SetDiskAtrribute(hDisk,FALSE,TRUE,&dwErrorCode);
	}
	else if (g_bVerify)
	{
		hFile = CreateFile(g_szFileName,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_WRITE|FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED,
			NULL);
	}
	else if (g_bErase)
	{

	}
	else
	{
		CloseHandle(hDisk);

		Usage();
		return 2;
	}

	if (!g_bErase)
	{
		if (hFile == INVALID_HANDLE_VALUE)
		{
			DWORD dwError = GetLastError();
			std::cout << red << "Open File Error---" << dwError << white << std::endl;
			CloseHandle(hDisk);
			return 2;
		}
	}

	memset(&g_OverlapedDisk,0,sizeof(g_OverlapedDisk));
	memset(&g_OverlapedFile,0,sizeof(g_OverlapedFile));

	g_OverlapedDisk.hEvent = CreateEvent(NULL,FALSE,TRUE,NULL);
	g_OverlapedFile.hEvent = CreateEvent(NULL,FALSE,TRUE,NULL);

	ULONGLONG ullSumSectors = GetDiskSectors(hDisk);

	BOOL bOK = TRUE;

	if (g_bRead)
	{
		if ((g_ullSectors == -1) || ((g_ullSectors + g_ullStartSector) > ullSumSectors))
		{
			g_ullSectors = ullSumSectors - g_ullStartSector;
		}

		std::cout << yellow << "Running --- Read Disk to File ......" << white << std::endl;

		bOK = ReadDiskToFile(hDisk,hFile,g_ullStartSector,g_ullSectors);

		if (bOK)
		{
			std::cout << green << "Read Disk to File --- PASS, PASS, PASS !" << white << std::endl;
		}

	}
	else if (g_bWrite)
	{
		ULONGLONG ullFileSectors = GetFileSectors(hFile);

		if (ullSumSectors> ullFileSectors)
		{
			ullSumSectors = ullFileSectors;
		}

		std::cout << yellow << "Running --- Write Disk from File ......" << white << std::endl;

		if ((g_ullSectors == -1) || ((g_ullSectors + g_ullStartSector) > ullSumSectors) )
		{
			g_ullSectors = ullSumSectors - g_ullStartSector;
		}

		bOK = WriteDiskFromFile(hDisk,hFile,g_ullStartSector,g_ullSectors);

		if (bOK)
		{
			std::cout << green << "Write Disk from File --- PASS, PASS, PASS !" << white << std::endl;
		}
	}
	else if (g_bVerify)
	{
		ULONGLONG ullFileSectors = GetFileSectors(hFile);

		if (ullSumSectors> ullFileSectors)
		{
			ullSumSectors = ullFileSectors;
		}

		if ((g_ullSectors == -1) || ((g_ullSectors + g_ullStartSector) > ullSumSectors) )
		{
			g_ullSectors = ullSumSectors - g_ullStartSector;
		}

		std::cout << yellow << "Running --- Verify Disk and File ......" << white << std::endl;
		bOK = VerifyDiskAndFile(hDisk,hFile,g_ullStartSector,g_ullSectors);

		if (bOK)
		{
			std::cout << green << "Verify Disk and File --- PASS, PASS, PASS !" << white << std::endl;
		}

	}
	else if (g_bErase)
	{
		std::cout << yellow << "Running --- Erase Disk ......" << white << std::endl;

		if ((g_ullSectors == -1) || ((g_ullSectors + g_ullStartSector) > ullSumSectors) )
		{
			g_ullSectors = ullSumSectors - g_ullStartSector;
		}

		bOK = EraseDisk(hDisk,g_ullStartSector,g_ullSectors);

		if (bOK)
		{
			std::cout << green << "Write Disk from File --- PASS, PASS, PASS !" << white << std::endl;
		}
	}
	else
	{
		bOK = FALSE;
		Usage();
	}

	if (g_bVerify && (g_bWrite || g_bRead ) && bOK && !g_bErase)
	{
		std::cout << yellow << "Running --- Verify Disk and File ......" << white << std::endl;
		bOK = VerifyDiskAndFile(hDisk,hFile,g_ullStartSector,g_ullSectors);

		if (bOK)
		{
			std::cout << green << "Verify Disk and File --- PASS, PASS, PASS !" << white << std::endl;
		}
	}

	CloseHandle(hFile);
	CloseHandle(hDisk);

	if (g_OverlapedDisk.hEvent != NULL)
	{
		CloseHandle(g_OverlapedDisk.hEvent);
	}

	if (g_OverlapedFile.hEvent != NULL)
	{
		CloseHandle(g_OverlapedFile.hEvent);
	}

	if (bOK)
	{
		std::cout << green << "Operation Success !!!" << white << std::endl;
	}

	std::cout << std::endl;

	return 0;
}

BOOL ReadSectors( HANDLE hDevice, ULONGLONG ullStartSector, DWORD dwSectors, DWORD dwBytesPerSector, LPBYTE lpSectBuff, LPOVERLAPPED lpOverlap, DWORD *pdwErrorCode )
{
	ULONGLONG ullOffset = ullStartSector * dwBytesPerSector;
	DWORD dwLen = dwSectors * dwBytesPerSector;
	DWORD dwReadLen = 0;
	DWORD dwErrorCode = 0;

	if (lpOverlap)
	{
		lpOverlap->Offset = LODWORD(ullOffset);
		lpOverlap->OffsetHigh = HIDWORD(ullOffset);
	}
	else
	{
		LARGE_INTEGER liFileSize = {0};
		liFileSize.QuadPart = (LONGLONG)ullOffset;

		if (!SetFilePointerEx(hDevice,liFileSize,NULL,FILE_BEGIN))
		{
			*pdwErrorCode = GetLastError();
			return FALSE;
		}
	}

	if (!ReadFile(hDevice,lpSectBuff,dwLen,&dwReadLen,lpOverlap))
	{
		dwErrorCode = ::GetLastError();

		if(dwErrorCode == ERROR_IO_PENDING) // 结束异步I/O
		{
			if (WaitForSingleObject(lpOverlap->hEvent, INFINITE) != WAIT_FAILED)
			{
				if(!::GetOverlappedResult(hDevice, lpOverlap, &dwReadLen, FALSE))
				{
					*pdwErrorCode = ::GetLastError();
					return FALSE;
				}
				else
				{
					return TRUE;
				}
			}
			else
			{
				*pdwErrorCode = ::GetLastError();
				return FALSE;
			}

		}
		else
		{
			*pdwErrorCode = dwErrorCode;
			return FALSE;
		}
	}
	else
	{
		return TRUE;
	}
}

BOOL WriteSectors( HANDLE hDevice, ULONGLONG ullStartSector, DWORD dwSectors, DWORD dwBytesPerSector, LPBYTE lpSectBuff, LPOVERLAPPED lpOverlap, DWORD *pdwErrorCode )
{
	ULONGLONG ullOffset = ullStartSector * dwBytesPerSector;
	DWORD dwLen = dwSectors * dwBytesPerSector;
	DWORD dwWriteLen = 0;
	DWORD dwErrorCode = 0;

	if (lpOverlap)
	{
		lpOverlap->Offset = LODWORD(ullOffset);
		lpOverlap->OffsetHigh = HIDWORD(ullOffset);
	}
	else
	{
		LARGE_INTEGER liFileSize = {0};
		liFileSize.QuadPart = (LONGLONG)ullOffset;

		if (!SetFilePointerEx(hDevice,liFileSize,NULL,FILE_BEGIN))
		{
			*pdwErrorCode = GetLastError();
			return FALSE;
		}
	}


	if (!WriteFile(hDevice,lpSectBuff,dwLen,&dwWriteLen,lpOverlap))
	{
		dwErrorCode = ::GetLastError();

		if(dwErrorCode == ERROR_IO_PENDING) // 结束异步I/O
		{
			if (WaitForSingleObject(lpOverlap->hEvent, INFINITE) != WAIT_FAILED)
			{
				if(!::GetOverlappedResult(hDevice, lpOverlap, &dwWriteLen, FALSE))
				{
					*pdwErrorCode = ::GetLastError();
					return FALSE;
				}
				else
				{
					return TRUE;
				}
			}
			else
			{
				*pdwErrorCode = ::GetLastError();
				return FALSE;
			}

		}
		else
		{
			*pdwErrorCode = dwErrorCode;
			return FALSE;
		}
	}
	else
	{
		return TRUE;
	}
}

void Usage()
{
	std::cout << blue << "Usage: RWDisk.exe -d:disknumber -r[:startsector-sectors] -w[:startsector-sectors] -f:filename -v -e" << std::endl;
	std::cout << "       -d:disknumber\t\t ---- disk number"  << std::endl;
	std::cout << "       -r[:startsector-sectors]\t ---- read disk sectors"  << std::endl;
	std::cout << "       -w[:startsector-sectors]\t ---- write disk sectors"  << std::endl;
	std::cout << "       -f:filename\t\t ---- read or write file name"  << std::endl;
	std::cout << "       -v\t\t\t ---- verify disk and file"  << std::endl;
	std::cout << "       -e\t\t\t ---- erase disk" << std::endl;

	std::cout << white << std::endl;

	exit(1);
}

void ValidateArgs( int argc, TCHAR **argv )
{
	if (argc == 1)
	{
		Usage();
	}
	else
	{
		for (int i = 1;i < argc;i++)
		{
			if (argv[i][0] == _T('-') || argv[i][0] == _T('/'))
			{
				TCHAR tcFlag = argv[i][1];
				switch (tolower(tcFlag))
				{
				case _T('d'):
					if (_tcslen(argv[i]) > 3)
					{
						_tcscpy(g_szDeviceName,_T("\\\\.\\PhysicalDrive"));
						//_tcscpy(g_szDeviceName,_T("\\\\.\\"));
						_tcscat(g_szDeviceName,&argv[i][3]);
						g_nDiskNum = _ttoi64(&argv[i][3]);
					}
					break;

				case _T('r'):
					g_bRead = TRUE;
					if (_tcslen(argv[i]) > 3)
					{
						g_ullStartSector = _ttoi64(&argv[i][3]);

						TCHAR *pszSectors = _tcsrchr(argv[i],_T('-'));

						if (pszSectors != NULL && _tcslen(pszSectors) > 1)
						{
							g_ullSectors = _ttoi64(pszSectors+1);
						}
					}
					break;

				case _T('w'):
					g_bWrite = TRUE;
					if (_tcslen(argv[i]) > 3)
					{
						g_ullStartSector = _ttoi64(&argv[i][3]);

						TCHAR *pszSectors = _tcschr(argv[i],_T('-'));

						if (pszSectors != NULL)
						{
							g_ullSectors = _ttoi64(pszSectors+1);
						}
					}
					break;

				case _T('f'):
					if (_tcslen(argv[i]) > 3)
					{
						memset(g_szFileName,0,sizeof(g_szFileName));
						_tcscpy(g_szFileName,&argv[i][3]);
					}
					break;

				case _T('v'):
					g_bVerify = TRUE;
					break;

				case _T('e'):
					g_bErase = TRUE;
					break;

				default:
					Usage();
					break;

				}
			}
		}
	}
	
}

ULONGLONG GetDiskSectors( HANDLE hDevice )
{
	DISK_GEOMETRY_EX pdg_ex;
	DWORD dwReturn=0;
	ULONGLONG ullSumSectors = 0;
	if (DeviceIoControl(hDevice,
		IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
		NULL, 
		0,
		&pdg_ex, 
		sizeof(pdg_ex),
		&dwReturn,
		(LPOVERLAPPED)NULL
		))
	{
		ullSumSectors = (ULONGLONG)(pdg_ex.DiskSize.QuadPart / 512);
		if (pdg_ex.DiskSize.QuadPart % 512 != 0)
		{
			ullSumSectors++;
		}
	}

	return ullSumSectors;
}

ULONGLONG GetFileSectors( HANDLE hFile )
{
	LARGE_INTEGER largeFileSize;
	GetFileSizeEx(hFile,&largeFileSize);

	ULONGLONG ullFileSectors = (ULONGLONG)(largeFileSize.QuadPart / 512);

	if (largeFileSize.QuadPart % 512 != 0)
	{
		ullFileSectors++;
	}

	return ullFileSectors;
}

BOOL ReadDiskToFile( HANDLE hDevice,HANDLE hFile,ULONGLONG ullStartSector, ULONGLONG ullSectors )
{
	ULONGLONG ullRealStartSector = ullStartSector;
	ULONGLONG ullReadSectors = 0;
	ULONGLONG ullRemainSectors = ullSectors;
	DWORD dwSectors = 0;
	BOOL  bOK = TRUE;
	ULONGLONG ullOffset = 0;

	PBYTE lpBuf = new BYTE[BUF_LENGTH];
	memset(lpBuf,0,BUF_LENGTH);

	while (ullReadSectors < ullSectors)
	{
		if (ullRemainSectors < PER_SECTORS)
		{
			dwSectors = (DWORD)ullRemainSectors;
		}
		else
		{
			dwSectors = PER_SECTORS;
		}
		DWORD dwLastError = 0;
		memset(lpBuf,0,BUF_LENGTH);
		BOOL bRec = ReadSectors(hDevice,ullRealStartSector,dwSectors,BYTES_PER_SECTOR,lpBuf,&g_OverlapedDisk,&dwLastError);

		if (!bRec)
		{
			bOK = FALSE;
			std::cout << red << "Read Sectors Error---" << dwLastError << white << std::endl;
			break;
		}
		DWORD dwSize = dwSectors * BYTES_PER_SECTOR;
		bRec = WriteFileAsyn(hFile,ullOffset,dwSize,lpBuf,&g_OverlapedFile,&dwLastError);

		if (!bRec)
		{
			bOK = FALSE;
			std::cout << red << "Write File Error---" << dwLastError << white << std::endl;
			break;
		}

		ullRealStartSector += dwSectors;
		ullReadSectors += dwSectors;
		ullRemainSectors -= dwSectors;
		ullOffset += dwSize;
	}

	delete[] lpBuf;

	return bOK;
}

BOOL WriteDiskFromFile( HANDLE hDevice,HANDLE hFile,ULONGLONG ullStartSector, ULONGLONG ullSectors )
{
	ULONGLONG ullRealStartSector = ullStartSector;
	ULONGLONG ullReadSectors = 0;
	ULONGLONG ullRemainSectors = ullSectors;
	DWORD dwSectors = 0;
	BOOL  bOK = TRUE;
	ULONGLONG ullOffset = 0;

	PBYTE lpBuf = new BYTE[BUF_LENGTH];
	memset(lpBuf,0,BUF_LENGTH);

	while (ullReadSectors < ullSectors)
	{
		if (ullRemainSectors < PER_SECTORS)
		{
			dwSectors = (DWORD)ullRemainSectors;
		}
		else
		{
			dwSectors = PER_SECTORS;
		}
		DWORD dwLastError = 0;
		memset(lpBuf,0,BUF_LENGTH);

		DWORD dwSize = dwSectors * BYTES_PER_SECTOR;
		BOOL bRec = ReadFileAsyn(hFile,ullOffset,dwSize,lpBuf,&g_OverlapedFile,&dwLastError);

		if (!bRec)
		{
			bOK = FALSE;
			std::cout << red << "Read File Error---" << dwLastError << white << std::endl;
			break;
		}

		bRec = WriteSectors(hDevice,ullRealStartSector,dwSectors,BYTES_PER_SECTOR,lpBuf,&g_OverlapedDisk,&dwLastError);

		if (!bRec)
		{
			bOK = FALSE;
			std::cout << red << "Write Sectors Error---" << dwLastError << white << std::endl;
			break;
		}

		ullRealStartSector += dwSectors;
		ullReadSectors += dwSectors;
		ullRemainSectors -= dwSectors;
		ullOffset += dwSize;
	}

	delete[] lpBuf;

	return bOK;
}

BOOL VerifyDiskAndFile( HANDLE hDevice,HANDLE hFile,ULONGLONG ullStartSector, ULONGLONG ullSectors )
{
	ULONGLONG ullRealStartSector = ullStartSector;
	ULONGLONG ullReadSectors = 0;
	ULONGLONG ullRemainSectors = ullSectors;
	DWORD dwSectors = 0;
	BOOL  bOK = TRUE;
	ULONGLONG ullOffet = 0;

	PBYTE lpBufDisk = new BYTE[BUF_LENGTH];
	memset(lpBufDisk,0,BUF_LENGTH);

	PBYTE lpBufFile = new BYTE[BUF_LENGTH];
	memset(lpBufFile,0,BUF_LENGTH);

	while (ullReadSectors < ullSectors)
	{
		if (ullRemainSectors < PER_SECTORS)
		{
			dwSectors = (DWORD)ullRemainSectors;
		}
		else
		{
			dwSectors = PER_SECTORS;
		}
		DWORD dwLastError = 0;
		memset(lpBufDisk,0,BUF_LENGTH);
		BOOL bRec = ReadSectors(hDevice,ullRealStartSector,dwSectors,BYTES_PER_SECTOR,lpBufDisk,&g_OverlapedDisk,&dwLastError);

		if (!bRec)
		{
			bOK = FALSE;
			std::cout << red << "Read Sectors Error---" << dwLastError << white << std::endl;
			break;
		}

		memset(lpBufFile,0,BUF_LENGTH);

		DWORD dwSize = dwSectors * BYTES_PER_SECTOR;
		bRec = ReadFileAsyn(hFile,ullOffet,dwSize,lpBufFile,&g_OverlapedFile,&dwLastError);

		if (!bRec)
		{
			bOK = FALSE;
			std::cout << red << "Read File Error---" << dwLastError << white << std::endl;
			break;
		}

		for (DWORD i = 0;i < dwSize;i++)
		{
			if (lpBufDisk[i] != lpBufFile[i])
			{
				bOK = FALSE;
				std::cout << red << "Verify Disk and File Error(offset=" << std::hex << ullOffet + i << ")!" << white << std::endl;
				break;
			}
		}

		if (!bOK)
		{
			break;
		}

		ullRealStartSector += dwSectors;
		ullReadSectors += dwSectors;
		ullRemainSectors -= dwSectors;
		ullOffet += dwSize;
	}

	delete[] lpBufDisk;

	return bOK;
}

BOOL ReadFileAsyn( HANDLE hFile, ULONGLONG ullOffset, DWORD &dwSize, LPBYTE lpBuffer, LPOVERLAPPED lpOverlap, PDWORD pdwErrorCode )
{
	DWORD dwReadLen = 0;
	DWORD dwErrorCode = 0;

	if (lpOverlap)
	{
		lpOverlap->Offset = LODWORD(ullOffset);
		lpOverlap->OffsetHigh = HIDWORD(ullOffset);
	}
	else
	{
		LARGE_INTEGER liFileSize = {0};
		liFileSize.QuadPart = (LONGLONG)ullOffset;

		if (!SetFilePointerEx(hFile,liFileSize,NULL,FILE_BEGIN))
		{
			*pdwErrorCode = GetLastError();
			return FALSE;
		}
	}

	if (!ReadFile(hFile,lpBuffer,dwSize,&dwReadLen,lpOverlap))
	{
		dwErrorCode = ::GetLastError();

		if(dwErrorCode == ERROR_IO_PENDING) // 结束异步I/O
		{
			if (WaitForSingleObject(lpOverlap->hEvent, INFINITE) != WAIT_FAILED)
			{
				if(!::GetOverlappedResult(hFile, lpOverlap, &dwReadLen, FALSE))
				{
					*pdwErrorCode = ::GetLastError();
					return FALSE;
				}
				else
				{
					dwSize = dwReadLen;
					return TRUE;
				}
			}
			else
			{
				*pdwErrorCode = ::GetLastError();
				return FALSE;
			}

		}
		else
		{
			*pdwErrorCode = dwErrorCode;
			return FALSE;
		}
	}
	else
	{
		dwSize = dwReadLen;
		return TRUE;
	}
}

BOOL WriteFileAsyn( HANDLE hFile, ULONGLONG ullOffset, DWORD &dwSize, LPBYTE lpBuffer, LPOVERLAPPED lpOverlap, PDWORD pdwErrorCode )
{
	DWORD dwWriteLen = 0;
	DWORD dwErrorCode = 0;

	if (lpOverlap)
	{
		lpOverlap->Offset = LODWORD(ullOffset);
		lpOverlap->OffsetHigh = HIDWORD(ullOffset);
	}
	else
	{
		LARGE_INTEGER liFileSize = {0};
		liFileSize.QuadPart = (LONGLONG)ullOffset;

		if (!SetFilePointerEx(hFile,liFileSize,NULL,FILE_BEGIN))
		{
			*pdwErrorCode = GetLastError();
			return FALSE;
		}
	}

	if (!WriteFile(hFile,lpBuffer,dwSize,&dwWriteLen,lpOverlap))
	{
		dwErrorCode = ::GetLastError();

		if(dwErrorCode == ERROR_IO_PENDING) // 结束异步I/O
		{
			if (WaitForSingleObject(lpOverlap->hEvent, INFINITE) != WAIT_FAILED)
			{
				if(!::GetOverlappedResult(hFile, lpOverlap, &dwWriteLen, FALSE))
				{
					*pdwErrorCode = ::GetLastError();
					return FALSE;
				}
				else
				{
					dwSize = dwWriteLen;
					return TRUE;
				}
			}
			else
			{
				*pdwErrorCode = ::GetLastError();
				return FALSE;
			}

		}
		else
		{
			*pdwErrorCode = dwErrorCode;
			return FALSE;
		}
	}
	else
	{
		dwSize = dwWriteLen;
		return TRUE;
	}
}

BOOL SetDiskAtrribute( HANDLE hDisk,BOOL bReadOnly,BOOL bOffline,PDWORD pdwErrorCode )
{
	BOOL bSuc;
	GET_DISK_ATTRIBUTES getAttr;
	SET_DISK_ATTRIBUTES setAttr;
	DWORD dwRet = 0;
	memset(&getAttr, 0, sizeof(GET_DISK_ATTRIBUTES));
	memset(&setAttr, 0, sizeof(SET_DISK_ATTRIBUTES));

	bSuc = DeviceIoControl(hDisk, IOCTL_DISK_GET_DISK_ATTRIBUTES, NULL, 0,
		&getAttr, sizeof(GET_DISK_ATTRIBUTES), &dwRet, NULL);

	if (bSuc)
	{
		if (bOffline)
		{
			if (!(getAttr.Attributes & DISK_ATTRIBUTE_OFFLINE)) 
			{
				setAttr.Attributes |= DISK_ATTRIBUTE_OFFLINE;
				setAttr.AttributesMask |= DISK_ATTRIBUTE_OFFLINE;
			}
		}
		else
		{
			if (getAttr.Attributes & DISK_ATTRIBUTE_OFFLINE) 
			{
				setAttr.Attributes &= ~DISK_ATTRIBUTE_OFFLINE;
				setAttr.AttributesMask |= DISK_ATTRIBUTE_OFFLINE;
			}
		}


		if (bReadOnly)
		{
			if (!(getAttr.Attributes & DISK_ATTRIBUTE_READ_ONLY))
			{
				setAttr.Attributes |= DISK_ATTRIBUTE_READ_ONLY;
				setAttr.AttributesMask |= DISK_ATTRIBUTE_READ_ONLY;
			}
		}
		else
		{
			if (getAttr.Attributes & DISK_ATTRIBUTE_READ_ONLY)
			{
				setAttr.Attributes &= ~DISK_ATTRIBUTE_READ_ONLY;
				setAttr.AttributesMask |= DISK_ATTRIBUTE_READ_ONLY;
			}
		}


		if (setAttr.AttributesMask)
		{
			setAttr.Version = sizeof(SET_DISK_ATTRIBUTES);
			bSuc = DeviceIoControl(hDisk, IOCTL_DISK_SET_DISK_ATTRIBUTES, &setAttr, sizeof(SET_DISK_ATTRIBUTES),NULL, 0,
				&dwRet, NULL);

			if (!bSuc)
			{

				for (int nRetry = 0; nRetry < 5; nRetry++)
				{
					Sleep(500);
					bSuc = DeviceIoControl(hDisk, IOCTL_DISK_SET_DISK_ATTRIBUTES, &setAttr, sizeof(SET_DISK_ATTRIBUTES),NULL, 0,
						&dwRet, NULL);

					if (bSuc)
					{
						break;
					}
				}

				*pdwErrorCode = GetLastError();
			}
		}
	}

	return bSuc;
}

BOOL EraseDisk( HANDLE hDevice,ULONGLONG ullStartSector, ULONGLONG ullSectors )
{
	ULONGLONG ullRealStartSector = ullStartSector;
	ULONGLONG ullReadSectors = 0;
	ULONGLONG ullRemainSectors = ullSectors;
	DWORD dwSectors = 0;
	BOOL  bOK = TRUE;

	PBYTE lpBuf = new BYTE[BUF_LENGTH];
	memset(lpBuf,0,BUF_LENGTH);

	while (ullReadSectors < ullSectors)
	{
		if (ullRemainSectors < PER_SECTORS)
		{
			dwSectors = (DWORD)ullRemainSectors;
		}
		else
		{
			dwSectors = PER_SECTORS;
		}

		DWORD dwLastError = 0;

		BOOL bRec = WriteSectors(hDevice,ullRealStartSector,dwSectors,BYTES_PER_SECTOR,lpBuf,&g_OverlapedDisk,&dwLastError);

		if (!bRec)
		{
			bOK = FALSE;
			std::cout << red << "Write Sectors Error---" << dwLastError << white << std::endl;
			break;
		}

		ullRealStartSector += dwSectors;
		ullReadSectors += dwSectors;
		ullRemainSectors -= dwSectors;
	}

	delete[] lpBuf;

	return bOK;
}

