#include "imguiManager.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


imguiManager::imguiManager(/* args */)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
}

imguiManager::~imguiManager()
{
    ImGui::DestroyContext();
    //ImGui_ImplDX11_Shutdown();//销毁
}

void imguiManager::ImGuiWin32_Init(HWND hWnd)
{
    ImGui_ImplWin32_Init(hWnd);
}

void imguiManager::ImGuiWin32_Destory()
{
	ImGui_ImplWin32_Shutdown();
}

void imguiManager::ShowImGui()
{
    //imgui stuff
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    static bool show_demo_window = true;
    if (show_demo_window)
    {
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

LRESULT imguiManager::ImGui_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}

void imguiManager::ImGuiDX11_Init(void* pDevice, void* pContext)
{
    ImGui_ImplDX11_Init(static_cast<ID3D11Device*>(pDevice), static_cast<ID3D11DeviceContext*>(pContext));
}