#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include "adl_defines.h"
#include "adl_sdk.h"
#include "adl_structures.h"

bool ForegroundWindowIsCSGO()
{
	// Get foreground window and the process ID of that window.
	HWND handle = GetForegroundWindow();
	DWORD procID = 0;
	GetWindowThreadProcessId(handle, &procID);

	TCHAR ProcessExeName[MAX_PATH] = TEXT("Can't get executable name.");
	// Open the process so we can query information from it.
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procID);

	// We'll get something with most programs, however explorer windows don't seem
	// to return explorer.exe as the executable name. This shouldn't matter for our purposes.
	if (hProcess != NULL)
	{
		HMODULE hMod;
		DWORD cbNeeded;

		// The name of the executable we're hoping to find.
		TCHAR ExpectedProcess[] = _T("cmd.exe");

		// Enumerable the modules of the process and grab the executable name.
		if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
			GetModuleBaseName(hProcess, hMod, ProcessExeName, sizeof(ProcessExeName) / sizeof(TCHAR));

		// Is it the process we were looking for?
		if (_tcscmp(ProcessExeName, ExpectedProcess) == 0)
			printf("Ideal process found - ");

		_tprintf(TEXT("%s  (PID: %u)\n"), ProcessExeName, procID);

		CloseHandle(hProcess);
		return true;
	}
	else
	{
		printf("Can't get process.\n");
		return false;
	}
}

void main(void)
{
	while (true)
	{
		if (ForegroundWindowIsCSGO())
		{
			// do stuff
		}
		Sleep(200);
	}
}

