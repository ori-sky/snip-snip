#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <Windows.h>
#include <WindowsX.h>

HWND hDesktop;
RECT rDesktop;
RECT rRegion;
HBRUSH brW = CreateSolidBrush(RGB(150, 150, 150));
HBRUSH brB = CreateSolidBrush(RGB(0, 0, 0));

HRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;

	switch(uMsg)
	{
	case WM_KEYDOWN:
		switch(wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
		break;
	case WM_LBUTTONDOWN:
		InvalidateRect(hWnd, &rRegion, true);
		rRegion.left = GET_X_LPARAM(lParam);
		rRegion.top = GET_Y_LPARAM(lParam);
		rRegion.right = GET_X_LPARAM(lParam);
		rRegion.bottom = GET_Y_LPARAM(lParam);
		InvalidateRect(hWnd, &rRegion, false);
		break;
	case WM_LBUTTONUP:
		{
			int x = min(rRegion.left, rRegion.right);
			int y = min(rRegion.top, rRegion.bottom);
			int w = abs(rRegion.right - rRegion.left);
			int h = abs(rRegion.top - rRegion.bottom);

			SetLayeredWindowAttributes(hWnd, 0, 0, LWA_ALPHA);

			HDC hdcScreen = GetDC(NULL);
			HDC hdcMem = CreateCompatibleDC(hdcScreen);
			HBITMAP hBmp = CreateCompatibleBitmap(hdcScreen, w, h);
			SelectObject(hdcMem, hBmp);
			BitBlt(hdcMem, 0, 0, w, h, hdcScreen, x, y, SRCCOPY);

			OpenClipboard(NULL);
			EmptyClipboard();
			SetClipboardData(CF_BITMAP, hBmp);
			CloseClipboard();

			DeleteDC(hdcMem);
			DeleteObject(hBmp);
			ReleaseDC(NULL, hdcScreen);
			PostQuitMessage(0);
		}
		break;
	case WM_MOUSEMOVE:
		if(wParam & MK_LBUTTON)
		{
			InvalidateRect(hWnd, &rRegion, true);
			rRegion.right = GET_X_LPARAM(lParam);
			rRegion.bottom = GET_Y_LPARAM(lParam);
			InvalidateRect(hWnd, &rRegion, false);
		}
		break;
	case WM_PAINT:
		FillRect(BeginPaint(hWnd, &ps), &rRegion, brW);
		EndPaint(hWnd, &ps);
	case WM_ERASEBKGND:
		FillRect((HDC)wParam, &rDesktop, brB);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

HWND init(HINSTANCE hInstance, int nCmdShow)
{
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = hInstance;
	wc.lpszClassName = "SnipSnip";
	wc.lpfnWndProc = WindowProc;
	wc.hCursor = LoadCursor(NULL, IDC_CROSS);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&wc);

	hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &rDesktop);

	HWND hWnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_LAYERED | WS_EX_COMPOSITED, "SnipSnip", "Snip Snip", WS_POPUP,
		rDesktop.left, rDesktop.top, rDesktop.right, rDesktop.bottom,
		NULL, NULL, hInstance, NULL);

	if(!hWnd) return hWnd;

	SetLayeredWindowAttributes(hWnd, 0, 100, LWA_ALPHA);
	ShowWindow(hWnd, SW_SHOW);

	/* force active, focus, front, etc */
	DWORD dwCurrentThread = GetCurrentThreadId();
	DWORD dwFGThread = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
	AttachThreadInput(dwCurrentThread, dwFGThread, TRUE);

	SetForegroundWindow(hWnd);
	SetCapture(hWnd);
	SetFocus(hWnd);
	SetActiveWindow(hWnd);
	BringWindowToTop(hWnd);
	EnableWindow(hWnd, TRUE);

	AttachThreadInput(dwCurrentThread, dwFGThread, FALSE);
	SetCursor(LoadCursor(NULL, IDC_CROSS));

	UpdateWindow(hWnd);
	return hWnd;
}

int run(void)
{
	MSG msg = {};
	for(; msg.message != WM_QUIT;)
	{
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
	HWND hWnd = init(hInstance, nCmdShow);
	if(!hWnd) return 0;

	return run();
}