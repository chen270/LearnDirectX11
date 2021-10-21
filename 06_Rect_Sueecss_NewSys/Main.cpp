#include <windows.h>
#include <stdio.h>
#include "d3dApp.h"
//#include "mpv/client.h"

#include "d3dDemo.h"

static void die(const char* msg)
{
	printf("die - %s\n", msg);
	exit(-1);
}


//回调函数
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(0); //WM_QUIT
		break;
	default:
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool InitMainWindowEx(HWND & hwnd)
{
	HINSTANCE hInstance = GetModuleHandle(NULL);//获取实例句柄
	//register window
	WNDCLASSEX wndclass;
	wndclass.cbClsExtra = 0;//窗口额外存储空间
	wndclass.cbSize = sizeof(wndclass);
	wndclass.cbWndExtra = 0;
	wndclass.hbrBackground = NULL;
	wndclass.hIcon = NULL;//图标
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);//鼠标箭头
	wndclass.hIconSm = NULL;
	wndclass.hInstance = hInstance;
	wndclass.lpfnWndProc = WndProc;// WndProc;//消息响应函数，回调函数
	wndclass.lpszClassName = L"Window";
	wndclass.lpszMenuName = NULL;
	wndclass.style = CS_VREDRAW | CS_HREDRAW;//水平和垂直重绘

	ATOM atom = RegisterClassEx(&wndclass);
	if (!atom)
		die("RegisterClassEx error");

	//create window
	hwnd = CreateWindowEx(NULL, L"Window", L"Window", WS_OVERLAPPEDWINDOW,
		100, 100, 600, 600, NULL, NULL, hInstance, NULL);//起始位置（100，100） 宽度800，高度600

	if (!hwnd)
		die("CreateWindowEx error");

	ShowWindow(hwnd, SW_RESTORE);
	return true;
}

#if 0
#define Check_MPV(res, str) \
	if(res < 0) \
		die(str); \

int MpvPlay(HWND hwnd)
{
	int res = -1;
	mpv_handle *pMpv = mpv_create();
	if(!pMpv)
		die("mpv_create error");

	res = mpv_initialize(pMpv);
	Check_MPV(res,"mpv_initialize error");

	res = mpv_set_property_string(pMpv, "vo", "gpu");
	Check_MPV(res, "vo gpu error");
	res = mpv_set_property_string(pMpv, "gpu-context", "d3d11");
	Check_MPV(res, "gpu-context d3d11 error");
	res = mpv_set_property_string(pMpv, "gpu-api", "d3d11");
	Check_MPV(res, "gpu-api d3d11 error");

	res = mpv_set_option(pMpv, "wid", MPV_FORMAT_INT64, (void*)&hwnd);//传入句柄
	Check_MPV(res, "mpv_set_option wid error");

	const char * path = "C:/Users/dell/Desktop/video/a.mp4";
	const char* cmd[] = {"loadfile", path, NULL};

	res = mpv_command(pMpv, cmd);
	Check_MPV(res, "mpv play video error");

	// while(1)
	// {
	// 	mpv_event* event = mpv_wait_event(pMpv, 1000);
	// 	if(event->event_id == MPV_EVENT_SHUTDOWN)
	// 		break;
	// }

	// mpv_terminate_destroy(pMpv);
	return 1;
}

#endif


int main()
{
	HWND hwnd;

	if (!InitMainWindowEx(hwnd))
		die("InitMainWindow error");

	//MpvPlay(hwnd);
	D3dDemo d3d(hwnd);
	

	float color[] = { 0.1f , 0.2f , 0.5f , 1.0f };
	d3d.SetTex();
	//d3d.CreateFbo();

	MSG msg;
	while(1)
	{
		if(PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			//处理消息
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else{
			UpdateWindow(hwnd);

			//d3d.DrawScene();

			d3d.pImmediateContext->ClearRenderTargetView(d3d.pD3D11RenderTargetView, color);
			d3d.pImmediateContext->VSSetShader(d3d.pVertexShader, nullptr, 0);	//set vs
			d3d.pImmediateContext->PSSetShader(d3d.pPixelShader, nullptr, 0);	//set ps
			//
			
			//d3d.pImmediateContext->Draw(3, 0);		//draw
			d3d.pImmediateContext->DrawIndexed(6,0, 0);		//draw
			d3d.pIdxgiSwapChain->Present(0, 0);
		}
	}

	DeleteObject(hwnd); //doing it just in case

	// D3DApp d3dApp(hwnd);
	// if (!d3dApp.Init())
	// 	return 0;

	// return d3dApp.Run();

	printf("end\n");
	return 0;
}