﻿#include "systemclass.h"
#include "math.h"
#include "misc/imguiManager.h"
#include "misc/imgui/imgui.h"
#include "misc/imgui/imgui_impl_win32.h"
#include "misc/imgui/imgui_impl_dx11.h"

#define CHECK_RES(res, str) \
if (res < 0) { perror(str); exit(-1); } \


SystemClass::SystemClass()
{
	m_width = 800;
	m_height = 600;

	d3d = new D3dClass();
	pD3d = std::make_unique<D3dClass>();
	m_imguiManager = std::make_unique<imguiManager>();
}

SystemClass::~SystemClass()
{
	m_imguiManager->ImGuiWin32_Destory();
}

int SystemClass::Init()
{
	int res = -1;

	//1.Window
	res = InitWindow(m_width, m_height);
	CHECK_RES(res, "InitWindow error");

	//2.D3d
	res = pD3d->InitD3d11(this->m_hwnd, m_width, m_height);
	CHECK_RES(res, "InitD3d11 error");

	//init imgui
	m_imguiManager->ImGuiWin32_Init(this->m_hwnd);

	//3.Init Graphics
	//设置视图矩阵
	pD3d->SetProjection(DirectX::XMMatrixPerspectiveLH(1.0f, (float)m_height/(float)m_width, 0.5f, 40.0f));
	pD3d->SetCamera(DirectX::XMMatrixTranslation(0.0f, 0.0f, 20.0f));

#if 1
	std::mt19937 rng(std::random_device{}());
	std::uniform_real_distribution<float> adist(0.0f, 3.1415f * 2.0f);
	std::uniform_real_distribution<float> ddist(0.0f, 3.1415f * 2.0f);
	std::uniform_real_distribution<float> odist(0.0f, 3.1415f * 0.3f);
	std::uniform_real_distribution<float> rdist(6.0f, 20.0f);
	for (auto i = 0; i < 80; i++)
	{
		skinned_boxes.push_back(std::make_unique<SkinnedBox>(
			*pD3d, rng, adist,
			ddist, odist, rdist
			));
	}
	pD3d->SetProjection(DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 40.0f));

#else

	//graphics2D = std::make_unique <Graphics2D>(m_width, m_height);
	//graphics2D->InitTriangle(*pD3d);
	//graphics2D->InitRectWithTexture(*pD3d, L"../data/cheese.jpg");
	//graphics2D->InitCube(*pD3d);
#endif
	return 0;
}

void SystemClass::DoFrame()
{
	//DirectX11
	const auto dt = m_time.Mark() * speed_factor;

#if 0
	//当前画出的图形随着窗口的大小而改变
	//const float c = sin(m_time.Peek()) / 2.0f + 0.5f;
	pD3d->DrawTriangle(m_time.Peek());
	//pD3d->DrawCube(m_time.Peek(), 0, 0);
	//pD3d->DrawCube(-m_time.Peek(), -1.0, 0.0);
	//d3d->DrawRect();
	pD3d->EndFrame();
#else
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	pD3d->ClearBuffer(0.5f, 0.5f, 0.5f);

	pD3d->SetCamera(cam.GetMatrix());
	//auto dt = m_time.Mark();
	for (auto& b : skinned_boxes)
	{
		b->Update(dt);
		b->Draw(*pD3d);
	}

	//graphics2D->Update(m_time.Peek());
	//graphics2D->Draw(*pD3d);

	//Imgui
	//m_imguiManager->ShowImGui();

	static char buffer[1024];
	// imgui window to control simulation speed
	if (ImGui::Begin("Simulation Speed"))
	{
		ImGui::SliderFloat("Speed Factor", &speed_factor, 0.0f, 4.0f);
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::InputText("Butts", buffer, sizeof(buffer));
	}
	ImGui::End();

	cam.SpawnControlWindow();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif
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
			pD3d->ClearBuffer(0.5f, 0.5f, 0.5f);
			DoFrame();
			pD3d->EndFrame();

		}
	}

	return 0;
}

int SystemClass::Shutdown()
{
	int res = -1;

	res = ShutdownWindow();
	CHECK_RES(res, "ShutdownWindow error");

	delete d3d;
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
	//get imgui message
	if (imguiManager::ImGui_WndProcHandler(hwnd, message, wparam, lparam))
	{
		return true;
	}

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