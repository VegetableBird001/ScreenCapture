// ScreenCapture.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "ScreenCapture.h"

#define MAX_LOADSTRING 100

// ȫ�ֱ���: 
HINSTANCE hInst;								// ��ǰʵ��
TCHAR szTitle[MAX_LOADSTRING];					// �������ı�
TCHAR szWindowClass[MAX_LOADSTRING];			// ����������
int g_ScreenX, g_ScreenY;						//��Ļ�ֱ���
HDC g_hMemDc;									//����ͼ����ڴ����DC	
HDC g_hGrayMemDc;								//�Ҷ�ͼ����ڴ����DC

RECT g_rtMouse;									//���ѡ����һ������
BOOL g_bMouseDown = FALSE;						//����������
BOOL g_bMouseUp = FALSE;						//������̧��

BOOL g_bIsRect = FALSE;							//����������ѡ��
HWND g_MainWnd;									//��ͼ���ھ��
// �˴���ģ���а����ĺ�����ǰ������: 
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//��Ϣ������
LRESULT CALLBACK OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OnLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OnLButtonDBClick(HWND hWnd, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK OnDlgCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
//���ܺ���
void ConvertToGrayBitmap(HDC hSrcDc, HBITMAP hSrcBitmap);
void WriteDatatoClipBoard();
void ScreenCapture();

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO:  �ڴ˷��ô��롣
	MSG msg;
	HACCEL hAccelTable;

	// ��ʼ��ȫ���ַ���
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SCREENCAPTURE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ִ��Ӧ�ó����ʼ��: 
	HWND hMainWnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN_DLG), NULL, DialogProc);
	ShowWindow(hMainWnd, nCmdShow);
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SCREENCAPTURE));

	// ����Ϣѭ��: 
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  ����:  MyRegisterClass()
//
//  Ŀ��:  ע�ᴰ���ࡣ
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SCREENCAPTURE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   ����:  InitInstance(HINSTANCE, int)
//
//   Ŀ��:  ����ʵ�����������������
//
//   ע��: 
//
//        �ڴ˺����У�������ȫ�ֱ����б���ʵ�������
//        ��������ʾ�����򴰿ڡ�
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����

   hWnd = CreateWindow(szWindowClass, szTitle, WS_POPUP,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, SW_MAXIMIZE);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  ����:  WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ŀ��:    ���������ڵ���Ϣ��
//
//  WM_COMMAND	- ����Ӧ�ó���˵�
//  WM_PAINT	- ����������
//  WM_DESTROY	- �����˳���Ϣ������
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// �����˵�ѡ��: 
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_CREATE:
		{
		g_MainWnd = hWnd;
			ScreenCapture();
		}
		break;
	case WM_PAINT:
		return OnPaint(hWnd, wParam, lParam);
	case WM_CLOSE:
		CloseWindow(hWnd);
		break;
	case WM_MOUSEMOVE:
		return OnMouseMove(hWnd, wParam, lParam);
	case WM_LBUTTONDOWN:
		return OnLButtonDown(hWnd, wParam, lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp(hWnd, wParam, lParam);
	case WM_LBUTTONDBLCLK:
		return OnLButtonDBClick(hWnd, wParam, lParam);
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// �����ڡ������Ϣ�������
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
//WM_PAINT��Ϣ������
LRESULT CALLBACK OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	POINT ptPenWidth = { 2, 2 };
	LOGPEN logpen = { 0 };
	HDC hdc;

	//��ʼ������
	logpen.lopnColor = RGB(0, 0, 255);
	logpen.lopnStyle = PS_SOLID;
	logpen.lopnWidth = ptPenWidth;
	HPEN hPen = CreatePenIndirect(&logpen);
	
	//��ʼ����ˢ
	LOGBRUSH logbrush = { 0 };
	logbrush.lbStyle = BS_NULL;
	HBRUSH hBrush = CreateBrushIndirect(&logbrush);
	hdc = BeginPaint(hWnd, &ps);
	SelectObject(hdc, hPen);
	SelectObject(hdc, hBrush);
	
	//���Ҷ�ͼ��ŵ��ڴ����dc�У�����Ǽ��ݸ�Ӧ�ó����dc
	HDC memDc = CreateCompatibleDC(hdc);
	HBITMAP bmp = CreateCompatibleBitmap(hdc, g_ScreenX, g_ScreenY);
	SelectObject(memDc, bmp);

	//���Ҷ�ͼ����Ƶ��������DC��
	BitBlt(memDc, 0, 0, g_ScreenX, g_ScreenX, g_hGrayMemDc, 0, 0, SRCCOPY);
	SelectObject(memDc, hBrush);
	SelectObject(memDc, hPen);

	if (g_bMouseDown || g_bIsRect)
	{
		//������ͼ��Ĳ��ֻ��Ƶ��ڴ�dc�У��൱����֮ǰ�Ļ���������һ��������ͼ��
		BitBlt(memDc, g_rtMouse.left, g_rtMouse.top, g_rtMouse.right - g_rtMouse.left, g_rtMouse.bottom - g_rtMouse.top, g_hMemDc, g_rtMouse.left, g_rtMouse.top, SRCCOPY);
		Rectangle(memDc, g_rtMouse.left, g_rtMouse.top, g_rtMouse.right, g_rtMouse.bottom);
	}
	//������ͼ��������������Ϊ����
	BitBlt(hdc, 0, 0, g_ScreenX, g_ScreenY, memDc, 0, 0, SRCCOPY);

	DeleteObject(bmp);
	DeleteObject(memDc);
	EndPaint(hWnd, &ps);
	return 0;
}
//��������, ��������Ƶ�����Ĵ�����
void ScreenCapture()
{
	HDC hDc = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);  //������Ļ��ͼ��DC
	g_ScreenX = GetDeviceCaps(hDc, HORZRES);   //��ȡ��Ļ��ˮƽ�ֱ���
	g_ScreenY = GetDeviceCaps(hDc, VERTRES);   //��ȡ��Ļ�Ĵ�ֱ�ֱ���
	
	g_hMemDc = CreateCompatibleDC(hDc);        //�������ݵ��ڴ�DC
	HBITMAP hBitMap = CreateCompatibleBitmap(hDc, g_ScreenX, g_ScreenY); //�������ݵ�bitmap
	SelectObject(g_hMemDc, hBitMap);
	BitBlt(g_hMemDc, 0, 0, g_ScreenX, g_ScreenY, hDc, 0, 0, SRCCOPY); //����Ļ��ͼ���浽�ڴ�DC��
	
	g_hGrayMemDc = CreateCompatibleDC(hDc);
	HBITMAP hGrayBitmap = CreateCompatibleBitmap(hDc, g_ScreenX, g_ScreenY);
	SelectObject(g_hGrayMemDc, hGrayBitmap);
	BitBlt(g_hGrayMemDc, 0, 0, g_ScreenX, g_ScreenY, hDc, 0, 0, SRCCOPY);

	ConvertToGrayBitmap(g_hGrayMemDc, hGrayBitmap);

	DeleteObject(hDc);
	DeleteObject(hBitMap);
	DeleteObject(hGrayBitmap);
}

void ConvertToGrayBitmap(HDC hSrcDc, HBITMAP hSrcBitmap)
{
	HBITMAP retBmp = hSrcBitmap;
	BITMAPINFO bmpInfo;
	ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	GetDIBits(hSrcDc, retBmp, 0, 0, NULL, &bmpInfo, DIB_RGB_COLORS);

	BYTE* bits = new BYTE[bmpInfo.bmiHeader.biSizeImage];
	GetBitmapBits(retBmp, bmpInfo.bmiHeader.biSizeImage, bits);

	int bytePerPixel = 4;//Ĭ��32λ
	if (bmpInfo.bmiHeader.biBitCount == 24)
	{
		bytePerPixel = 3;
	}
	for (DWORD i = 0; i<bmpInfo.bmiHeader.biSizeImage; i += bytePerPixel)
	{
		BYTE r = *(bits + i);
		BYTE g = *(bits + i + 1);
		BYTE b = *(bits + i + 2);
		*(bits + i) = *(bits + i + 1) = *(bits + i + 2) = (r + b + g) / 3;
	}
	SetBitmapBits(hSrcBitmap, bmpInfo.bmiHeader.biSizeImage, bits);
	delete[] bits;
}

LRESULT CALLBACK OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (!g_bMouseDown && !g_bIsRect)
	{
		g_bMouseDown = TRUE;
		POINT pt = { 0 };
		GetCursorPos(&pt);
		g_rtMouse.left = pt.x;
		g_rtMouse.top = pt.y;
		g_rtMouse.right = pt.x;
		g_rtMouse.bottom = pt.y;
		InvalidateRgn(hWnd, 0, FALSE);
	}

	return 0;
}

LRESULT CALLBACK OnLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (g_bMouseDown)
	{
		g_bMouseUp = TRUE;
		g_bMouseDown = FALSE;
		g_bIsRect = TRUE;
	}
	return 0;
}

LRESULT CALLBACK OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (g_bMouseDown)
	{
		POINT pt = { 0 };
		GetCursorPos(&pt);
		g_rtMouse.right = pt.x;
		g_rtMouse.bottom = pt.y;
		InvalidateRgn(hWnd, 0, FALSE);
	}
	return 0;
}

LRESULT CALLBACK OnLButtonDBClick(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (g_bIsRect)
	{
		WriteDatatoClipBoard();
		ZeroMemory(&g_rtMouse, sizeof(POINT));
		g_bIsRect = FALSE;
		g_bMouseDown = FALSE;
		g_bMouseUp = FALSE;
		InvalidateRgn(hWnd, 0, FALSE);
	}
	return 0;
}

void WriteDatatoClipBoard()
{
	HDC hMemDc, hScrDc;
	HBITMAP hBmp, hOldBmp;
	int width, height;
	width = g_rtMouse.right - g_rtMouse.left;
	height = g_rtMouse.bottom - g_rtMouse.top;

	hScrDc = CreateDC(L"DISPLAY", NULL, NULL, NULL);
	hMemDc = CreateCompatibleDC(hScrDc);
	hBmp = CreateCompatibleBitmap(hScrDc, width, height);

	//�⼸������SelectObject�᷵��ԭʼGDI��������ԣ���DC�л�ȡ��Ӧλͼ����Ϣ
	hOldBmp = (HBITMAP)SelectObject(hMemDc, hBmp);
	//����ȡ���ֵ�ͼƬ���뵽������
	BitBlt(hMemDc, 0, 0, width, height, g_hMemDc, g_rtMouse.left, g_rtMouse.top, SRCCOPY);
	hBmp = (HBITMAP)SelectObject(hMemDc, hOldBmp);

	DeleteDC(hMemDc);
	DeleteDC(hScrDc);
	//���Ƶ�������
	if (OpenClipboard(0))
	{
		EmptyClipboard();
		SetClipboardData(CF_BITMAP, hBmp);
		CloseClipboard();
	}

	DeleteObject(hBmp);
	DeleteObject(hMemDc);
	DeleteObject(hScrDc);
}

BOOL CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;
	case WM_COMMAND:
		return OnDlgCommand(hWnd, wParam, lParam);
	case WM_CLOSE:
		CloseWindow(hWnd);
		SendMessage(hWnd, WM_DESTROY, 0, 0);
		return FALSE;
	default:
		return FALSE;
	}
}

BOOL CALLBACK OnDlgCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	int wmEvent = HIWORD(wParam);

	if (wmEvent == BN_CLICKED)
	{
		if (wmId == IDC_NEW)
		{
			if (!IsWindow(g_MainWnd))
			{
				//��ͼ���ڲ����ڣ�����ʾ�������
				if (!InitInstance(hInst, SW_NORMAL))
				{
					return FALSE;
				}
			}
		}

		if (wmId == IDCANCEL)
		{
			if (IsWindow(g_MainWnd))
			{
				//��ͼ���ڴ�����رս�ͼ����
				SendMessage(g_MainWnd, WM_CLOSE, 0, 0);
				return TRUE;
			}
		}
		return TRUE;
	}
	return FALSE;
}