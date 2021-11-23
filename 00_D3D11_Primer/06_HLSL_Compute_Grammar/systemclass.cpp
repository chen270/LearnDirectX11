#include "systemclass.h"
#include "math.h"

#define CHECK_RES(res, str) \
if (res < 0) { perror(str); exit(-1); } \


SystemClass::SystemClass()
{
	m_width = 800;
	m_height = 600;

	d3d = std::make_unique<D3dClass>();
}

SystemClass::~SystemClass()
{
}

int SystemClass::Init()
{
	int res = -1;

	//1.Window
	res = InitWindow(m_width, m_height);
	CHECK_RES(res, "InitWindow error");

	//2.D3d
	res = d3d->InitD3d11(this->m_hwnd, m_width, m_height);
	CHECK_RES(res, "InitD3d11 error");

	//3.Init Resource
	//d3d->InitResource();
	//d3d->InitComputeShaderResource();
	d3d->UseComputeShader();

	return 0;
}


int SystemClass::Run()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// If windows signals to end the application then exit out.
		if (msg.message == WM_QUIT)
			break;
		else
		{
			//DirectX11
#if 0
			//当前画出的图形随着窗口的大小而改变
			//const float c = sin(m_time.Peek()) / 2.0f + 0.5f;
			d3d->ClearBuffer(0.5f, 0.5f, 0.5f);
			//d3d->DrawTriangle(m_time.Peek());
			d3d->DrawCube(m_time.Peek(), 0, 0);
			//d3d->DrawCube(-m_time.Peek(),-1.0,0.0);
			//d3d->DrawRect();
			d3d->EndFrame();
#else
			//d3d->DrawScene();
			//d3d->Compute();
			//d3d->UseComputeShader();
#endif

		}
	}

	return 0;
}

int SystemClass::Shutdown()
{
	int res = -1;

	res = ShutdownWindow();
	CHECK_RES(res, "ShutdownWindow error");

	return 0;
}


int SystemClass::InitWindow(const int& width, const int& height)
{
	WNDCLASSEX wc;
	int posX, posY;

	m_hinstance = GetModuleHandle(NULL);
	m_applicationName = L"MyWindow";

	// Setup the windows class with default settings.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;// (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	ATOM atom = RegisterClassEx(&wc);
	if (!atom)
		return -1;

	// Place the window in the middle of the screen.
	posX = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
	posY = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

	// Create the window with the screen settings and get the handle to it.
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName,
		WS_OVERLAPPEDWINDOW,//WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		posX, posY, width, height, NULL, NULL, m_hinstance, NULL);
	if (!m_hwnd)
		return -1;

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	return 0;
}

int SystemClass::ShutdownWindow()
{
	// Show the mouse cursor.
	ShowCursor(true);

	// Remove the window.
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// Remove the application instance.
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	return 0;
}


LRESULT CALLBACK SystemClass::WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
		// Check if the window is being destroyed.
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}

	// Check if the window is being closed.
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}

	// All other messages pass to the message handler in the system class.
	default:
		return DefWindowProc(hwnd, message, wparam, lparam);
	}
}