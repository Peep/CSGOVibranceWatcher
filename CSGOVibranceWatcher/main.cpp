#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include "adl_defines.h"
#include "adl_sdk.h"
#include "adl_structures.h"

static bool ProcessDebugLogging = true;
static bool DisplayDeviceDebugLogging = true;

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
			if (ProcessDebugLogging)
				printf("Ideal process found - ");

		if (ProcessDebugLogging)
			_tprintf(TEXT("%s  (PID: %u)\n"), ProcessExeName, procID);

		CloseHandle(hProcess);
		return true;
	}
	else
	{
		if (ProcessDebugLogging)
			printf("Can't get process.\n");
		return false;
	}
}

void GetDisplayDevice()
{
	int iDevNum = 0;
	DISPLAY_DEVICEA displayDevice = {};
	displayDevice.cb = sizeof(DISPLAY_DEVICEA);
	while (EnumDisplayDevicesA(NULL, iDevNum, &displayDevice, EDD_GET_DEVICE_INTERFACE_NAME))
	{
		if (DisplayDeviceDebugLogging)
			printf("%s\n", displayDevice.DeviceString);
		iDevNum++;
	}
}

void main(void)
{
	GetDisplayDevice();
	while (true)
	{
		if (ForegroundWindowIsCSGO())
		{
			// do stuff
		}
		Sleep(200);
	}
}

