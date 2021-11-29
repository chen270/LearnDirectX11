#include "systemclass.h"
#include "math.h"

#define CHECK_RES(res, str) \
if (res < 0) { perror(str); exit(-1); } \


std::unique_ptr<DirectX::Mouse> SystemClass::m_mouse = nullptr;

SystemClass::SystemClass()
{
	m_width = 800;
	m_height = 600;

	d3d = std::make_unique<D3dClass>();

	m_mouse = std::make_unique<DirectX::Mouse>();
	m_MouseTracker = std::make_unique<DirectX::Mouse::ButtonStateTracker>();
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

	// Mouse
	m_mouse->SetWindow(this->m_hwnd);
	m_mouse->SetMode(DirectX::Mouse::MODE_ABSOLUTE);

	//3.Init Resource
	//d3d->InitResource();
	d3d->InitCubeResource();
	//d3d->InitTriangleResource();
	//d3d->InitComputeShaderResource();
	//d3d->UseComputeShader();

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
			// 获取鼠标状态
			DirectX::Mouse::State mouseState = m_mouse->GetState();
			DirectX::Mouse::State lastMouseState = m_MouseTracker->GetLastState();
			
			// 更新鼠标按钮状态跟踪器，仅当鼠标按住的情况下才进行移动
			float cubeTheta = 0, cubePhi = 0;
			m_MouseTracker->Update(mouseState);
			if (mouseState.leftButton == true && m_MouseTracker->leftButton == m_MouseTracker->HELD)
			{
				cubeTheta -= (mouseState.x - lastMouseState.x) * 0.01f;
				cubePhi -= (mouseState.y - lastMouseState.y) * 0.01f;
			}
			d3d->UpdateScene(cubeTheta, cubePhi);
			d3d->DrawScene();
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
		m_mouse->ProcessMessage(message, wparam, lparam);
		return 0;


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