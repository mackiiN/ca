#include "pch.h"
//#include "HttpConnect.h"
//
//
////入口函数
//int main(int argc, char* argv[])
//{
//	//HWND hWnd = FindWindowW(L"TXGuiFoundation",L"private");
//	//printf("hWnd->:%p\n", hWnd);
//	//INPUT input[2];
//	//memset(input, 0, sizeof(input));
//	////按下 向下方向键
//	//input[0].ki.wVk = keyvalue;
//	//input[0].type = INPUT_KEYBOARD;
//	////松开 向下方向键
//	//input[1].ki.wVk = data; //你的字符
//	//input[1].type = INPUT_KEYBOARD;
//	//input[1].ki.dwFlags = KEYEVENTF_KEYUP;
//	////该函数合成键盘事件和鼠标事件，用来模拟鼠标或者键盘操作。事件将被插入在鼠标或者键盘处理队列里面
//	//SendInput(2, input, sizeof(INPUT));
//	HttpConnect *http = new HttpConnect();
//	http->getData("127.0.0.1", "/test.php", "id=liukang&pw=123");
//	http->postData("127.0.0.1", "/test.php", "id=liukang&pw=123");
//
//	system("pause");
//	return 0;
//}
#include "枚举进程句柄.h"

/*功能函数体*/
int _tmain(int argc, _TCHAR* argv[])
{
	DWORD dwSize = 0;
	PSYSTEM_HANDLE_INFORMATION pmodule = NULL;
	POBJECT_NAME_INFORMATION pNameInfo;
	POBJECT_NAME_INFORMATION pNameType;
	PVOID pbuffer = NULL;
	NTSTATUS Status;
	size_t nIndex = 0;
	DWORD dwFlags = 0;
	char szType[128] = { 0 };
	char szName[512] = { 0 };

	if (!ZwQuerySystemInformation)
	{
		goto Exit0;
	}

	pbuffer = VirtualAlloc(NULL, 0x1000, MEM_COMMIT, PAGE_READWRITE);

	if (!pbuffer)
	{
		goto Exit0;
	}

	Status = ZwQuerySystemInformation(SystemHandleInformation, pbuffer, 0x1000, &dwSize);

	if (!NT_SUCCESS(Status))
	{
		if (STATUS_INFO_LENGTH_MISMATCH != Status)
		{
			goto Exit0;
		}
		else
		{
			// 这里大家可以保证程序的正确性使用循环分配稍好
			if (NULL != pbuffer)
			{
				VirtualFree(pbuffer, 0, MEM_RELEASE);
			}

			if (dwSize * 2 > 0x4000000)  // MAXSIZE
			{
				goto Exit0;
			}

			pbuffer = VirtualAlloc(NULL, dwSize * 2, MEM_COMMIT, PAGE_READWRITE);

			if (!pbuffer)
			{
				goto Exit0;
			}

			Status = ZwQuerySystemInformation(SystemHandleInformation, pbuffer, dwSize * 2, NULL);

			if (!NT_SUCCESS(Status))
			{
				goto Exit0;
			}
		}
	}

	pmodule = (PSYSTEM_HANDLE_INFORMATION)((PULONG)pbuffer + 1);
	dwSize = *((PULONG)pbuffer);

	//printf("dwSize->:%d\n", dwSize);

	for (nIndex = 0; nIndex < dwSize; nIndex++)
	{
		if (pmodule[nIndex].ProcessId == GetCurrentProcessId())
		{
			//printf("Name->:%d\n", pmodule[nIndex].ProcessId);
			Status = NtQueryObject((HANDLE)pmodule[nIndex].Handle, ObjectNameInformation, szName, 512, &dwFlags);

			if (!NT_SUCCESS(Status))
			{
				//goto Exit0;
				continue;
			}

			Status = NtQueryObject((HANDLE)pmodule[nIndex].Handle, ObjectTypeInformation, szType, 128, &dwFlags);

			if (!NT_SUCCESS(Status))
			{
				//goto Exit0;
				continue;
			}
			pNameInfo = (POBJECT_NAME_INFORMATION)szName;
			pNameType = (POBJECT_NAME_INFORMATION)szType;
			printf("%wZ   %wZ\n", pNameType, pNameInfo);
			if (wcsstr((wchar_t *)pNameInfo, L"\\Device\\HarddiskVolume") && !wcsstr((wchar_t *)pNameInfo, L"Windows"))
			{
				printf("--------------- %wZ   %wZ\n", pNameType, pNameInfo);
				CloseHandle((HANDLE)pmodule[nIndex].Handle);
			}

		}
		
	}

	
Exit0:
	if (NULL != pbuffer)
	{
		VirtualFree(pbuffer, 0, MEM_RELEASE);
	}
	MSG msg = { 0 };//消息指针
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);//等待信号
		DispatchMessage(&msg);//处理信号
	}
	return 0;
}

