#include "StdAfx.h"
#include "Disk.h"
#include <SetupAPI.h>
#pragma comment(lib,"setupapi.lib")

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
		ullSumSectors = (ULONGLONG)(pdg_ex.DiskSize.QuadPart / BYTES_PER_SECTOR);
		if (pdg_ex.DiskSize.QuadPart % BYTES_PER_SECTOR != 0)
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

	ULONGLONG ullFileSectors = (ULONGLONG)(largeFileSize.QuadPart / BYTES_PER_SECTOR);

	if (largeFileSize.QuadPart % BYTES_PER_SECTOR != 0)
	{
		ullFileSectors++;
	}

	return ullFileSectors;
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

BOOL GetStorageDeviceNumber( HANDLE hStorageDevice,PSTORAGE_DEVICE_NUMBER pStorageDevNumber )
{
	DWORD                   dwOutBytes;
	BOOL                    bResult;

	VOLUME_DISK_EXTENTS		DiskExtents;

	if(INVALID_HANDLE_VALUE == hStorageDevice)
	{
		return FALSE;
	}

	////使用IOCTL_STORAGE_GET_DEVICE_NUMBER
	bResult = DeviceIoControl(hStorageDevice,
		IOCTL_STORAGE_GET_DEVICE_NUMBER ,
		NULL,
		0,
		pStorageDevNumber,
		sizeof(STORAGE_DEVICE_NUMBER),
		&dwOutBytes,
		(LPOVERLAPPED) NULL);

	if(FALSE == bResult)
	{
		//IOCTL_STORAGE_GET_DEVICE_NUMBER 失败
		bResult = DeviceIoControl(	hStorageDevice,
			IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS ,
			NULL,
			0,
			&DiskExtents,
			sizeof(VOLUME_DISK_EXTENTS),
			&dwOutBytes,
			(LPOVERLAPPED) NULL);

		if(FALSE == bResult)
		{
			DWORD dwErrorCode = GetLastError();
			TCHAR szErrorMsg[1024] = {NULL};
			_stprintf_s(szErrorMsg,_T("GetStorageDeviceNumber - DeviceIoControl(%d)"),dwErrorCode);
			OutputDebugString(szErrorMsg);

			return FALSE;
		}

		//复制DiskExtents信息 到 pStorageDevNumber
		if(0 != DiskExtents.NumberOfDiskExtents)
		{
			pStorageDevNumber->DeviceNumber = DiskExtents.Extents[0].DiskNumber;
		}
	}

	return TRUE;
}

HANDLE GetHandleOnPhysicalDrive( int iDiskNumber,DWORD *pdwErrorCode )
{
	TCHAR szPhysicalDrive[20] = {NULL};

	_stprintf_s(szPhysicalDrive,_T("\\\\.\\PhysicalDrive%d"),iDiskNumber);

	HANDLE hDisk = CreateFile(szPhysicalDrive,
		GENERIC_READ | GENERIC_WRITE ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL);

	*pdwErrorCode = GetLastError();

	return hDisk;
}

#define INTERFACE_DETAIL_SIZE 1024

int EnumDisk( int *disks )
{
	HDEVINFO                            hDevInfo;
	SP_DEVINFO_DATA                     spDevInfoData;
	SP_DEVICE_INTERFACE_DATA            spDevInterfaceData = {0};
	PSP_DEVICE_INTERFACE_DETAIL_DATA    pspDevInterfaceDetail = NULL;

	STORAGE_DEVICE_NUMBER   StorageDevNumber;
	HANDLE                  hStorageDevice;
	CString                 strDiskDescrip;

	int     nDevice;
	int     nIndex;

	// 取得一个该GUID相关的设备信息集句柄
	hDevInfo = SetupDiGetClassDevs((LPGUID)&GUID_DEVINTERFACE_DISK,             // class GUID
		NULL,                                        // 无关键字
		NULL,                                        // 不指定父窗口句柄
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);      // 目前存在的设备

	// 失败...
	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		//::AfxMessageBox(_T("SetupDiGetClassDevs FAIL!"));
		OutputDebugString(_T("EnumStorage - SetupDiGetClassDevs"));
		return 0;
	}

	//扫描所有的设备，筛选出符合条件的
	pspDevInterfaceDetail =(PSP_DEVICE_INTERFACE_DETAIL_DATA) LocalAlloc(LPTR,INTERFACE_DETAIL_SIZE * sizeof(TCHAR));
	pspDevInterfaceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

	nIndex = 0;
	nDevice = 0;

	while (1)
	{
		spDevInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		spDevInfoData.cbSize = sizeof (SP_DEVINFO_DATA);
		//获取DevInfoData
		if(SetupDiEnumDeviceInfo(hDevInfo,nIndex,&spDevInfoData))
		{

			//获取InterfaceData
			if (SetupDiEnumDeviceInterfaces(hDevInfo,                               // 设备信息集句柄
				NULL,                                   // 不需额外的设备描述
				(LPGUID)&GUID_DEVINTERFACE_DISK,        // GUID
				(ULONG) nIndex,                         // 设备信息集里的设备序号
				&spDevInterfaceData))                   // 设备接口信息)
			{
				//获取InterfaceDetail
				if (SetupDiGetInterfaceDeviceDetail(hDevInfo,                       // 设备信息集句柄
					&spDevInterfaceData,            // 设备接口信息
					pspDevInterfaceDetail,          // 设备接口细节(设备路径)
					INTERFACE_DETAIL_SIZE,          // 输出缓冲区大小
					NULL,                           // 不需计算输出缓冲区大小(直接用设定值)
					NULL))                          // 不需额外的设备描述
				{
					//检测当前设备是否是USBSTOR设备
					if (_tcsstr(pspDevInterfaceDetail->DevicePath,
						_T("usbstor")))
					{
						//是Usb Storage设备

						//获取Sotrage设备的DeviceNumber
						//DeviceNumber作为不同存储设备的索引，以此可以根据DeviceNumber检索到盘符
						hStorageDevice = CreateFile(pspDevInterfaceDetail->DevicePath,                 // 设备路径
							GENERIC_READ | GENERIC_WRITE,                      // 读写方式
							FILE_SHARE_READ | FILE_SHARE_WRITE,                // 共享方式
							NULL,                                              // 默认的安全描述符
							OPEN_EXISTING,                                     // 创建方式
							0,                                                 // 不需设置文件属性
							NULL);                                             // 不需参照模板文件
						if(INVALID_HANDLE_VALUE == hStorageDevice)
						{
							DWORD dwErrorCode = GetLastError();
							TCHAR szErrorMsg[1024] = {NULL};
							_stprintf_s(szErrorMsg,_T("EnumStorage - CreateFile(%d)"),dwErrorCode);
							OutputDebugString(szErrorMsg);
							nIndex += 1;
							continue;           //检索下一个storage设备
						}
						//获取DeviceNumber
						if(FALSE == GetStorageDeviceNumber(hStorageDevice,&StorageDevNumber))
						{
							//DeviceNumber获取失败
							OutputDebugString(_T("EnumStorage - GetStorageDeviceNumber"));
							CloseHandle(hStorageDevice);
							nIndex += 1;
							continue;           //检索下一个storage设备
						}

						disks[nDevice++] = StorageDevNumber.DeviceNumber;

						CloseHandle(hStorageDevice);
					}
				}
			}

			nIndex += 1;
		}
		else
		{
			OutputDebugString(_T("EnumStorage - SetupDiEnumDeviceInfo"));
			break;
		}

	}

	LocalFree(pspDevInterfaceDetail);
	SetupDiDestroyDeviceInfoList (hDevInfo);

	return nDevice;
}

BOOL GetDriveProperty( HANDLE hDevice, PSTORAGE_DEVICE_DESCRIPTOR pDevDesc )
{
	STORAGE_PROPERTY_QUERY Query; // 查询输入参数
	DWORD dwOutBytes; // IOCTL输出数据长度
	BOOL bResult; // IOCTL返回值

	// 指定查询方式
	Query.PropertyId = StorageDeviceProperty;
	Query.QueryType = PropertyStandardQuery;

	// 用IOCTL_STORAGE_QUERY_PROPERTY取设备属性信息
	bResult = ::DeviceIoControl(hDevice, // 设备句柄
		IOCTL_STORAGE_QUERY_PROPERTY, // 取设备属性信息
		&Query, sizeof(STORAGE_PROPERTY_QUERY), // 输入数据缓冲区
		pDevDesc, pDevDesc->Size, // 输出数据缓冲区
		&dwOutBytes, // 输出数据长度
		(LPOVERLAPPED)NULL); // 用同步I/O    

	return bResult;
}
