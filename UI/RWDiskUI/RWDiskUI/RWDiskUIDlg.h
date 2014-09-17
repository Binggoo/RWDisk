
// RWDiskUIDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "ProgressCtrlEx.h"

// CRWDiskUIDlg dialog
class CRWDiskUIDlg : public CDialogEx
{
// Construction
public:
	CRWDiskUIDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_RWDISKUI_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	CComboBox m_ComboBoxDisk;
	ULONGLONG m_ullEditStartSector;
	ULONGLONG m_ullEditSectorCount;
	CComboBox m_ComboBoxFunction;
	BOOL m_bCheckVerify;

	CStatusBarCtrl       m_StatusBar;
	CProgressCtrlEx m_ProgressCtrl;
	CFont m_font;

	BOOL  m_bStart;

	ULONGLONG m_ullTotalSize;
	ULONGLONG m_ullCompletedSize;
	double    m_dbUsedTime;

	OVERLAPPED m_OverlapedDisk;
	OVERLAPPED m_OverlapedFile;
	HANDLE m_hDisk;
	HANDLE m_hFile;

	CString m_strSaveFileName;

	enum
	{
		OP_READ = 0,
		OP_WRITE,
		OP_VERIFY,
		OP_ERASE
	};

	void InitialComboBoxDisk();

	void OnStart();
	static DWORD WINAPI StartProcThread(LPVOID lpParm);

	BOOL ReadDiskToFile(HANDLE hDevice,HANDLE hFile,ULONGLONG ullStartSector, ULONGLONG ullSectors);
	BOOL WriteDiskFromFile(HANDLE hDevice,HANDLE hFile,ULONGLONG ullStartSector, ULONGLONG ullSectors);
	BOOL VerifyDiskAndFile(HANDLE hDevice,HANDLE hFile,ULONGLONG ullStartSector, ULONGLONG ullSectors);
	BOOL EraseDisk(HANDLE hDevice,ULONGLONG ullStartSector, ULONGLONG ullSectors);

	void EnableControl(BOOL bEnable);
	void UpdateState();

public:
	afx_msg void OnBnClickedBtnStart();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL DestroyWindow();
	virtual BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);
	afx_msg void OnCbnSelchangeComboFunction();
};
