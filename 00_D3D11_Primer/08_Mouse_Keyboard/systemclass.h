#ifndef __SYSTEMCLASS_H__
#define __SYSTEMCLASS_H__

#include <windows.h>
#include "d3dclass.h"
#include "ChiliTimer.h"
#include "misc/m_k/mouse.h"
#include "misc/m_k/keyboard.h"
#include <memory>

class SystemClass
{
public:
	SystemClass();
	~SystemClass();
	
	int Init();
	int Shutdown();
	int Run();

private:
	int InitWindow(const int& width, const int& height);
	int ShutdownWindow();
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
private:
	//tiem
	ChiliTimer m_time;

	LPCWSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;
	int m_width;
	int m_height;

	std::unique_ptr<D3dClass>d3d;
	std::unique_ptr<DirectX::Mouse::ButtonStateTracker>m_MouseTracker;
	static std::unique_ptr<DirectX::Mouse>m_mouse;

	std::unique_ptr<DirectX::Keyboard::KeyboardStateTracker>m_KeyboardTracker;
	static std::unique_ptr<DirectX::Keyboard>m_keyboard;

	//D3dClass* d3d;
};

#endif //__SYSTEMCLASS_H__