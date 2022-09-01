#include "systemclass.h"
#include "math.h"

#define CHECK_RES(res, str) \
if (res < 0) { perror(str); exit(-1); } \


std::unique_ptr<DirectX::Mouse> SystemClass::m_mouse = nullptr;
std::unique_ptr<DirectX::Keyboard> SystemClass::m_keyboard = nullptr;

SystemClass::SystemClass()
{
	m_width = 800;
	m_height = 600;

	d3d = std::make_unique<D3dClass>();

	m_mouse = std::make_unique<DirectX::Mouse>();

	m_keyboard = std::make_unique<DirectX::Keyboard>();
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

	//2.d2d
	res = d3d->InitDirect2D();
	CHECK_RES(res, "InitDirect2D error");

	//3.D3d
	res = d3d->InitD3d11_DXGI(this->m_hwnd, m_width, m_height);
	CHECK_RES(res, "InitD3d11 error");



	// Mouse & Keyboard
	//m_mouse->SetWindow(this->m_hwnd);
	//m_mouse->SetMode(DirectX::Mouse::MODE_ABSOLUTE);

#if 1 //自写
	d3d->InitShader_CompileInRunTime(L"../shader/Light_VS.hlsl", L"../shader/Light_PS.hlsl");
#else //参考
	auto r = d3d->InitEffect();
#endif // 1

	//3.Init Resource
	//d3d->InitResource();
	//d3d->InitCubeResource();
	d3d->InitLightResource();

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
			//当前画出的图形随着窗口的大小而改变
			d3d->ClearBuffer(0.0f, 0.0f, 0.0f);
			d3d->UpdateScene(m_time.Peek(), m_keyboard->GetState());
			//d3d->ClearBuffer(0.5f, 0.5f, 0.5f);

			d3d->DrawScene();
			d3d->DrawDWrite();
			d3d->EndFrame();
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


LRESULT CALLBACK SystemClass::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
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

	// 监测这些键盘/鼠标事件
	case WM_INPUT:

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_XBUTTONDOWN:

	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	case WM_XBUTTONUP:

	case WM_MOUSEWHEEL:
	case WM_MOUSEHOVER:
	case WM_MOUSEMOVE:
		m_mouse->ProcessMessage(msg, wParam, lParam);
		return 0;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		m_keyboard->ProcessMessage(msg, wParam, lParam);
		return 0;

	case WM_ACTIVATEAPP:
		m_mouse->ProcessMessage(msg, wParam, lParam);
		m_keyboard->ProcessMessage(msg, wParam, lParam);
		return 0;

	// All other messages pass to the message handler in the system class.
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}