
// RWDiskUIDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RWDiskUI.h"
#include "RWDiskUIDlg.h"
#include "afxdialogex.h"

#include "Utils.h"
#include "Disk.h"

#include <dbt.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRWDiskUIDlg dialog




CRWDiskUIDlg::CRWDiskUIDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRWDiskUIDlg::IDD, pParent)
	, m_ullEditStartSector(0)
	, m_ullEditSectorCount(0)
	, m_bCheckVerify(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_hDisk = INVALID_HANDLE_VALUE;
	m_hFile = INVALID_HANDLE_VALUE;

	m_bStart = FALSE;

	m_ullCompletedSize = 0;
	m_ullTotalSize = 0;
	m_dbUsedTime = 0.0;

	ZeroMemory(&m_OverlapedDisk,sizeof(m_OverlapedDisk));
	ZeroMemory(&m_OverlapedFile,sizeof(m_OverlapedFile));

	m_OverlapedDisk.hEvent = CreateEvent(NULL,FALSE,TRUE,NULL);

	m_OverlapedFile.hEvent = CreateEvent(NULL,FALSE,TRUE,NULL);
}

void CRWDiskUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_DISK, m_ComboBoxDisk);
	DDX_Text(pDX, IDC_EDIT_START_SECTOR, m_ullEditStartSector);
	DDX_Text(pDX, IDC_EDIT_SECTOR_COUNT, m_ullEditSectorCount);
	DDX_Control(pDX, IDC_COMBO_FUNCTION, m_ComboBoxFunction);
	DDX_Check(pDX, IDC_CHECK_VERIFY, m_bCheckVerify);
	DDX_Control(pDX, IDC_PROGRESS1, m_ProgressCtrl);
}

BEGIN_MESSAGE_MAP(CRWDiskUIDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_START, &CRWDiskUIDlg::OnBnClickedBtnStart)
	ON_WM_TIMER()
	ON_WM_DEVICECHANGE()
	ON_CBN_SELCHANGE(IDC_COMBO_FUNCTION, &CRWDiskUIDlg::OnCbnSelchangeComboFunction)
END_MESSAGE_MAP()


// CRWDiskUIDlg message handlers

BOOL CRWDiskUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	CString strPath = CUtils::GetAppPath();
	CString strTitle;
	strTitle.Format(_T("RWDisk V%s"),CUtils::GetAppVersion(strPath));

	SetWindowText(strTitle);

	DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
	ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
	NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	NotificationFilter.dbcc_classguid = GUID_DEVINTERFACE_DISK;
	HDEVNOTIFY hDevNotify = RegisterDeviceNotification(this->GetSafeHwnd(), &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
	if( !hDevNotify ) 
	{
		AfxMessageBox(CString("Can't register device notification: ") + _com_error(GetLastError()).ErrorMessage(), MB_ICONEXCLAMATION);
	}

	// Initial Disk
	InitialComboBoxDisk();

	// Initial Fuction
	m_ComboBoxFunction.AddString(_T("Read disk to file"));
	m_ComboBoxFunction.AddString(_T("Write disk from file"));
	m_ComboBoxFunction.AddString(_T("Verify disk and file"));
	m_ComboBoxFunction.AddString(_T("Erase disk"));
	m_ComboBoxFunction.SetCurSel(0);

	m_font.CreatePointFont(120,_T("Arial"));

	m_StatusBar.Create(WS_CHILD|WS_VISIBLE|CCS_BOTTOM|SBARS_SIZEGRIP, CRect(0,0,0,0), this, IDC_MAIN_STATUSBAR);

	m_StatusBar.SetFont(&m_font);

	int nPartDim[3]= {350,450, -1};  // text,speed,time
	m_StatusBar.SetParts(3, nPartDim);

	m_ProgressCtrl.SetRange32(0,100);
	m_ProgressCtrl.ShowPercent(TRUE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRWDiskUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRWDiskUIDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRWDiskUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CRWDiskUIDlg::InitialComboBoxDisk()
{

	m_ComboBoxDisk.ResetContent();

	// 获取Disk信息
	int disks[20] = {0};

	int nDiskCount = EnumDisk(disks);

	DWORD dwErrorCode = 0;
	CString strModel,strSize,strDescription;

	for (int i = 0; i < nDiskCount;i++)
	{
		HANDLE hDisk = GetHandleOnPhysicalDrive(disks[i],&dwErrorCode);

		if (hDisk == INVALID_HANDLE_VALUE)
		{
			TRACE1("Open disk failed with error code:%d",dwErrorCode);
			continue;
		}

		// PSTORAGE_DEVICE_DESCRIPTOR 结构体的初始化
		PSTORAGE_DEVICE_DESCRIPTOR DeviceDescriptor;
		DeviceDescriptor=(PSTORAGE_DEVICE_DESCRIPTOR)new BYTE[sizeof(STORAGE_DEVICE_DESCRIPTOR) + 512 - 1];
		DeviceDescriptor->Size = sizeof(STORAGE_DEVICE_DESCRIPTOR) + 512 - 1;

		if (GetDriveProperty(hDisk,DeviceDescriptor))
		{
			LPCSTR vendorId = "";
			LPCSTR productId = "";

			LPCSTR serialNumber = "";

			if ((DeviceDescriptor->ProductIdOffset != 0) &&
				(DeviceDescriptor->ProductIdOffset != -1)) 
			{
				productId        = (LPCSTR)(DeviceDescriptor);
				productId       += (ULONG_PTR)DeviceDescriptor->ProductIdOffset;
			}

			if ((DeviceDescriptor->VendorIdOffset != 0) &&
				(DeviceDescriptor->VendorIdOffset != -1)) 
			{
				vendorId         = (LPCSTR)(DeviceDescriptor);
				vendorId        += (ULONG_PTR)DeviceDescriptor->VendorIdOffset;
			}

			strModel = CString(vendorId) + CString(productId);
			strModel.Trim();
		}

		delete []DeviceDescriptor;

		ULONGLONG ullSize = GetDiskSectors(hDisk) * BYTES_PER_SECTOR;

		strSize = CUtils::AdjustFileSize(ullSize);

		strDescription.Format(_T("HD%d:%s (%s,USB)"),disks[i],strModel,strSize);

		m_ComboBoxDisk.AddString(strDescription);
		m_ComboBoxDisk.SetItemData(i,disks[i]);

		CloseHandle(hDisk);
	}

	m_ComboBoxDisk.SetCurSel(0);
}

void CRWDiskUIDlg::OnBnClickedBtnStart()
{
	// TODO: 在此添加控件通知处理程序代码

	if (m_bStart)
	{
		//Stop
		if (m_hDisk != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_hDisk);
			m_hDisk = INVALID_HANDLE_VALUE;
		}

		if (m_hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_hFile);
			m_hFile = INVALID_HANDLE_VALUE;
		}
		m_bStart = FALSE;

		SetDlgItemText(IDC_BTN_START,_T("Start"));

		EnableControl(TRUE);

	}
	else
	{
		UpdateData(TRUE);

		// 获取磁盘
		int nSelectDiskIndex = m_ComboBoxDisk.GetCurSel();

		if (nSelectDiskIndex == -1)
		{
			CString strText;
			strText.Format(_T("No disk has been selected."));

			m_StatusBar.SetText(strText,0,0);

			MessageBox(strText);
			return;
		}

		int disk = m_ComboBoxDisk.GetItemData(nSelectDiskIndex);
		DWORD dwErrorCode = 0;
		m_hDisk = GetHandleOnPhysicalDrive(disk,&dwErrorCode);

		if (m_hDisk == INVALID_HANDLE_VALUE)
		{
			CString strText;
			strText.Format(_T("Open disk failed with error code:%d"),dwErrorCode);

			m_StatusBar.SetText(strText,0,0);

			MessageBox(strText);

			return;
		}

		ULONGLONG ullDiskSector = GetDiskSectors(m_hDisk);

		int nSelectFunction = m_ComboBoxFunction.GetCurSel();

		switch (nSelectFunction)
		{
		case OP_READ:
			{
				//需要指定保存文件
				CFileDialog dlg(FALSE,_T("bin"),_T("phiyo"),OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT
					,_T("BIN(*.bin)|*.bin|All Files(*.*)|*.*||"));

				if (dlg.DoModal() == IDCANCEL)
				{
					CloseHandle(m_hDisk);
					return;
				}

				m_strSaveFileName = dlg.GetPathName();

				m_hFile = CreateFile(m_strSaveFileName,
									 GENERIC_READ | GENERIC_WRITE,
									 FILE_SHARE_READ | FILE_SHARE_WRITE,
									 NULL,
									 CREATE_ALWAYS,
									 FILE_FLAG_OVERLAPPED,
									 NULL);

				if (m_hFile == INVALID_HANDLE_VALUE)
				{
					CString strText;
					strText.Format(_T("Create image file failed with error code:%d"),dwErrorCode);

					m_StatusBar.SetText(strText,0,0);

					MessageBox(strText);

					CloseHandle(m_hDisk);

					m_hDisk = INVALID_HANDLE_VALUE;

					return;
				}

				// 指定扇区数不能超过磁盘扇区数
				if ((m_ullEditSectorCount == 0) || ((m_ullEditSectorCount + m_ullEditStartSector) > ullDiskSector))
				{
					m_ullEditSectorCount = ullDiskSector - m_ullEditStartSector;
				}
			}

			break;

		case OP_WRITE:
		case OP_VERIFY:
			{
				//需要指定打开文件
				CFileDialog dlg(TRUE,_T("bin"),_T("phiyo"),OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT
					,_T("BIN(*.bin)|*.bin|All Files(*.*)|*.*||"));

				if (dlg.DoModal() == IDCANCEL)
				{
					CloseHandle(m_hDisk);
					m_hDisk = INVALID_HANDLE_VALUE;
					return;
				}

				m_hFile = CreateFile(dlg.GetPathName(),
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					FILE_FLAG_OVERLAPPED,
					NULL);

				if (m_hFile == INVALID_HANDLE_VALUE)
				{
					CString strText;
					strText.Format(_T("Open image file failed with error code:%d"),dwErrorCode);

					m_StatusBar.SetText(strText,0,0);

					MessageBox(strText);

					CloseHandle(m_hDisk);
					m_hDisk = INVALID_HANDLE_VALUE;
					return;
				}

				//取文件和磁盘中较小的
				ULONGLONG ullFileSectors = GetFileSectors(m_hFile);

				if (ullFileSectors == 0)
				{
					MessageBox(_T("The file size is 0."));

					m_StatusBar.SetText(_T("The file size is 0."),0,0);

					CloseHandle(m_hFile);
					CloseHandle(m_hDisk);

					m_hDisk = INVALID_HANDLE_VALUE;

					m_hFile = INVALID_HANDLE_VALUE;
					return;

				}

				if (ullDiskSector >= ullFileSectors)
				{
					ullDiskSector = ullFileSectors;
				}
				else
				{
					if (IDNO == MessageBox(_T("The image file size is larger than the disk size. Do you want to continue?")
						,_T("Warnning"),MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON1))
					{

						m_StatusBar.SetText(_T("The image file size is larger than the disk size."),0,0);

						CloseHandle(m_hFile);
						CloseHandle(m_hDisk);

						m_hDisk = INVALID_HANDLE_VALUE;

						m_hFile = INVALID_HANDLE_VALUE;
						return;
					}
				}

				// 指定扇区数不能超过磁盘扇区数
				if ((m_ullEditSectorCount == 0) || ((m_ullEditSectorCount + m_ullEditStartSector) > ullDiskSector))
				{
					m_ullEditSectorCount = ullDiskSector - m_ullEditStartSector;
				}
			}
			break;

		case OP_ERASE:
			{
				// 指定扇区数不能超过磁盘扇区数
				if ((m_ullEditSectorCount == 0) || ((m_ullEditSectorCount + m_ullEditStartSector) > ullDiskSector))
				{
					m_ullEditSectorCount = ullDiskSector - m_ullEditStartSector;
				}
			}
			break;
		}

		m_bStart = TRUE;
		SetDlgItemText(IDC_BTN_START,_T("Stop"));

		m_ullTotalSize = ullDiskSector * BYTES_PER_SECTOR;

		m_StatusBar.SetText(_T("0.00 MB/s"),1,0);
		m_StatusBar.SetText(_T("00:00:00"),2,0);

		CString strUsage;
		strUsage.Format(_T("0 / %s"),CUtils::AdjustFileSize(m_ullTotalSize));
		
		SetDlgItemText(IDC_TEXT_USAGE,strUsage);

		m_ProgressCtrl.SetPos(0);


		UpdateData(FALSE);

		if (nSelectFunction == OP_WRITE || nSelectFunction == OP_ERASE)
		{
			SetDiskAtrribute(m_hDisk,FALSE,TRUE,&dwErrorCode);
		}

		EnableControl(FALSE);

		AfxBeginThread((AFX_THREADPROC)StartProcThread,this);

	}

}


void CRWDiskUIDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	UpdateState();
	
	CDialogEx::OnTimer(nIDEvent);
}

void CRWDiskUIDlg::OnStart()
{
	int nSelectFunction = m_ComboBoxFunction.GetCurSel();

	m_ullCompletedSize = 0;
	m_dbUsedTime = 0.0;

	BOOL bOK = FALSE;

	SetTimer(1,1000,NULL);

	switch (nSelectFunction)
	{
	case OP_READ:
		m_StatusBar.SetText(_T("Read disk to file is running ......"),0,0);
		bOK = ReadDiskToFile(m_hDisk,m_hFile,m_ullEditStartSector,m_ullEditSectorCount);

		if (bOK)
		{
			m_StatusBar.SetText(_T("Read disk to file - PASS,PASS,PASS"),0,0);
		}
		else
		{
			CloseHandle(m_hFile);

			m_hFile = INVALID_HANDLE_VALUE;

			DeleteFile(m_strSaveFileName);
		}

		break;

	case OP_WRITE:

		m_StatusBar.SetText(_T("Write disk from file is running ......"),0,0);

		bOK = WriteDiskFromFile(m_hDisk,m_hFile,m_ullEditStartSector,m_ullEditSectorCount);

		if (bOK)
		{
			m_StatusBar.SetText(_T("Write disk from file - PASS,PASS,PASS"),0,0);
		}

		break;

	case OP_VERIFY:
		m_StatusBar.SetText(_T("Verify disk and file is running ......"),0,0);
		bOK = VerifyDiskAndFile(m_hDisk,m_hFile,m_ullEditStartSector,m_ullEditSectorCount);
		if (bOK)
		{
			m_StatusBar.SetText(_T("Verify disk and file - PASS,PASS,PASS"),0,0);
		}
		break;

	case OP_ERASE:
		m_StatusBar.SetText(_T("Erase disk is running ......"),0,0);
		bOK = EraseDisk(m_hDisk,m_ullEditStartSector,m_ullEditSectorCount);
		if (bOK)
		{
			m_StatusBar.SetText(_T("Erase disk - PASS,PASS,PASS"),0,0);
		}
		break;
	}

	if (m_bCheckVerify)
	{
		m_ullCompletedSize = 0;
		m_dbUsedTime = 0.0;
		m_StatusBar.SetText(_T("Verify disk and file is running ......"),0,0);

		bOK = VerifyDiskAndFile(m_hDisk,m_hFile,m_ullEditStartSector,m_ullEditSectorCount);
		if (bOK)
		{
			m_StatusBar.SetText(_T("Verify disk and file - PASS,PASS,PASS"),0,0);
		}
	}

	KillTimer(1);

	UpdateState();

	if (m_bStart)
	{
		PostMessage(WM_COMMAND,MAKEWPARAM(IDC_BTN_START,BN_CLICKED),(LPARAM)m_hWnd);
	}
	
}

BOOL CRWDiskUIDlg::ReadDiskToFile( HANDLE hDevice,HANDLE hFile,ULONGLONG ullStartSector, ULONGLONG ullSectors )
{
	ULONGLONG ullRealStartSector = ullStartSector;
	ULONGLONG ullReadSectors = 0;
	ULONGLONG ullRemainSectors = ullSectors;
	DWORD dwSectors = 0;
	BOOL  bOK = TRUE;
	ULONGLONG ullOffset = 0;

	PBYTE lpBuf = new BYTE[BUF_LENGTH];
	memset(lpBuf,0,BUF_LENGTH);

	LARGE_INTEGER frq,t1,t2;
	QueryPerformanceFrequency(&frq);

	while (ullReadSectors < ullSectors && m_bStart)
	{
		QueryPerformanceCounter(&t1);

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
		BOOL bRec = ReadSectors(hDevice,ullRealStartSector,dwSectors,BYTES_PER_SECTOR,lpBuf,&m_OverlapedDisk,&dwLastError);

		if (!bRec)
		{
			bOK = FALSE;
			
			CString strText;
			strText.Format(_T("Read disk failed with read sector error code:%d"),dwLastError);

			m_StatusBar.SetText(strText,0,0);

			break;
		}
		DWORD dwSize = dwSectors * BYTES_PER_SECTOR;
		bRec = WriteFileAsyn(hFile,ullOffset,dwSize,lpBuf,&m_OverlapedFile,&dwLastError);

		if (!bRec)
		{
			bOK = FALSE;
			
			CString strText;
			strText.Format(_T("Read disk failed with write file error code:%d"),dwLastError);

			m_StatusBar.SetText(strText,0,0);

			break;
		}

		QueryPerformanceCounter(&t2);

		m_dbUsedTime += ((double)(t2.QuadPart - t1.QuadPart)) / frq.QuadPart;

		ullRealStartSector += dwSectors;
		ullReadSectors += dwSectors;
		ullRemainSectors -= dwSectors;
		ullOffset += dwSize;

		m_ullCompletedSize += dwSize;
	}

	delete[] lpBuf;

	if (!m_bStart)
	{
		bOK = FALSE;

		CString strText;
		strText.Format(_T("Read disk failed with user cancelled"));

		m_StatusBar.SetText(strText,0,0);
	}

	return bOK;
}

BOOL CRWDiskUIDlg::WriteDiskFromFile( HANDLE hDevice,HANDLE hFile,ULONGLONG ullStartSector, ULONGLONG ullSectors )
{
	ULONGLONG ullRealStartSector = ullStartSector;
	ULONGLONG ullReadSectors = 0;
	ULONGLONG ullRemainSectors = ullSectors;
	DWORD dwSectors = 0;
	BOOL  bOK = TRUE;
	ULONGLONG ullOffset = 0;

	PBYTE lpBuf = new BYTE[BUF_LENGTH];
	memset(lpBuf,0,BUF_LENGTH);

	LARGE_INTEGER frq,t1,t2;
	QueryPerformanceFrequency(&frq);

	while (ullReadSectors < ullSectors && m_bStart)
	{
		QueryPerformanceCounter(&t1);
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
		BOOL bRec = ReadFileAsyn(hFile,ullOffset,dwSize,lpBuf,&m_OverlapedFile,&dwLastError);

		if (!bRec)
		{
			bOK = FALSE;
			
			CString strText;
			strText.Format(_T("Write disk failed with read file error code:%d"),dwLastError);

			m_StatusBar.SetText(strText,0,0);

			break;
		}

		bRec = WriteSectors(hDevice,ullRealStartSector,dwSectors,BYTES_PER_SECTOR,lpBuf,&m_OverlapedDisk,&dwLastError);

		if (!bRec)
		{
			bOK = FALSE;
			
			CString strText;
			strText.Format(_T("Write disk failed with write sector error code:%d"),dwLastError);

			m_StatusBar.SetText(strText,0,0);

			break;
		}

		QueryPerformanceCounter(&t2);

		m_dbUsedTime += ((double)(t2.QuadPart - t1.QuadPart)) / frq.QuadPart;

		ullRealStartSector += dwSectors;
		ullReadSectors += dwSectors;
		ullRemainSectors -= dwSectors;
		ullOffset += dwSize;

		m_ullCompletedSize += dwSize;
	}

	delete[] lpBuf;

	if (!m_bStart)
	{
		bOK = FALSE;

		CString strText;
		strText.Format(_T("Write disk failed with user cancelled"));

		m_StatusBar.SetText(strText,0,0);
	}

	return bOK;
}

BOOL CRWDiskUIDlg::VerifyDiskAndFile( HANDLE hDevice,HANDLE hFile,ULONGLONG ullStartSector, ULONGLONG ullSectors )
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

	LARGE_INTEGER frq,t1,t2;
	QueryPerformanceFrequency(&frq);

	while (ullReadSectors < ullSectors && m_bStart)
	{
		QueryPerformanceCounter(&t1);
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
		BOOL bRec = ReadSectors(hDevice,ullRealStartSector,dwSectors,BYTES_PER_SECTOR,lpBufDisk,&m_OverlapedDisk,&dwLastError);

		if (!bRec)
		{
			bOK = FALSE;

			CString strText;
			strText.Format(_T("Verify failed with read sector error code:%d"),dwLastError);

			m_StatusBar.SetText(strText,0,0);
			break;
		}

		memset(lpBufFile,0,BUF_LENGTH);

		DWORD dwSize = dwSectors * BYTES_PER_SECTOR;
		bRec = ReadFileAsyn(hFile,ullOffet,dwSize,lpBufFile,&m_OverlapedFile,&dwLastError);

		if (!bRec)
		{
			bOK = FALSE;

			CString strText;
			strText.Format(_T("Verify failed with read file error:%d"),dwLastError);

			m_StatusBar.SetText(strText,0,0);
			break;
		}

		for (DWORD i = 0;i < dwSize;i++)
		{
			if (lpBufDisk[i] != lpBufFile[i])
			{
				bOK = FALSE;

				CString strText;
				strText.Format(_T("Verify failed with compare error:sector %d,offset %d")
					,(ullOffet + i) / BYTES_PER_SECTOR,(ullOffet + i) % BYTES_PER_SECTOR);

				m_StatusBar.SetText(strText,0,0);
				
				break;
			}
		}

		if (!bOK)
		{
			break;
		}

		QueryPerformanceCounter(&t2);

		m_dbUsedTime += ((double)(t2.QuadPart - t1.QuadPart)) / frq.QuadPart;

		ullRealStartSector += dwSectors;
		ullReadSectors += dwSectors;
		ullRemainSectors -= dwSectors;
		ullOffet += dwSize;

		m_ullCompletedSize += dwSize;
	}

	delete[] lpBufDisk;

	if (!m_bStart)
	{
		bOK = FALSE;

		CString strText;
		strText.Format(_T("Verify disk failed with user cancelled"));

		m_StatusBar.SetText(strText,0,0);
	}

	return bOK;
}

BOOL CRWDiskUIDlg::EraseDisk( HANDLE hDevice,ULONGLONG ullStartSector, ULONGLONG ullSectors )
{
	ULONGLONG ullRealStartSector = ullStartSector;
	ULONGLONG ullReadSectors = 0;
	ULONGLONG ullRemainSectors = ullSectors;
	DWORD dwSectors = 0;
	BOOL  bOK = TRUE;

	PBYTE lpBuf = new BYTE[BUF_LENGTH];
	memset(lpBuf,0,BUF_LENGTH);

	LARGE_INTEGER frq,t1,t2;
	QueryPerformanceFrequency(&frq);

	while (ullReadSectors < ullSectors && m_bStart)
	{
		QueryPerformanceCounter(&t1);
		if (ullRemainSectors < PER_SECTORS)
		{
			dwSectors = (DWORD)ullRemainSectors;
		}
		else
		{
			dwSectors = PER_SECTORS;
		}

		DWORD dwLastError = 0;

		BOOL bRec = WriteSectors(hDevice,ullRealStartSector,dwSectors,BYTES_PER_SECTOR,lpBuf,&m_OverlapedDisk,&dwLastError);

		if (!bRec)
		{
			bOK = FALSE;
			
			CString strText;
			strText.Format(_T("Erase disk failed with write sector error:%d"),dwLastError);

			m_StatusBar.SetText(strText,0,0);

			break;
		}

		QueryPerformanceCounter(&t2);

		m_dbUsedTime += ((double)(t2.QuadPart - t1.QuadPart) / frq.QuadPart);

		ullRealStartSector += dwSectors;
		ullReadSectors += dwSectors;
		ullRemainSectors -= dwSectors;

		m_ullCompletedSize += dwSectors * BYTES_PER_SECTOR;
	}

	delete[] lpBuf;

	if (!m_bStart)
	{
		bOK = FALSE;

		CString strText;
		strText.Format(_T("Erase disk failed with user cancelled"));

		m_StatusBar.SetText(strText,0,0);
	}

	return bOK;
}

DWORD WINAPI CRWDiskUIDlg::StartProcThread( LPVOID lpParm )
{
	CRWDiskUIDlg *pDlg = (CRWDiskUIDlg *)lpParm;

	pDlg->OnStart();

	return 1;
}


BOOL CRWDiskUIDlg::DestroyWindow()
{
	// TODO: 在此添加专用代码和/或调用基类

	if (m_hDisk != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hDisk);
		m_hDisk = INVALID_HANDLE_VALUE;
	}

	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}

	if (m_OverlapedDisk.hEvent != NULL)
	{
		CloseHandle(m_OverlapedDisk.hEvent);

		m_OverlapedDisk.hEvent = NULL;
	}

	if (m_OverlapedFile.hEvent != NULL)
	{
		CloseHandle(m_OverlapedFile.hEvent);

		m_OverlapedFile.hEvent = NULL;
	}

	return CDialogEx::DestroyWindow();
}

void CRWDiskUIDlg::EnableControl( BOOL bEnable )
{
	m_ComboBoxDisk.EnableWindow(bEnable);
	m_ComboBoxFunction.EnableWindow(bEnable);
	GetDlgItem(IDC_CHECK_VERIFY)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_START_SECTOR)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_SECTOR_COUNT)->EnableWindow(bEnable);
}

BOOL CRWDiskUIDlg::OnDeviceChange( UINT nEventType, DWORD_PTR dwData )
{
	PDEV_BROADCAST_HDR   pDevBroadcastHdr;   
	//这里进行信息匹配,比如guid等

	switch (nEventType)
	{		
	case DBT_DEVICEREMOVECOMPLETE:
	case DBT_DEVICEARRIVAL:
		pDevBroadcastHdr = (PDEV_BROADCAST_HDR)dwData;
		if (pDevBroadcastHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
		{
			if (!m_bStart)
			{
				InitialComboBoxDisk();
			}
		}
		break;
	default:
		break;
	}
	return TRUE;
}

void CRWDiskUIDlg::UpdateState()
{
	CString strUsage;
	strUsage.Format(_T("%s / %s"),CUtils::AdjustFileSize(m_ullCompletedSize),CUtils::AdjustFileSize(m_ullTotalSize));
	SetDlgItemText(IDC_TEXT_USAGE,strUsage);

	int nPercent = (int)(m_ullCompletedSize * 100 / m_ullTotalSize);

	m_ProgressCtrl.SetPos(nPercent);

	if (m_dbUsedTime > 0)
	{
		double dbSpeed = 0.0;
		dbSpeed = m_ullCompletedSize / 1024. / 1024./m_dbUsedTime;

		CString strSpeed;
		strSpeed.Format(_T("%.2f MB/s"),dbSpeed);

		m_StatusBar.SetText(strSpeed,1,0);

		__time64_t time = (__time64_t)((m_ullTotalSize - m_ullCompletedSize) / 1024. / 1024. / dbSpeed);
		CTimeSpan span(time);

		m_StatusBar.SetText(span.Format(_T("%H:%M:%S")),2,0);
	}
}


void CRWDiskUIDlg::OnCbnSelchangeComboFunction()
{
	// TODO: 在此添加控件通知处理程序代码
	int nSelectIndex = m_ComboBoxFunction.GetCurSel();

	if (nSelectIndex == OP_ERASE || nSelectIndex == OP_VERIFY)
	{
		m_bCheckVerify = FALSE;
		GetDlgItem(IDC_CHECK_VERIFY)->ShowWindow(SW_HIDE);

		UpdateData(FALSE);
	}
	else
	{
		GetDlgItem(IDC_CHECK_VERIFY)->ShowWindow(SW_SHOW);
	}
}
