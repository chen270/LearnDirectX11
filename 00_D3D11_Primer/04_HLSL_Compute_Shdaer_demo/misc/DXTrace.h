﻿#ifndef __DXTRACE_H__
#define __DXTRACE_H__

#include "windows.h"
#include "misc/dxgiInfoManager.h"

class DXTrace
{
public:
	// ------------------------------
	// DXTraceW函数
	// ------------------------------
	// 在调试输出窗口中输出格式化错误信息，可选的错误窗口弹出(已汉化)
	// [In]strFile			当前文件名，通常传递宏__FILEW__
	// [In]hlslFileName     当前行号，通常传递宏__LINE__
	// [In]hr				函数执行出现问题时返回的HRESULT值
	// [In]strMsg			用于帮助调试定位的字符串，通常传递L#x(可能为NULL)
	// [In]bPopMsgBox       如果为TRUE，则弹出一个消息弹窗告知错误信息
	// 返回值: 形参hr
	static HRESULT WINAPI DXTraceW(_In_z_ const WCHAR* strFile, _In_ DWORD dwLine, _In_ HRESULT hr, _In_opt_ const WCHAR* strMsg, _In_ bool bPopMsgBox);
	
public:

	static void DXTraceExcep(_In_z_ const WCHAR* file, _In_ DWORD dwLine, std::vector<std::string> infoMsgs)noexcept;

#if defined(DEBUG) || defined(_DEBUG)  
	static DxgiInfoManager infoManager;
#endif

private:
	//int line;
	//std::string file;
};





// ------------------------------
// HR宏
// ------------------------------
// Debug模式下的错误提醒与追踪
#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)												\
	{															\
		DXTrace::infoManager.Set();							\
		HRESULT hr_tmp = (x);										\
		if(FAILED(hr_tmp))											\
		{														\
			DXTrace::DXTraceW(__FILEW__, (DWORD)__LINE__, hr_tmp, L#x, true);\
		}														\
	}
#endif
#else
#ifndef HR
#define HR(x) (x)
#endif 
#endif


// ------------------------------
// THROW_D3D_EXCEPTION 宏
// ------------------------------
// Debug模式下查找, D3D中没有返回hr的函数, 发生错误的情况
#if defined(DEBUG) | defined(_DEBUG)
#ifndef THROW_D3D_EXCEPTION
#define THROW_D3D_EXCEPTION(call)												\
 DXTrace::infoManager.Set();  \
 (call);             \
 {auto v = DXTrace::infoManager.GetMessages(); if(!v.empty()) { DXTrace::DXTraceExcep(__FILEW__, (DWORD)__LINE__,v);}}
#endif
#else
#ifndef THROW_D3D_EXCEPTION
#define THROW_D3D_EXCEPTION(x) (x)
#endif 
#endif


#endif //__DXTRACE_H__