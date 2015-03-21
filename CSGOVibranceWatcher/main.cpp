#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include "adl_defines.h"
#include "adl_sdk.h"
#include "adl_structures.h"

void TryFindCSGOHandle()
{
	HWND foregroundWindowHandle = GetForegroundWindow();
	LPDWORD procID = 0;
	DWORD processID = GetWindowThreadProcessId(foregroundWindowHandle, procID);
	DWORD chrome = 6516;
	TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, chrome);

	if (hProcess != NULL)
	{
		HMODULE hMod;
		DWORD cbNeeded;

		if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
			GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));

		_tprintf(TEXT("%s  (PID: %u)\n"), szProcessName, processID);

		CloseHandle(hProcess);
	}
	else
		printf("Failed!\n");
}

void main(void)
{
	TryFindCSGOHandle();
}

