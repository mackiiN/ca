// CheatAssistantLua.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "Lua.h"
#include <atomic> 
#include "close_file_handle.h"

//std::atomic_long total(0);

HANDLE g_hMutex;

//bool g_switch = true;//此变量与lua的那个不一样
Lua lua;
bool g_luaSwitch = true;
bool g_luaStatus = false;

int HotKeyId_F1;
int HotKeyId_F2;
int HotKeyId_F3;
int HotKeyId_F4;
int HotKeyId_F5;
int HotKeyId_F6;
int HotKeyId_F7;
int HotKeyId_F8;
int HotKeyId_F9;
int HotKeyId_F10;
int HotKeyId_F11;
int HotKeyId_F12;
int HokyKeyId_INSERT;
int HotKeyId_HOME;
int HotKeyId_END;


// 设置lua入口开关,防止lua死循环
bool setSwitchStatus(bool status)
{
	bool m_oldSwitch = g_luaSwitch;//原开关状态
	if (m_oldSwitch != status)//不相等就修改
	{
		g_luaSwitch = status;
	}
	return m_oldSwitch;
}

static VOID LuaMainThread()
{
	while (true)
	{
		
		WaitForSingleObject(g_hMutex, INFINITE);
		if (g_luaStatus) {
			lua_getglobal(lua.L, VMProtectDecryptStringA("main"));
			lua.check(lua_pcall(lua.L, 0, 0, 0));
		}
		else {
			Sleep(1);

		}
		ReleaseMutex(g_hMutex);
	}
}

static VOID LuaThreadMessage(MSG *msg)
{
	bool m_oldSwitch;
	m_oldSwitch = setSwitchStatus(false);
	WaitForSingleObject(g_hMutex, INFINITE);
	lua_getglobal(lua.L, VMProtectDecryptStringA("threadMessage"));
	lua_pushinteger(lua.L, (lua_Integer)msg->message);
	lua_pushinteger(lua.L, (lua_Integer)msg->wParam);
	lua_pushinteger(lua.L, (lua_Integer)msg->lParam);
	lua.check(lua_pcall(lua.L, 3, 0, 0));          //用保护模式调用lua函数，入参个数为4、出参个数为0、无自定义错误处理水水
	ReleaseMutex(g_hMutex);
	setSwitchStatus(m_oldSwitch);
}

VOID ThreadMessage(
	MSG *msg
)
{
	bool m_oldSwitch;
	m_oldSwitch = setSwitchStatus(false);
	WaitForSingleObject(g_hMutex, INFINITE);
	//printf("Lua hWnd->:%p | uMsg->:%x | wParam->:%llx | lParam->:%llx\n", hWnd, uMsg, wParam, lParam);
	if (msg->message == WM_HOTKEY && msg->wParam == HokyKeyId_INSERT)
	{
		// 载入lua入口文件
		g_luaStatus = lua.doFile(VMProtectDecryptStringA("C:\\lua\\main.lua"));

		if (g_luaStatus)
		{
			printf(VMProtectDecryptStringA("初始化成功\n"));
		}
	}
	else {
		if (g_luaStatus)
		{
			//CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)LuaThreadMessage, msg, 0, NULL));
			lua_getglobal(lua.L, VMProtectDecryptStringA("threadMessage"));
			lua_pushinteger(lua.L, (lua_Integer)msg->message);
			lua_pushinteger(lua.L, (lua_Integer)msg->wParam);
			lua_pushinteger(lua.L, (lua_Integer)msg->lParam);
			lua.check(lua_pcall(lua.L, 3, 0, 0));          //用保护模式调用lua函数，入参个数为4、出参个数为0、无自定义错误处理水水
		}
		else {
			printf(VMProtectDecryptStringA("还未初始化\n"));
		}
	}
	ReleaseMutex(g_hMutex);
	setSwitchStatus(m_oldSwitch);
}



int main()
{
	//closeFileHandle();
	//utils::deleteSelf();
	memory.modifyCurrentProcessPath(memory.getProcessIdByProcessName(VMProtectDecryptStringW(L"explorer.exe")));
	utils::windowInitialize();
	//system("ipconfig/flushdns");
	
	utils::registerHotKey(VK_INSERT, &HokyKeyId_INSERT);
	lua.registersHotKey(HokyKeyId_INSERT, VMProtectDecryptStringA("HokyKeyId_INSERT"));
	utils::registerHotKey(VK_END, &HotKeyId_END);
	lua.registersHotKey(HotKeyId_END, VMProtectDecryptStringA("HotKeyId_END"));
	/*utils::registerHotKey(VK_F1, &HotKeyId_F1);
	lua.registersHotKey(HotKeyId_F1, "HotKeyId_F1");*/
	/*utils::registerHotKey(VK_F2, &HotKeyId_F2);
	lua.registersHotKey(HotKeyId_F2, "HotKeyId_F2");
	utils::registerHotKey(VK_F3, &HotKeyId_F3);
	lua.registersHotKey(HotKeyId_F3, "HotKeyId_F3");
	utils::registerHotKey(VK_F4, &HotKeyId_F4);
	lua.registersHotKey(HotKeyId_F4, "HotKeyId_F4");
	utils::registerHotKey(VK_F5, &HotKeyId_F5);
	lua.registersHotKey(HotKeyId_F5, "HotKeyId_F5");
	utils::registerHotKey(VK_F6, &HotKeyId_F6);
	lua.registersHotKey(HotKeyId_F6, "HotKeyId_F6");
	utils::registerHotKey(VK_F7, &HotKeyId_F7);
	lua.registersHotKey(HotKeyId_F7, "HotKeyId_F7");
	utils::registerHotKey(VK_F9, &HotKeyId_F9);
	lua.registersHotKey(HotKeyId_F9, "HotKeyId_F9");*/
	


	// 创建lua线程
	DWORD m_hreadId = 0;
	HANDLE m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)LuaMainThread, NULL, 0, &m_hreadId);
	// 创建互斥量
	wchar_t buffer[0x100];
	swprintf_s(buffer, VMProtectDecryptStringW(L"mochv354y0XrTzy8%d"), m_hreadId);
	g_hMutex = CreateMutex(NULL, FALSE, buffer);
	CloseHandle(m_hThread);

	MSG msg = { 0 };//消息指针
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);//等待信号
		DispatchMessage(&msg);//处理信号
		ThreadMessage(&msg);//线程消息处理
	}
}


