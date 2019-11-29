//----------------------------------------------------------------------------------------------
// StdAfx.cpp
//----------------------------------------------------------------------------------------------
#include "StdAfx.h"

#define WINDOWSFORM_CLASS	L"MFMultiVideo"
#define PLAYER_COUNT		16
#define PLAYER_PER_LINE		4
#define MAX_PLAYER_PLAY		4
#define VIDEO_INPUT_FILE	L"big_buck_bunny_240p_5mb.mp4"
#define VIDEO_INPUT_TIME	60000

HWND g_hWnd = NULL;
HWND g_hWndPlayers[PLAYER_COUNT] = {0};
CPlayer* g_pPlayer[PLAYER_COUNT] = {0};

BOOL InitInstance(HINSTANCE, int);
void AutoStartPlayback();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
	if(!InitInstance(hInstance, nCmdShow))
	{
		return -1;
	}

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	while(GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if(IsWindow(g_hWnd)){
		DestroyWindow(g_hWnd);
	}

	UnregisterClass(WINDOWSFORM_CLASS, hInstance);

	MFShutdown();
	CoUninitialize();

	return 0;
}

BOOL InitInstance(HINSTANCE hInst, int nCmdShow)
{
	WNDCLASSEX wcex;

	ZeroMemory(&wcex, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInst;
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = WINDOWSFORM_CLASS;

	if(RegisterClassEx(&wcex) == 0)
	{
		return FALSE;
	}

	// Create the application window.
	g_hWnd = CreateWindow(WINDOWSFORM_CLASS, WINDOWSFORM_CLASS, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInst, NULL);

	if(g_hWnd == 0)
	{
		return FALSE;
	}

	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);

	CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
	MFStartup(MF_VERSION, MFSTARTUP_LITE);

	for(int i = 0; i < PLAYER_COUNT; i++)
	{
		g_hWndPlayers[i] = CreateWindowEx(0, WINDOWSFORM_CLASS, L"Player Window", WS_VISIBLE | WS_CHILD, (i % PLAYER_PER_LINE) * 164, int((i / PLAYER_PER_LINE)) * 117, 164, 117, g_hWnd, (HMENU)1, hInst, NULL);

		assert(g_hWndPlayers[i] != NULL);

		HRESULT hr = CPlayer::CreateInstance(g_hWndPlayers[i], i, &g_pPlayer[i]);

		assert(hr == S_OK && g_pPlayer[i] != NULL);
	}

	std::thread t(AutoStartPlayback);
	t.detach();

	return TRUE;
}

void AutoStartPlayback()
{
	int iCount = 0;

	do
	{
		for(int i = 0; i < PLAYER_COUNT; i++)
		{
			LOG_HRESULT(g_pPlayer[i]->OpenUrl(VIDEO_INPUT_FILE));
		}

		Sleep(VIDEO_INPUT_TIME);

		for(int i = 0; i < PLAYER_COUNT; i++)
		{
			LOG_HRESULT(g_pPlayer[i]->Shutdown());
		}

		iCount++;
	}
	while(iCount < MAX_PLAYER_PLAY);

	for(int i = 0; i < PLAYER_COUNT; i++)
	{
		LOG_HRESULT(g_pPlayer[i]->Shutdown());
		SAFE_RELEASE(g_pPlayer[i]);
	}

	for(int i = 0; i < PLAYER_COUNT; i++)
	{
		DestroyWindow(g_hWndPlayers[i]);
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_ERASEBKGND:
			return 1L;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hwnd, message, wParam, lParam);
	}

	return 0L;
}