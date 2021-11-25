#ifndef __SYSTEMCLASS_H__
#define __SYSTEMCLASS_H__

#include <windows.h>
#include "d3dclass.h"
#include "ChiliTimer.h"
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
	//D3dClass* d3d;
};

#endif //__SYSTEMCLASS_H__