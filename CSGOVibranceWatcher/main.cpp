#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <string>
#include <vector>
#include <iostream>
#include "adl_defines.h"
#include "adl_sdk.h"
#include "adl_structures.h"

using namespace std;

#define AMD 0
#define NVIDIA 1

typedef int(*ADL_MAIN_CONTROL_CREATE)(ADL_MAIN_MALLOC_CALLBACK, int);
typedef int(*ADL_MAIN_CONTROL_DESTROY)();
typedef int(*ADL_ADAPTER_PRIMARY_GET) (int*);
typedef int(*ADL_DISPLAY_COLOR_GET) (int, int, int, int *, int *, int *, int *, int *);
typedef int(*ADL_DISPLAY_COLOR_SET) (int, int, int, int);
typedef int(*ADL_DISPLAY_DISPLAYINFO_GET) (int, int *, ADLDisplayInfo **, int);

ADL_MAIN_CONTROL_CREATE			 ADL_Main_Control_Create;
ADL_MAIN_CONTROL_DESTROY		 ADL_Main_Control_Destroy;
ADL_ADAPTER_PRIMARY_GET			 ADL_Adapter_Primary_Get;
ADL_DISPLAY_COLOR_GET			 ADL_Display_Color_Get;
ADL_DISPLAY_COLOR_SET			 ADL_Display_Color_Set;
ADL_DISPLAY_DISPLAYINFO_GET		 ADL_Display_DisplayInfo_Get;

static int DisplayVendor;
static bool ProcessDebugLogging = true;
static bool DisplayDeviceDebugLogging = true;

// Memory allocation function
void* __stdcall ADL_Main_Memory_Alloc(int iSize)
{
	void* lpBuffer = malloc(iSize);
	return lpBuffer;
}

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
		TCHAR ExpectedProcess[] = _T("csgo.exe");

		// Enumerable the modules of the process and grab the executable name.
		if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
			GetModuleBaseName(hProcess, hMod, ProcessExeName, sizeof(ProcessExeName) / sizeof(TCHAR));

		// Is it the process we were looking for?
		if (_tcscmp(ProcessExeName, ExpectedProcess) == 0)
		{
			return true;
			CloseHandle(hProcess);
		}
		else
		{
			return false;
			CloseHandle(hProcess);
		}

		if (ProcessDebugLogging)
			_tprintf(TEXT("%s  (PID: %u)\n"), ProcessExeName, procID);
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
		if (strstr(displayDevice.DeviceString, "AMD") != NULL)
		{
			if (DisplayDeviceDebugLogging)
				printf("AMD card found!\n");
			DisplayVendor = AMD;
			break;
		}
		if (strstr(displayDevice.DeviceString, "NVIDIA") != NULL)
		{
			if (DisplayDeviceDebugLogging)
				printf("NVIDIA card found!\n");
			DisplayVendor = NVIDIA;
			break;
		}

		if (DisplayDeviceDebugLogging)
			printf("%s\n", displayDevice.DeviceString);
		iDevNum++;
	}
}

int GetPrimaryDisplayIndex()
{
	int primary;
	ADL_Adapter_Primary_Get(&primary);
	int displayCount;
	ADLDisplayInfo *displayInfo = NULL;

	ADL_Display_DisplayInfo_Get(primary, &displayCount, &displayInfo, 0);
	return displayInfo[0].displayID.iDisplayLogicalIndex;
}

int GetCurrentSaturation(int displayNum)
{
	int curSat, def, minSat, maxSat, step;
	ADL_Display_Color_Get(displayNum, GetPrimaryDisplayIndex(), 
		ADL_DISPLAY_COLOR_SATURATION, &curSat, &def, &minSat, &maxSat, &step);
	return curSat;
}

int main(void)
{
	GetDisplayDevice();
	if (DisplayVendor == AMD)
	{
		HINSTANCE hDLL = LoadLibraryA("atiadlxx.dll");
		if (hDLL == NULL) // try 32 bit dll
			hDLL = LoadLibraryA("atiadlxy.dll");
		if (hDLL == NULL)
		{
			printf("ADL library not found!\n"); 
			return 0;
		}

		ADL_Display_Color_Get = (ADL_DISPLAY_COLOR_GET)GetProcAddress(hDLL, "ADL_Display_Color_Get");
		ADL_Display_Color_Set = (ADL_DISPLAY_COLOR_SET)GetProcAddress(hDLL, "ADL_Display_Color_Set");
		ADL_Adapter_Primary_Get = (ADL_ADAPTER_PRIMARY_GET)GetProcAddress(hDLL, "ADL_Adapter_Primary_Get");
		ADL_Display_DisplayInfo_Get = (ADL_DISPLAY_DISPLAYINFO_GET)GetProcAddress(hDLL, "ADL_Display_DisplayInfo_Get");

		if (NULL == ADL_Display_Color_Get || NULL == ADL_Display_Color_Set)
		{
			printf("ADL's API is missing!\n");
			return 1;
		}

		ADL_Main_Control_Create = (ADL_MAIN_CONTROL_CREATE)GetProcAddress(hDLL, "ADL_Main_Control_Create");
		ADL_Display_Color_Get = (ADL_DISPLAY_COLOR_GET)GetProcAddress(hDLL, "ADL_Display_Color_Get");

		if (ADL_OK != ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1))
		{
			printf("ADL Initialization Error!\n"); 
			return 1;
		}

		int primary;
		ADL_Adapter_Primary_Get(&primary);

		printf("Primary display is %d\n", primary);

		ADL_Display_Color_Set(primary, GetPrimaryDisplayIndex(), ADL_DISPLAY_COLOR_SATURATION, 100);

		while (true)
		{
			if (ForegroundWindowIsCSGO())
			{
				if (DisplayVendor == AMD)
				{
					ADL_Display_Color_Set(primary, GetPrimaryDisplayIndex(), ADL_DISPLAY_COLOR_SATURATION, 200);
				}
			}
			else
			{
				ADL_Display_Color_Set(primary, GetPrimaryDisplayIndex(), ADL_DISPLAY_COLOR_SATURATION, 100);
			}
			Sleep(200);
		}
	}
	else if (DisplayVendor == NVIDIA)
	{
		// TODO
	}
}

