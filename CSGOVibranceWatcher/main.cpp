#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <tchar.h>
#include <psapi.h>
#include "adl_defines.h"
#include "adl_sdk.h"
#include "adl_structures.h"

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

static int PrimaryDisplay = 0;
static int DisplayVendor = 0;
static bool ProcessDebugLogging = false;
static bool DisplayDeviceDebugLogging = false;
static int LastSaturation = 0;
static int PrimaryDisplayIndex = 0;

void* __stdcall ADL_Main_Memory_Alloc(int iSize)
{
	auto lpBuffer = malloc(iSize);
	return lpBuffer;
}

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

int GetCurrentSaturation()
{
	int curSat, def, minSat, maxSat, step;
	ADL_Display_Color_Get(PrimaryDisplay, PrimaryDisplayIndex, 
		ADL_DISPLAY_COLOR_SATURATION, &curSat, &def, &minSat, &maxSat, &step);
	return curSat;
}

void SetSaturationFade(int saturation)
{
	if (saturation != LastSaturation)
	{
		LastSaturation = saturation;
		auto currentSaturation = GetCurrentSaturation();

		if (DisplayVendor == AMD)
		{
			std::cout << "Fading saturation from " << currentSaturation 
				<< " to " << saturation << std::endl;
			if (currentSaturation > saturation)
			{
				for (auto i = currentSaturation; i >= saturation; i--)
				{
					ADL_Display_Color_Set(PrimaryDisplay, PrimaryDisplayIndex,
						ADL_DISPLAY_COLOR_SATURATION, i);
					Sleep(20);
				}
			}
			else
			{
				for (auto i = currentSaturation; i <= saturation; i++)
				{
					ADL_Display_Color_Set(PrimaryDisplay, PrimaryDisplayIndex,
						ADL_DISPLAY_COLOR_SATURATION, i);
					Sleep(20);
				}
			}
		}
		std::cout << "Done. Saturation is now " << saturation << std::endl;
	}
}

int main()
{
	GetDisplayDevice();
	if (DisplayVendor == AMD)
	{
		std::cout << "AMD card detected, using ADL.." << std::endl;
		auto hDLL = LoadLibraryA("atiadlxx.dll");
		if (hDLL == nullptr) // try 32 bit dll
			hDLL = LoadLibraryA("atiadlxy.dll");
		if (hDLL == nullptr)
		{
			printf("ADL library not found!\n"); 
			return 1;
		}

		ADL_Display_Color_Get = ADL_DISPLAY_COLOR_GET(GetProcAddress(hDLL, "ADL_Display_Color_Get"));
		ADL_Display_Color_Set = ADL_DISPLAY_COLOR_SET(GetProcAddress(hDLL, "ADL_Display_Color_Set"));
		ADL_Adapter_Primary_Get = ADL_ADAPTER_PRIMARY_GET(GetProcAddress(hDLL, "ADL_Adapter_Primary_Get"));
		ADL_Display_DisplayInfo_Get = ADL_DISPLAY_DISPLAYINFO_GET(GetProcAddress(hDLL, "ADL_Display_DisplayInfo_Get"));
		ADL_Main_Control_Create = ADL_MAIN_CONTROL_CREATE(GetProcAddress(hDLL, "ADL_Main_Control_Create"));
		ADL_Display_Color_Get = ADL_DISPLAY_COLOR_GET(GetProcAddress(hDLL, "ADL_Display_Color_Get"));

		if (ADL_OK != ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1))
		{
			printf("ADL Initialization Error!\n"); 
			return 1;
		}

		ADL_Adapter_Primary_Get(&PrimaryDisplay);
		LastSaturation = GetCurrentSaturation();

		int primary;
		ADL_Adapter_Primary_Get(&primary);
		int displayCount;
		ADLDisplayInfo *displayInfo = nullptr;

		ADL_Display_DisplayInfo_Get(primary, &displayCount, &displayInfo, 0);
		PrimaryDisplayIndex = displayInfo[0].displayID.iDisplayLogicalIndex;

		while (true)
		{
			if (ForegroundWindowIsCSGO())
				SetSaturationFade(130);
			else
				SetSaturationFade(100);

			Sleep(200);
		}
	}
	if (DisplayVendor == NVIDIA)
	{
		// TODO
	}
}

