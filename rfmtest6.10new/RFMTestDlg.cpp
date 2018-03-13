// RFMTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RFMTest.h"
#include "RFMTestDlg.h"
#include <stdlib.h>
#include <stdio.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DATA_BUF_SIZE   (4096)
#define WM_UPDATE    (WM_USER+100) 
#define WM_UPDATE1    (WM_USER+101) 

// clear data send/recv counter
#define UPDATE_SNDRCV_DATA_CLEAR 0
//clear msg send/recv counter
#define UPDATE_SNDRCV_MSG_CLEAR 1

// data send/recv thread stopped, enable buttons 
#define UPDATE_SNDRCV_DATA_EXIT 2
// msg send/recv thread stopped, enable buttons 
#define UPDATE_SNDRCV_MSG_EXIT 3

#define UPDATE_SEND_DATA_COUNT 4
#define UPDATE_RECV_DATA_COUNT 5
#define UPDATE_SEND_MSG_COUNT 6
#define UPDATE_RECV_MSG_COUNT 7

#define DATA_HANDSHAKE_MSG RFM2GEVENT_INTR3
#define DATA_HANDSHAKE_ACK RFM2GEVENT_INTR4

/////////////////////////////////////////////////////////////////////////////
// 反射内存的名称，第一块名称"\\\\.\\rfm2g1"，第2块"\\\\.\\rfm2g2"，依次类推
char	*rfmFn = "\\\\.\\rfm2g1";
// 函数调用返回状态
ULONG retstatus;
// 反射内存句柄
RFM2GHANDLE	g_rfgmHandle;
RFM2GEVENTTYPE g_msgEventType;

// 显示起始地址
BYTE DataBuf[DATA_BUF_SIZE];
DWORD *dwDataBuf;
DWORD g_count=0,g_count1,g_msgValue,g_msgValue1,g_msgFromNode,g_datFromNode;
DWORD g_tick0,g_tick,g_tick1,g_tick2;

// 显示字符
CString strInfo;
HWND myhWnd;
int threadExit=FALSE,threadActive=FALSE;
int threadExit1=FALSE,threadActive1=FALSE;

// 速度测试用--1MB
BYTE rateTestData[1*1024*1024];

UINT RecvDataThread( LPVOID lParam );
UINT SendDataThread( LPVOID lParam );
UINT RecvMsgThread( LPVOID lParam );
UINT SendMsgThread( LPVOID lParam );

/////////////////////////////////////////////////////////////////////////////
// CRFMTestDlg dialog

CRFMTestDlg::CRFMTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRFMTestDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRFMTestDlg)
	m_nFromNode = 0;
	m_nToNode = 0;
	m_nRateTestNode = 0;
	m_szConfig = _T("");
	m_nCount = 0;
	m_szSpeed = _T("");
	m_szDrvVersion = _T("R6.0");
	m_szReadValue = _T("");
	m_szFillValue = _T("12345678");
	m_szReadData = _T("");
	m_szFillOffset = _T("00000000");
	m_szReadOffset = _T("00000000");
	m_nMsgCount = 0;
	m_nSendMsg = -1;
	m_nRecvMsg = -1;
	m_nMsgMilSeconds = 0;
	m_nMsgValue = 0;
	m_nMsgValue1 = 0;
	m_nSendMsg=0;
	m_nRecvMsg=0;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRFMTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRFMTestDlg)
	DDX_Control(pDX, IDC_BUTTON_STOP_MSG, m_ctrlStopMsg);
	DDX_Control(pDX, IDC_BUTTON_RCV, m_ctrlRecvMsg);
	DDX_Control(pDX, IDC_BUTTON_SEND, m_ctrlSendMsg);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_ctrlStop);
	DDX_Control(pDX, IDC_BUTTON_RCVDATA, m_ctrlRecvData);
	DDX_Control(pDX, IDC_BUTTON_SND_DATA, m_ctrlSendData);
	DDX_Text(pDX, IDC_EDIT_FROM_NODE, m_nFromNode);
	DDX_Text(pDX, IDC_EDIT_TONODE, m_nToNode);
	DDX_Text(pDX, IDC_EDIT_TONODE2, m_nRateTestNode);
	DDX_Text(pDX, IDC_EDIT_CONFIG, m_szConfig);
	DDX_Text(pDX, IDC_EDIT_COUNT, m_nCount);
	DDX_Text(pDX, IDC_EDIT_SPEED, m_szSpeed);
	DDX_Text(pDX, IDC_EDIT_DRV_VERSION, m_szDrvVersion);
	DDX_Text(pDX, IDC_EDIT_RD_VALUE, m_szReadValue);
	DDX_Text(pDX, IDC_EDIT_FILL_VALUE, m_szFillValue);
	DDX_Text(pDX, IDC_EDIT_READ_DATA, m_szReadData);
	DDX_Text(pDX, IDC_EDIT_FILL_ADDRESS, m_szFillOffset);
	DDX_Text(pDX, IDC_EDIT_RD_ADDRESS, m_szReadOffset);
	DDX_Text(pDX, IDC_EDIT_MSG_COUNT, m_nMsgCount);
	DDX_CBIndex(pDX, IDC_COMBO_SNDMSG, m_nSendMsg);
	DDX_CBIndex(pDX, IDC_COMBO_RCVMSG, m_nRecvMsg);
	DDX_Text(pDX, IDC_EDIT_MSG_SECONDS, m_nMsgMilSeconds);
	DDX_Text(pDX, IDC_EDIT_USERDATA, m_nMsgValue);
	DDX_Text(pDX, IDC_EDIT_RCVDATA, m_nMsgValue1);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRFMTestDlg, CDialog)
	//{{AFX_MSG_MAP(CRFMTestDlg)
	ON_BN_CLICKED(IDC_BUTTON_WRITE, OnButtonWrite)
	ON_BN_CLICKED(IDC_BUTTON_FILL, OnButtonFill)
	ON_BN_CLICKED(IDC_BUTTON_SEND, OnButtonSendMSG)
	ON_BN_CLICKED(IDC_BUTTON_RCV, OnButtonRcvMSG)
	ON_BN_CLICKED(IDC_BUTTON_SND_DATA, OnButtonSndData)
	ON_BN_CLICKED(IDC_BUTTON_RCVDATA, OnButtonRcvdata)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_READ, OnButtonRead)
	ON_BN_CLICKED(IDC_BUTTON_READ_DATA, OnButtonReadData)
	ON_BN_CLICKED(IDC_BUTTON_LAST_READ, OnButtonLastRead)
	ON_BN_CLICKED(IDC_BUTTON_NEXT_READ, OnButtonNextRead)
	ON_BN_CLICKED(IDC_BUTTON_LAST_FILL, OnButtonLastFill)
	ON_BN_CLICKED(IDC_BUTTON_NEXT_FILL, OnButtonNextFill)
	ON_MESSAGE( WM_UPDATE, CountUpdate )
	ON_BN_CLICKED(IDC_BUTTON_STOP_MSG, OnButtonStopMsg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRFMTestDlg message handlers

BOOL CRFMTestDlg::OnInitDialog()
{
	DWORD memsize=0;
	RFM2G_NODE nodeId;
	RFM2G_UINT8 boardId;

	dwDataBuf=(DWORD*)DataBuf;
	m_nMemSize=16*1024*1024;

	retstatus = RFM2gOpen( rfmFn, &g_rfgmHandle );

	if( retstatus != RFM2G_SUCCESS)
	{
		strInfo.Format( "Cannot open RFM2g file (%s)!\n", rfmFn );
		AfxMessageBox(strInfo);
		return FALSE;
	} 

	if(g_rfgmHandle == (RFM2GHANDLE) NULL)
	{
		strInfo.Format( "RFM2gopen returned NULL Handle (%s)!\n", rfmFn );
		AfxMessageBox(strInfo);
		return FALSE;
	}

	char DllVersionPtr[16];
	// 显示配置信息
	retstatus = RFM2gDeviceName(g_rfgmHandle, DllVersionPtr);
	if(retstatus != RFM2G_SUCCESS)
	{
		strInfo.Format("RFM2gDeviceName returned %d\n",retstatus);
		AfxMessageBox(strInfo);
		return FALSE;
	}
	strInfo.Format("Reflect Memory Board-2G: %s",DllVersionPtr );
	m_szConfig = strInfo;

	// 内存范围
	retstatus = RFM2gSize(g_rfgmHandle, &memsize );
	if(retstatus != RFM2G_SUCCESS)
	{
		strInfo.Format("RFM2gSize returned %d\n",retstatus);
		AfxMessageBox(strInfo);
		return FALSE;
	}
	strInfo.Format(", Size %d MB.",memsize/1024/1024 );
	m_szConfig += strInfo;
	m_nMemSize=memsize;

	// board ID
	retstatus = RFM2gBoardID(g_rfgmHandle, &boardId );
	if(retstatus != RFM2G_SUCCESS)
	{
		strInfo.Format("RFM2gBoardID returned %d\n",retstatus);
		AfxMessageBox(strInfo);
		return FALSE;
	}
	strInfo.Format("\r\nBoard ID:0x%02X.", boardId&0xff );
	m_szConfig += strInfo;

	// Node ID
	retstatus = RFM2gNodeID(g_rfgmHandle, &nodeId );
	if(retstatus != RFM2G_SUCCESS)
	{
		strInfo.Format("RFM2gNodeID returned %d\n",retstatus);
		AfxMessageBox(strInfo);
		return FALSE;
	}
	strInfo.Format("   Node ID:%d.", nodeId&0xff );
	m_szConfig += strInfo;

	//driver version
	retstatus = RFM2gDriverVersion(g_rfgmHandle, DllVersionPtr);
	if(retstatus != RFM2G_SUCCESS)
	{
		strInfo.Format("RFM2gDriverVersion returned %d\n",retstatus);
		AfxMessageBox(strInfo);
		return FALSE;
	}
	strInfo.Format("Driver %s",DllVersionPtr );
	m_szDrvVersion = strInfo;

	
	// 取得DLL驱动版本信息
	retstatus = RFM2gDllVersion(g_rfgmHandle, DllVersionPtr);
	if(retstatus != RFM2G_SUCCESS)
	{
		strInfo.Format("RFM2gDriverVersion returned %d\n",retstatus);
		AfxMessageBox(strInfo);
		return FALSE;
	}
	strInfo.Format(", DLL %s.",DllVersionPtr );
	m_szDrvVersion += strInfo;

	m_nToNode = m_nRateTestNode = m_nFromNode = nodeId+1;


	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	m_ctrlSendData.EnableWindow(TRUE);
	m_ctrlRecvData.EnableWindow(TRUE);
	m_ctrlStop.EnableWindow(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRFMTestDlg::CountUpdate(WPARAM wParam, LPARAM lParam)
{
	m_nCount=g_count;
	m_nMsgCount=g_count1;
	
	if( lParam==UPDATE_SNDRCV_DATA_CLEAR )
	{
		m_szSpeed = "";
		m_nCount = 0;
		m_ctrlSendData.EnableWindow(FALSE);
		m_ctrlRecvData.EnableWindow(FALSE);
		m_ctrlStop.EnableWindow(TRUE);
		UpdateData(FALSE);
		return;
	}
	else
	if( lParam == UPDATE_SNDRCV_DATA_EXIT )
	{
		m_ctrlSendData.EnableWindow(TRUE);
		m_ctrlRecvData.EnableWindow(TRUE);
		m_ctrlStop.EnableWindow(FALSE);

		UpdateData(FALSE);
		
		return;
	}
	else 
		if( lParam == UPDATE_SEND_DATA_COUNT || lParam == UPDATE_RECV_DATA_COUNT  )
	{
		if( g_tick!=g_tick0)
		m_szSpeed.Format("%.3f",sizeof(rateTestData)/1024.0/1024.0/(g_tick-g_tick0)*1000.0*(g_count-1));
		else m_szSpeed="";
		
		m_nRateTestNode = g_datFromNode;

		UpdateData(FALSE);
		
		return;
	}
	else
	if( lParam==UPDATE_SNDRCV_MSG_CLEAR )
	{
		m_nMsgCount = m_nMsgMilSeconds = g_msgFromNode = 0;
		m_ctrlSendMsg.EnableWindow(FALSE);
		m_ctrlRecvMsg.EnableWindow(FALSE);
		m_ctrlStopMsg.EnableWindow(TRUE);
		UpdateData(FALSE);
		return;
	}
	else if( lParam == UPDATE_SEND_MSG_COUNT || lParam == UPDATE_RECV_MSG_COUNT  )
	{

		if( g_tick2!=g_tick1)	m_nMsgMilSeconds = (g_tick2-g_tick1);
		else m_nMsgMilSeconds=0;

		m_nMsgCount = g_count1;
		m_nFromNode = g_msgFromNode;
		m_nMsgValue1 = g_msgValue1;

		UpdateData(FALSE);
		
		return;
	}
	else
		if( lParam == UPDATE_SNDRCV_MSG_EXIT )
	{
		m_ctrlSendMsg.EnableWindow(TRUE);
		m_ctrlRecvMsg.EnableWindow(TRUE);
		m_ctrlStopMsg.EnableWindow(FALSE);

		UpdateData(FALSE);
		
		return;
	}

	UpdateData(FALSE);
}

/////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// 速度测试 - 发送端
void CRFMTestDlg::OnButtonSndData() 
{
	UpdateData(TRUE);

	myhWnd = this->m_hWnd;

	// 测试数据节点
	if(m_nRateTestNode < 0 || m_nRateTestNode > 255)
	{
		strInfo.Format("错误的节点号！");
		AfxMessageBox(strInfo);
		return;
	}

	if(AfxBeginThread(SendDataThread,(LPVOID)m_nRateTestNode ) == NULL) {
			AfxMessageBox("Can not start Send Thread!");
			m_ctrlSendData.EnableWindow(TRUE);
			m_ctrlRecvData.EnableWindow(TRUE);
			m_ctrlStop.EnableWindow(FALSE);
	}

	m_ctrlSendData.EnableWindow(FALSE);
	m_ctrlRecvData.EnableWindow(FALSE);
	m_ctrlStop.EnableWindow(TRUE);

}

///////////////////////////////////////////////////////////////////////////////
// 速度测试，接收端
void CRFMTestDlg::OnButtonRcvdata() 
{
	// 消息ID -- 1 
	myhWnd = this->m_hWnd;

	if(AfxBeginThread(RecvDataThread,0) == NULL) {
			AfxMessageBox("Can not start Receive Thread!");
			m_ctrlSendData.EnableWindow(TRUE);
			m_ctrlRecvData.EnableWindow(TRUE);
			m_ctrlStop.EnableWindow(FALSE);
	}

			m_ctrlSendData.EnableWindow(FALSE);
			m_ctrlRecvData.EnableWindow(FALSE);
			m_ctrlStop.EnableWindow(TRUE);

}

// 速度测试，发送端
UINT SendDataThread( LPVOID lParam ) 
{
	int j,destId,n=0,count,retval;
		
	destId = (int)lParam;
	// 测试数据节点
	if(destId < 0 || destId > 255)
	{
		AfxMessageBox("错误的节点号！");
		return 0;
	}
	// 写入数据，0FFSET = 0
	for(j=0;j<256;j+=4)
	{
		*(int *)(rateTestData+j)=j;
	}

	// 通信用消息
	RFM2GEVENTINFO EventInfo;
	// 消息ID -- 1 
	EventInfo.Event = DATA_HANDSHAKE_ACK;
	EventInfo.Timeout = 100;
	EventInfo.NodeId = destId;

	// 使能消息
	if(RFM2gEnableEvent(g_rfgmHandle,EventInfo.Event) != RFM2G_SUCCESS)
	{
		strInfo.Format("RFM2gEnableEvent : Failure\n");
		AfxMessageBox(strInfo);
		return -1;
	}

	g_count=count=0;
	threadExit=FALSE;
	threadActive=TRUE;

	//clear counter
	::PostMessage(myhWnd,WM_UPDATE,g_tick-g_tick0, UPDATE_SNDRCV_DATA_CLEAR );

while( !threadExit )
{
	if(RFM2gWrite(g_rfgmHandle,0,(void*)rateTestData,sizeof(rateTestData)) != RFM2G_SUCCESS)
	{
		strInfo.Format("RFM2gWrite Error!");
		AfxMessageBox(strInfo);
		break;
	}
	*(int *)(rateTestData+0)=count++;
#if 1
	// 通知接收节点数据已发送
	if(RFM2gSendEvent( g_rfgmHandle, destId, DATA_HANDSHAKE_MSG, g_count )!=RFM2G_SUCCESS)
	{
		strInfo.Format("RFM2gSendEvent : Failure\n");
		AfxMessageBox(strInfo);
		break;
	}
#endif


	// 等待接收方发回的确认消息 2
#if 1
	EventInfo.Event = DATA_HANDSHAKE_ACK;
	EventInfo.Timeout = 100;
	// 使能消息
	if(RFM2gEnableEvent(g_rfgmHandle,EventInfo.Event) != RFM2G_SUCCESS)
	{
		strInfo.Format("RFM2gEnableEvent : Failure\n");
		AfxMessageBox(strInfo);
		break;
	}
	n=0;
	while( !threadExit )
{
	retval = RFM2gWaitForEvent(g_rfgmHandle,&EventInfo);

	if(retval == RFM2G_TIMED_OUT)
	{
		n += EventInfo.Timeout;
		if( n>30000) //30 seconds will timeout 
		{
		strInfo.Format("RFM2gWaitForEvent : 30 Seconds Timeout %d!\n",retval);
		AfxMessageBox(strInfo);
		threadExit=TRUE;
		break;		
		}
		continue;
	}
	else if(retval != RFM2G_SUCCESS)
	{
		strInfo.Format("RFM2gWaitForEvent : Failure %d!\n",retval);
		AfxMessageBox(strInfo);
		threadExit=TRUE;
		break;		
	}
	//got event!!
	break;
}
#endif

	if( !g_count )	 g_tick0 = GetTickCount(); 
	else g_tick = GetTickCount();

	g_count++;

	::PostMessage(myhWnd,WM_UPDATE,g_tick-g_tick0, UPDATE_SEND_DATA_COUNT );
}

	// enable buttons 
	::PostMessage(myhWnd,WM_UPDATE,g_tick-g_tick0,UPDATE_SNDRCV_DATA_EXIT );

	threadActive=FALSE;

	return 0;
}

// 速度测试，接收端
UINT RecvDataThread( LPVOID lParam ) 
{
	RFM2GEVENTINFO EventInfo;
	int n,retval;

	// 消息ID -- 1 
	EventInfo.Event = DATA_HANDSHAKE_MSG;
	EventInfo.Timeout = 100;

	// 使能消息
	if(RFM2gEnableEvent(g_rfgmHandle,EventInfo.Event) != RFM2G_SUCCESS)
	{
		strInfo.Format("RFM2gEnableEvent: Failure! \n" );
		AfxMessageBox(strInfo);
		return -1;
	}

	g_count=0;
	threadExit=FALSE;
	threadActive=TRUE;
	n=0;
	// clear counter and disable buttons 
	::PostMessage(myhWnd,WM_UPDATE,g_tick-g_tick0,UPDATE_SNDRCV_DATA_CLEAR );

while( !threadExit )
{
	// 等待接收消息
	EventInfo.Event = DATA_HANDSHAKE_MSG;
	EventInfo.Timeout = 100;
	// 使能消息
	if(RFM2gEnableEvent(g_rfgmHandle,EventInfo.Event) != RFM2G_SUCCESS)
	{
		strInfo.Format("RFM2gEnableEvent: Failure! \n" );
		AfxMessageBox(strInfo);
		threadExit=TRUE;
		break;		
	}
	retval = RFM2gWaitForEvent(g_rfgmHandle,&EventInfo);
	if(retval == RFM2G_TIMED_OUT)
	{
		n += EventInfo.Timeout;
		 //30 seconds will timeout 
		if( n>30000)
		{
		strInfo.Format("RFM2gWaitForEvent : 30 Seconds Timeout %d!\n",retval);
		AfxMessageBox(strInfo);
		threadExit=TRUE;
		break;		
		}
		continue;
	}
	else if(retval != RFM2G_SUCCESS)
	{
		strInfo.Format("RFM2gWaitForEvent : Failure %d!\n",retval);
		AfxMessageBox(strInfo);
		threadExit=TRUE;
		break;		
	}
	
	n=0;

	if( !g_count )	 g_tick0 = GetTickCount(); 
	else g_tick = GetTickCount();

	g_count++;

#if 0
	// 读取数据
	if(RFM2gRead(g_rfgmHandle,0,(void*)rateTestData,sizeof(rateTestData)) != RFM2G_SUCCESS)
	{
		strInfo.Format("RFM2gRead Error!");
		AfxMessageBox(strInfo);
		break;
	}	
#endif
	
	// 统治发送节点数据已收到
	g_datFromNode = EventInfo.NodeId;
#if 1
	if(RFM2gSendEvent( g_rfgmHandle, EventInfo.NodeId,DATA_HANDSHAKE_ACK, 0)!=RFM2G_SUCCESS)
	{
		strInfo.Format("RFM2gSendEvent : Failure\n");
		AfxMessageBox(strInfo);
		break;
	}
#endif

	::PostMessage(myhWnd,WM_UPDATE,g_tick-g_tick0,UPDATE_RECV_DATA_COUNT);

}
	threadActive=FALSE;

	::PostMessage(myhWnd,WM_UPDATE,g_tick-g_tick0, UPDATE_SNDRCV_DATA_EXIT); /* enable buttons */

	return 0;
}

void CRFMTestDlg::OnButtonStop() 
{
	// TODO: Add your control notification handler code here
	threadExit=TRUE;
	Sleep(500);
	if( threadActive ) 	Sleep(1000);
	if( threadActive ) 	Sleep(1000);
	if( threadActive ) 
	{
		AfxMessageBox("Failed to stop the thread!");
	}
	else
	{
		m_ctrlSendData.EnableWindow(TRUE);
		m_ctrlRecvData.EnableWindow(TRUE);
		m_ctrlStop.EnableWindow(FALSE);
	}
}

void CRFMTestDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	if( threadActive ) OnButtonStop();
	if( threadActive1 ) OnButtonStopMsg();
	RFM2gClose( &g_rfgmHandle );
	CDialog::OnCancel();
}


//////////////////////////////////////////////////////////////////////
// 设置列表显示中行


void CRFMTestDlg::OnButtonRead() 
{
	// TODO: Add your control notification handler code here
	DWORD value=0,offset=0;

	UpdateData(TRUE);

	sscanf( m_szReadOffset,"%x",&offset);
	
	retstatus=RFM2gPeek32(g_rfgmHandle, (RFM2G_UINT32)offset, 
                         (RFM2G_UINT32 *)&value);
	if(retstatus != RFM2G_SUCCESS)
	{
		strInfo.Format("RFM2gPeek32 : Failure %d!\n",retstatus);
		AfxMessageBox(strInfo);
		return;		
	}
	m_szReadValue.Format("%X",value);
	m_szReadOffset.Format("%X",offset);

	UpdateData(FALSE);
	
}
// 写入数据
void CRFMTestDlg::OnButtonWrite() 
{

	DWORD value=0,offset=0;

	UpdateData(TRUE);
	
	sscanf( m_szFillOffset,"%x",&offset);
	sscanf( m_szFillValue,"%x",&value);

	retstatus=RFM2gPoke32(g_rfgmHandle, (RFM2G_UINT32)offset, 
                         (RFM2G_UINT32)value);
	if(retstatus != RFM2G_SUCCESS)
	{
		strInfo.Format("RFM2gPeek32 : Failure %d!\n",retstatus);
		AfxMessageBox(strInfo);
		return;		
	}
	m_szFillValue.Format("%X",value);
	m_szFillOffset.Format("%X",offset);
	m_szReadOffset.Format("%X",offset);

	value=0;
	retstatus=RFM2gPeek32(g_rfgmHandle, (RFM2G_UINT32)offset, 
                         (RFM2G_UINT32 *)&value);
	if(retstatus != RFM2G_SUCCESS)
	{
		strInfo.Format("RFM2gPeek32 : Failure %d!\n",retstatus);
		AfxMessageBox(strInfo);
		return;		
	}
	m_szReadValue.Format("%X",value);

	UpdateData(FALSE);

}

////////////////////////////////////////////////////////////////////////////////
// 填充一块内存
short CRFMTestDlg::FillDataBuffer( DWORD offset,BYTE *buf,DWORD size) 
{
	retstatus = RFM2gWrite( g_rfgmHandle, (RFM2G_UINT32)offset, (void *)buf, 
                        size );
	if(retstatus != RFM2G_SUCCESS)
	{
		strInfo.Format("RFM2gPeek32 : Failure %d!\n",retstatus);
		AfxMessageBox(strInfo);
		return retstatus;		
	}
	return 0;
}
void CRFMTestDlg::OnButtonFill() 
{
	DWORD value,i,offset=0;

	UpdateData(TRUE);

	sscanf( m_szFillOffset,"%x",&offset);
	sscanf( m_szFillValue,"%x",&value);

	for(i=0;i<DATA_BUF_SIZE/4;i++)	dwDataBuf[i]=value;

	retstatus = FillDataBuffer( offset, DataBuf, DATA_BUF_SIZE );
	if(retstatus != 0 )
	{
		strInfo.Format("RFM2gPeek32 : Failure %d!\n",retstatus);
		AfxMessageBox(strInfo);
		return;		
	}
	m_szFillValue.Format("%X",value);
	m_szFillOffset.Format("%X",offset);

	if( ReadDataBuffer( offset, DataBuf, DATA_BUF_SIZE )!=0 )
	{
		strInfo.Format("ReadDataBuffer : Failure !\n");
		AfxMessageBox(strInfo);
	}

	UpdateData(FALSE);

}

/////////////////////////////////////////////////////////////
// 发送消息
void CRFMTestDlg::OnButtonSendMSG() 
{
	myhWnd = this->m_hWnd;

	UpdateData(TRUE);

	g_msgValue = m_nMsgValue;

	// 消息ID -- 1 
	switch( m_nSendMsg )
	{
	case 1: g_msgEventType = RFM2GEVENT_INTR2; break;
	case 2: g_msgEventType = RFM2GEVENT_INTR3; break;
	default: g_msgEventType = RFM2GEVENT_INTR1; break;
	}

	if(AfxBeginThread(SendMsgThread,(LPVOID)m_nToNode ) == NULL) {
			AfxMessageBox("Can not start Send Thread!");
			m_ctrlSendMsg.EnableWindow(TRUE);
			m_ctrlRecvMsg.EnableWindow(TRUE);
			m_ctrlStopMsg.EnableWindow(FALSE);
	}

	m_ctrlSendMsg.EnableWindow(FALSE);
	m_ctrlRecvMsg.EnableWindow(FALSE);
	m_ctrlStopMsg.EnableWindow(TRUE);

  
}
// msg速度测试，发送端
 UINT SendMsgThread( LPVOID lParam ) 
{
	int destId,n=0,count,retval;
	RFM2GEVENTINFO EventInfo;
		
	destId = (int)lParam;
	// 测试数据节点
	if(destId < 0 || destId > 255)
	{
		AfxMessageBox("错误的节点号！");
		return 0;
	}

	g_count1=count=0;
	threadExit1=FALSE;
	threadActive1=TRUE;

	//clear counter
	::PostMessage(myhWnd,WM_UPDATE,0,UPDATE_SNDRCV_MSG_CLEAR );

while( !threadExit1 )
{
	Sleep(0);
	// 发送 msg
	if(RFM2gSendEvent( g_rfgmHandle, destId, g_msgEventType, g_msgValue )!=RFM2G_SUCCESS)
	{
		strInfo.Format("RFM2gSendEvent : Failure\n");
		AfxMessageBox(strInfo);
		break;
	}

	// 等待接收方发回的确认消息
#if 1
	EventInfo.Event = g_msgEventType;
	EventInfo.Timeout = 100;
	if(RFM2gEnableEvent(g_rfgmHandle,EventInfo.Event) != RFM2G_SUCCESS)
	{
		AfxMessageBox("RFM2gEnableEvent : Failure\n");
		break;
	}

	n=0;
	while( !threadExit1 )
{
	retval = RFM2gWaitForEvent(g_rfgmHandle,&EventInfo);

	if(retval == RFM2G_TIMED_OUT)
	{
		n += EventInfo.Timeout;
		if( n>30000) //30 seconds will timeout 
		{
		strInfo.Format("RFM2gWaitForEvent : 30 Seconds Timeout %d!\n",retval);
		AfxMessageBox(strInfo);
		threadExit1=TRUE;
		break;		
		}
		continue;
	}
	else if(retval != RFM2G_SUCCESS)
	{
		strInfo.Format("RFM2gWaitForEvent : Failure %d!\n",retval);
		AfxMessageBox(strInfo);
		threadExit1=TRUE;
		break;		
	}
	//got event

	break;
}
#endif

	if( !g_count1 )	 g_tick1 = GetTickCount(); 
	else g_tick2 = GetTickCount();

	g_count1++;

	::PostMessage(myhWnd,WM_UPDATE,g_tick2-g_tick1, UPDATE_SEND_MSG_COUNT );
}

	// enable buttons 
	::PostMessage(myhWnd,WM_UPDATE,g_tick-g_tick0,UPDATE_SNDRCV_MSG_EXIT );

	threadActive1=FALSE;

	return 0;
}

/////////////////////////////////////////////////////////////////////
// 接收网络消息
void CRFMTestDlg::OnButtonRcvMSG() 
{
	myhWnd = this->m_hWnd;

	UpdateData(TRUE);

	// 消息ID -- 1 
	switch( m_nRecvMsg )
	{
	case 1: g_msgEventType = RFM2GEVENT_INTR2; break;
	case 2: g_msgEventType = RFM2GEVENT_INTR3; break;
	default: g_msgEventType = RFM2GEVENT_INTR1; break;
	}

	if(AfxBeginThread(RecvMsgThread,(LPVOID)m_nToNode ) == NULL) {
			AfxMessageBox("Can not start Recv Thread!");
			m_ctrlSendMsg.EnableWindow(TRUE);
			m_ctrlRecvMsg.EnableWindow(TRUE);
			m_ctrlStopMsg.EnableWindow(FALSE);
	}

	m_ctrlSendMsg.EnableWindow(FALSE);
	m_ctrlRecvMsg.EnableWindow(FALSE);
	m_ctrlStopMsg.EnableWindow(TRUE);
}
// msg速度测试，recv端
UINT RecvMsgThread( LPVOID lParam ) 
{
	int n=0,count,retval;
		
	// 通信用消息
	RFM2GEVENTINFO EventInfo;

	g_count1=count=0;
	threadExit1=FALSE;
	threadActive1=TRUE;

	//clear counter
	::PostMessage(myhWnd,WM_UPDATE,g_tick2-g_tick1,UPDATE_SNDRCV_MSG_CLEAR );

	n=0;
while( !threadExit1 )
{
	EventInfo.Event = g_msgEventType;
	EventInfo.Timeout = 100;
	if(RFM2gEnableEvent(g_rfgmHandle,EventInfo.Event) != RFM2G_SUCCESS)
	{
		AfxMessageBox("RFM2gEnableEvent : Failure\n");
		break;
	}

	retval = RFM2gWaitForEvent(g_rfgmHandle,&EventInfo);

	if(retval == RFM2G_TIMED_OUT)
	{
		n += EventInfo.Timeout;
		if( n>30000) //30 seconds will timeout 
		{
		strInfo.Format("RFM2gWaitForEvent : 30 Seconds Timeout %d!\n",retval);
		AfxMessageBox(strInfo);
		threadExit1=TRUE;
		break;		
		}
		continue;
	}
	else if(retval != RFM2G_SUCCESS)
	{
		strInfo.Format("RFM2gWaitForEvent : Failure %d!\n",retval);
		AfxMessageBox(strInfo);
		threadExit1=TRUE;
		break;		
	}
	n=0;
	g_msgFromNode = EventInfo.NodeId;
	g_msgValue1 = EventInfo.ExtendedInfo;

	// 发送 ack msg
#if 1
	if(RFM2gSendEvent( g_rfgmHandle, EventInfo.NodeId, g_msgEventType, EventInfo.ExtendedInfo )!=RFM2G_SUCCESS)
	{
		strInfo.Format("RFM2gSendEvent : Failure\n");
		AfxMessageBox(strInfo);
		break;
	}
#endif

	if( !g_count1 )	 g_tick1 = GetTickCount(); 
	else g_tick2 = GetTickCount();

	g_count1++;

	::PostMessage(myhWnd,WM_UPDATE,g_tick2-g_tick1, UPDATE_RECV_MSG_COUNT );
}

	// enable buttons 
	::PostMessage(myhWnd,WM_UPDATE,g_tick-g_tick0, UPDATE_SNDRCV_MSG_EXIT );

	threadActive1=FALSE;

	return 0;
}
void CRFMTestDlg::OnButtonStopMsg() 
{
	// TODO: Add your control notification handler code here
	threadExit1 = TRUE;
	Sleep(500);
	if( threadActive1 ) 	Sleep(1000);
	if( threadActive1 ) 	Sleep(1000);
	if( threadActive1 ) 
	{
		AfxMessageBox("Failed to stop the thread!");
	}
	else
	{
		m_ctrlSendMsg.EnableWindow(TRUE);
		m_ctrlRecvMsg.EnableWindow(TRUE);
		m_ctrlStopMsg.EnableWindow(FALSE);
	}
}

/* read a buffer of data */
short CRFMTestDlg::ReadDataBuffer(DWORD offset,BYTE *buf, DWORD size) 
{
	// TODO: Add your control notification handler code here

	DWORD i,j;
	CString szTmp;
	char *ptr;

	retstatus = RFM2gRead( g_rfgmHandle, (RFM2G_UINT32)offset, (void *)buf, 
                        size );
	if(retstatus != RFM2G_SUCCESS)
	{
		strInfo.Format("RFM2gRead : Failure %d!\n",retstatus);
		AfxMessageBox(strInfo);
		return -1;		
	}

	m_szReadData="";
	ptr = (char*)dwDataBuf;

	//	 lines for 16 bytes each 
	for(i=0;i<size/16;i++) 
	{
		//address
		szTmp.Format("%06X:",offset+i*16 );
		m_szReadData += szTmp;
		for(j=0;j<16;j++)
		{
			szTmp.Format(" %02X",ptr[i*16+j]&0xff );
			m_szReadData += szTmp;
		}
		m_szReadData += "\r\n";
	}

	m_szReadOffset.Format("%X",offset);

	return 0;
}

void CRFMTestDlg::OnButtonReadData() 
{
	// TODO: Add your control notification handler code here

	DWORD offset=0;

	UpdateData(TRUE);

	sscanf( m_szReadOffset,"%x",&offset);
	if( ReadDataBuffer( offset, DataBuf,DATA_BUF_SIZE )!=0 )
	{
		strInfo.Format("ReadDataBuffer : Failure !\n");
		AfxMessageBox(strInfo);
		return;		
	}
	UpdateData(FALSE);	
}

void CRFMTestDlg::OnButtonLastRead() 
{
	// TODO: Add your control notification handler code here
	
	DWORD offset=0;

	UpdateData(TRUE);

	sscanf( m_szReadOffset,"%x",&offset);
	
	if( offset > DATA_BUF_SIZE ) offset-=DATA_BUF_SIZE;
	else offset=0;

	if( ReadDataBuffer( offset, DataBuf,DATA_BUF_SIZE )!=0 )
	{
		strInfo.Format("ReadDataBuffer : Failure !\n");
		AfxMessageBox(strInfo);
		return;		
	}
	//update screen
	if( ReadDataBuffer( offset,DataBuf, DATA_BUF_SIZE )!=0 )
	{
		strInfo.Format("ReadDataBuffer : Failure !\n");
		AfxMessageBox(strInfo);
		return;		
	}

	UpdateData(FALSE);	
}

//max 128MB
void CRFMTestDlg::OnButtonNextRead() 
{
	// TODO: Add your control notification handler code here
	DWORD offset=0,maxOffset;

	UpdateData(TRUE);

	sscanf( m_szReadOffset,"%x",&offset);
	
	maxOffset = m_nMemSize-DATA_BUF_SIZE;

	if( (offset+DATA_BUF_SIZE) <= maxOffset ) offset +=DATA_BUF_SIZE;
	else offset=maxOffset;

	if( ReadDataBuffer( offset, DataBuf,DATA_BUF_SIZE )!=0 )
	{
		strInfo.Format("ReadDataBuffer : Failure !\n");
		AfxMessageBox(strInfo);
		return;		
	}
	UpdateData(FALSE);	
}

void CRFMTestDlg::OnButtonLastFill() 
{
	// TODO: Add your control notification handler code here
	DWORD value,i,offset=0;

	UpdateData(TRUE);

	sscanf( m_szFillOffset,"%x",&offset);
	sscanf( m_szFillValue,"%x",&value);

	for(i=0;i<DATA_BUF_SIZE/4;i++)	dwDataBuf[i]=value;

	if( offset>DATA_BUF_SIZE ) offset=DATA_BUF_SIZE;
	else  offset=0;

	retstatus = FillDataBuffer( offset, DataBuf, DATA_BUF_SIZE );
	if(retstatus != 0 )
	{
		strInfo.Format("RFM2gPeek32 : Failure %d!\n",retstatus);
		AfxMessageBox(strInfo);
		return;		
	}
	m_szFillValue.Format("%X",value);
	m_szFillOffset.Format("%X",offset);

	//update screen
	if( ReadDataBuffer( offset, DataBuf, DATA_BUF_SIZE )!=0 )
	{
		strInfo.Format("ReadDataBuffer : Failure !\n");
		AfxMessageBox(strInfo);
	}

	UpdateData(FALSE);
	
}

void CRFMTestDlg::OnButtonNextFill() 
{
	// TODO: Add your control notification handler code here
	DWORD value,i,offset=0,maxOffset=0;

	UpdateData(TRUE);

	sscanf( m_szFillOffset,"%x",&offset);
	sscanf( m_szFillValue,"%x",&value);

	for(i=0;i<DATA_BUF_SIZE/4;i++)	dwDataBuf[i]=value;

	maxOffset = m_nMemSize-DATA_BUF_SIZE;

	if( (offset+DATA_BUF_SIZE) <= maxOffset ) offset +=DATA_BUF_SIZE;
	else offset=maxOffset;

	retstatus = FillDataBuffer( offset, DataBuf, DATA_BUF_SIZE );
	if(retstatus != 0 )
	{
		strInfo.Format("RFM2gPeek32 : Failure %d!\n",retstatus);
		AfxMessageBox(strInfo);
		return;		
	}
	m_szFillValue.Format("%X",value);
	m_szFillOffset.Format("%X",offset);

	if( ReadDataBuffer( offset, DataBuf, DATA_BUF_SIZE )!=0 )
	{
		strInfo.Format("ReadDataBuffer : Failure !\n");
		AfxMessageBox(strInfo);
	}

	UpdateData(FALSE);
	
}

