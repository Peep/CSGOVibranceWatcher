#include <windows.h>
#include <tchar.h>
#include <psapi.h>

#include "win32_amd.h"

#define AMD 0
#define NVIDIA 1

static int PrimaryDisplay = 0;
static int DisplayVendor = 0;

bool ForegroundWindowIsCSGO()
{
	auto handle = GetForegroundWindow();
	DWORD procID = 0;
	GetWindowThreadProcessId(handle, &procID);

	TCHAR ProcessExeName[MAX_PATH] = TEXT("Can't get executable name.");
	auto hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procID);

	if (hProcess != nullptr)
	{
		HMODULE hMod;
		DWORD cbNeeded;

		TCHAR ExpectedProcess[] = _T("csgo.exe");

		if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
			GetModuleBaseName(hProcess, hMod, ProcessExeName, sizeof(ProcessExeName) / sizeof(TCHAR));

		if (_tcscmp(ProcessExeName, ExpectedProcess) == 0)
		{
			CloseHandle(hProcess);
			return true;
		}
		CloseHandle(hProcess);
		return false;
	}
	return false;
}

void GetDisplayDevice()
{
	auto device = 0;
	DISPLAY_DEVICEA displayDevice = {};
	displayDevice.cb = sizeof(DISPLAY_DEVICEA);

	while (EnumDisplayDevicesA(nullptr, device, &displayDevice, EDD_GET_DEVICE_INTERFACE_NAME))
	{
		if (strstr(displayDevice.DeviceString, "AMD") != nullptr)
		{
			DisplayVendor = AMD;
			break;
		}
		if (strstr(displayDevice.DeviceString, "NVIDIA") != nullptr)
		{
			DisplayVendor = NVIDIA;
			break;
		}
		device++;
	}
}

int main()
{
	GetDisplayDevice();
	if (DisplayVendor == AMD)
	{
		AMDProgramLoop();
	}
	if (DisplayVendor == NVIDIA)
	{
		// TODO
	}
}

