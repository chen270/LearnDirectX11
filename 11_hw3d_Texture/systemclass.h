﻿#ifndef __SYSTEMCLASS_H__
#define __SYSTEMCLASS_H__

#include <windows.h>
#include "d3dclass.h"
#include "ChiliTimer.h"
#include <vector>
#include <memory>
#include "box.h"
#include "graphics2D.h"
#include "skinned_box.h"

class SystemClass
{
public:
	SystemClass();
	~SystemClass() = default;
	
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

	std::unique_ptr<D3dClass> pD3d;
	D3dClass* d3d;

	std::vector<std::unique_ptr<class Box>> boxes;
	std::vector<std::unique_ptr<class SkinnedBox>> skinned_boxes;
	std::unique_ptr<class Graphics2D> graphics2D;
};

#endif //__SYSTEMCLASS_H__