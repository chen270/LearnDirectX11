#ifndef __IMGUIMANAGER_H__
#define __IMGUIMANAGER_H__

#include <windows.h>


class imguiManager
{
private:
    /* data */
public:
    imguiManager(/* args */);
    ~imguiManager();

public:
	void ImGuiWin32_Init(HWND hWnd);
	void ImGuiWin32_Destory();
	void ShowImGui();

public:
	static LRESULT ImGui_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static void ImGuiDX11_Init(void* pDevice, void* pContext);
};




#endif // __IMGUIMANAGER_H__