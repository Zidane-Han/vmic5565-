// RFMTestDlg.h : header file
//

#if !defined(AFX_RFMTESTDLG_H__27487BC5_671B_46AE_B2EA_781AEB0179FA__INCLUDED_)
#define AFX_RFMTESTDLG_H__27487BC5_671B_46AE_B2EA_781AEB0179FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////////
// RFM Includes
#include <sys/types.h>
#include <sys/timeb.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>

#include "windows.h"
#include "windowsx.h"
#include "winioctl.h"

#include "rfm2gdll_stdc.h"
#include "rfm2g_errno.h"
#include "version.h"


/////////////////////////////////////////////////////////////////////////////
// CRFMTestDlg dialog

class CRFMTestDlg : public CDialog
{
// Construction
public:
	CRFMTestDlg(CWnd* pParent = NULL);	// standard constructor
	void CountUpdate(WPARAM wParam, LPARAM lParam);
	short ReadDataBuffer(DWORD offset,BYTE *buf,DWORD size) ;
	short FillDataBuffer(DWORD offset,BYTE *buf, DWORD size) ;

// Dialog Data
	//{{AFX_DATA(CRFMTestDlg)
	enum { IDD = IDD_RFMTEST_DIALOG };
	CButton	m_ctrlStopMsg;
	CButton	m_ctrlRecvMsg;
	CButton	m_ctrlSendMsg;
	CButton	m_ctrlStop;
	CButton	m_ctrlRecvData;
	CButton	m_ctrlSendData;
	int		m_nFromNode;
	int		m_nToNode;
	int		m_nRateTestNode;
	CString	m_szConfig;
	DWORD	m_nCount;
	CString	m_szSpeed;
	CString	m_szDrvVersion;
	CString	m_szReadValue;
	CString	m_szFillValue;
	CString	m_szReadData;
	CString	m_szFillOffset;
	CString	m_szReadOffset;
	DWORD	m_nMsgCount;
	int		m_nSendMsg;
	int		m_nRecvMsg;
	DWORD	m_nMsgMilSeconds;
	DWORD	m_nMsgValue;
	DWORD	m_nMsgValue1;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRFMTestDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CRFMTestDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonWrite();
	afx_msg void OnButtonFill();
	afx_msg void OnButtonSendMSG();
	afx_msg void OnButtonRcvMSG();
	afx_msg void OnButtonSndData();
	afx_msg void OnButtonRcvdata();
	afx_msg void OnButtonStop();
	virtual void OnCancel();
	afx_msg void OnButtonRead();
	afx_msg void OnButtonReadData();
	afx_msg void OnButtonLastRead();
	afx_msg void OnButtonNextRead();
	afx_msg void OnButtonLastFill();
	afx_msg void OnButtonNextFill();
	afx_msg void OnButtonStopMsg();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	// 列表项
	LV_ITEM m_lvitem;
	// 添加条目
	int iPos;
	char  strTemp[256];
	DWORD m_nMemSize;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RFMTESTDLG_H__27487BC5_671B_46AE_B2EA_781AEB0179FA__INCLUDED_)
