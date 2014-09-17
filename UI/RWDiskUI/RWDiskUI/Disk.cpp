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

		if(dwErrorCode == ERROR_IO_PENDING) // �����첽I/O
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

		if(dwErrorCode == ERROR_IO_PENDING) // �����첽I/O
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

		if(dwErrorCode == ERROR_IO_PENDING) // �����첽I/O
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

		if(dwErrorCode == ERROR_IO_PENDING) // �����첽I/O
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

	////ʹ��IOCTL_STORAGE_GET_DEVICE_NUMBER
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
		//IOCTL_STORAGE_GET_DEVICE_NUMBER ʧ��
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

		//����DiskExtents��Ϣ �� pStorageDevNumber
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

	// ȡ��һ����GUID��ص��豸��Ϣ�����
	hDevInfo = SetupDiGetClassDevs((LPGUID)&GUID_DEVINTERFACE_DISK,             // class GUID
		NULL,                                        // �޹ؼ���
		NULL,                                        // ��ָ�������ھ��
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);      // Ŀǰ���ڵ��豸

	// ʧ��...
	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		//::AfxMessageBox(_T("SetupDiGetClassDevs FAIL!"));
		OutputDebugString(_T("EnumStorage - SetupDiGetClassDevs"));
		return 0;
	}

	//ɨ�����е��豸��ɸѡ������������
	pspDevInterfaceDetail =(PSP_DEVICE_INTERFACE_DETAIL_DATA) LocalAlloc(LPTR,INTERFACE_DETAIL_SIZE * sizeof(TCHAR));
	pspDevInterfaceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

	nIndex = 0;
	nDevice = 0;

	while (1)
	{
		spDevInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		spDevInfoData.cbSize = sizeof (SP_DEVINFO_DATA);
		//��ȡDevInfoData
		if(SetupDiEnumDeviceInfo(hDevInfo,nIndex,&spDevInfoData))
		{

			//��ȡInterfaceData
			if (SetupDiEnumDeviceInterfaces(hDevInfo,                               // �豸��Ϣ�����
				NULL,                                   // ���������豸����
				(LPGUID)&GUID_DEVINTERFACE_DISK,        // GUID
				(ULONG) nIndex,                         // �豸��Ϣ������豸���
				&spDevInterfaceData))                   // �豸�ӿ���Ϣ)
			{
				//��ȡInterfaceDetail
				if (SetupDiGetInterfaceDeviceDetail(hDevInfo,                       // �豸��Ϣ�����
					&spDevInterfaceData,            // �豸�ӿ���Ϣ
					pspDevInterfaceDetail,          // �豸�ӿ�ϸ��(�豸·��)
					INTERFACE_DETAIL_SIZE,          // �����������С
					NULL,                           // ������������������С(ֱ�����趨ֵ)
					NULL))                          // ���������豸����
				{
					//��⵱ǰ�豸�Ƿ���USBSTOR�豸
					if (_tcsstr(pspDevInterfaceDetail->DevicePath,
						_T("usbstor")))
					{
						//��Usb Storage�豸

						//��ȡSotrage�豸��DeviceNumber
						//DeviceNumber��Ϊ��ͬ�洢�豸���������Դ˿��Ը���DeviceNumber�������̷�
						hStorageDevice = CreateFile(pspDevInterfaceDetail->DevicePath,                 // �豸·��
							GENERIC_READ | GENERIC_WRITE,                      // ��д��ʽ
							FILE_SHARE_READ | FILE_SHARE_WRITE,                // ����ʽ
							NULL,                                              // Ĭ�ϵİ�ȫ������
							OPEN_EXISTING,                                     // ������ʽ
							0,                                                 // ���������ļ�����
							NULL);                                             // �������ģ���ļ�
						if(INVALID_HANDLE_VALUE == hStorageDevice)
						{
							DWORD dwErrorCode = GetLastError();
							TCHAR szErrorMsg[1024] = {NULL};
							_stprintf_s(szErrorMsg,_T("EnumStorage - CreateFile(%d)"),dwErrorCode);
							OutputDebugString(szErrorMsg);
							nIndex += 1;
							continue;           //������һ��storage�豸
						}
						//��ȡDeviceNumber
						if(FALSE == GetStorageDeviceNumber(hStorageDevice,&StorageDevNumber))
						{
							//DeviceNumber��ȡʧ��
							OutputDebugString(_T("EnumStorage - GetStorageDeviceNumber"));
							CloseHandle(hStorageDevice);
							nIndex += 1;
							continue;           //������һ��storage�豸
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
	STORAGE_PROPERTY_QUERY Query; // ��ѯ�������
	DWORD dwOutBytes; // IOCTL������ݳ���
	BOOL bResult; // IOCTL����ֵ

	// ָ����ѯ��ʽ
	Query.PropertyId = StorageDeviceProperty;
	Query.QueryType = PropertyStandardQuery;

	// ��IOCTL_STORAGE_QUERY_PROPERTYȡ�豸������Ϣ
	bResult = ::DeviceIoControl(hDevice, // �豸���
		IOCTL_STORAGE_QUERY_PROPERTY, // ȡ�豸������Ϣ
		&Query, sizeof(STORAGE_PROPERTY_QUERY), // �������ݻ�����
		pDevDesc, pDevDesc->Size, // ������ݻ�����
		&dwOutBytes, // ������ݳ���
		(LPOVERLAPPED)NULL); // ��ͬ��I/O    

	return bResult;
}
