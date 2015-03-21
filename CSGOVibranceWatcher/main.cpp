#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include "adl_defines.h"
#include "adl_sdk.h"
#include "adl_structures.h"

bool ForegroundWindowIsCSGO()
{
	HWND handle = GetForegroundWindow();
	DWORD procID = 0;
	GetWindowThreadProcessId(handle, &procID);

	TCHAR ProcessExeName[MAX_PATH] = TEXT("<unknown>");
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procID);

	if (hProcess != NULL)
	{
		HMODULE hMod;
		DWORD cbNeeded;

		TCHAR ExpectedProcess[] = _T("cmd.exe");

		if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
			GetModuleBaseName(hProcess, hMod, ProcessExeName, sizeof(ProcessExeName) / sizeof(TCHAR));

		if (_tcscmp(ProcessExeName, ExpectedProcess) == 0)
			printf("It worked!");

		_tprintf(TEXT("%s  (PID: %u)\n"), ProcessExeName, procID);

		CloseHandle(hProcess);
		return true;
	}
	else
	{
		printf("Failed!\n");
		return false;
	}
}

void main(void)
{
	ForegroundWindowIsCSGO();
}

