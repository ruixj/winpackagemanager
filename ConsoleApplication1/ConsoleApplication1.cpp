#include "stdafx.h"
#include <windows.h>
#include <tlhelp32.h> 
//#include <VersionHelpers.h>

BOOL Is_Win_Server()
{
	OSVERSIONINFOEX osvi;
	DWORDLONG dwlConditionMask = 0;
	int op = VER_GREATER_EQUAL;

	// Initialize the OSVERSIONINFOEX structure.

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osvi.dwMajorVersion = 5;
	osvi.dwMinorVersion = 0;
	////osvi.wServicePackMajor = 0;
	//osvi.wServicePackMinor = 0;
	//osvi.wProductType = VER_NT_SERVER;

	// Initialize the condition mask.

	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op);
	VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, op);
	////VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMAJOR, op);
	//VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMINOR, op);
	//VER_SET_CONDITION(dwlConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);

	// Perform the test.

	BOOL  bRet = FALSE;
	bRet = VerifyVersionInfo(
		&osvi,
		VER_MAJORVERSION | VER_MINORVERSION |
		VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR |
		VER_PRODUCT_TYPE,
		dwlConditionMask);

	DWORD retCode;
	retCode = GetLastError();
	return bRet;
}


void printError(TCHAR* msg)
{
	DWORD eNum;
	TCHAR sysMsg[256];
	TCHAR* p;

	eNum = GetLastError();
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, eNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		sysMsg, 256, NULL);

	// Trim the end of the line and terminate it with a null
	p = sysMsg;
	while ((*p > 31) || (*p == 9))
		++p;
	do { *p-- = 0; } while ((p >= sysMsg) &&
		((*p == '.') || (*p < 33)));

	// Display the message
	_tprintf(TEXT("\n  WARNING: %s failed with error %d (%s)"), msg, eNum, sysMsg);
}

 BOOL GetProcessId(LPCTSTR pProgExe,DWORD sessionId,DWORD *pPid)
{
 
	HANDLE hProcessSnap;
	//HANDLE hProcess;
	PROCESSENTRY32 pe32;
	//DWORD dwPriorityClass;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		printError(TEXT("CreateToolhelp32Snapshot (of processes)"));
		return(FALSE);
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (!Process32First(hProcessSnap, &pe32))
	{
		printError(TEXT("Process32First")); // show cause of failure
		CloseHandle(hProcessSnap);          // clean the snapshot object
		return(FALSE);
	}
	DWORD currentSessionId;
	// Now walk the snapshot of processes, and
	// display information about each process in turn
	do
	{
		_tprintf(TEXT("\n\n====================================================="));
		_tprintf(TEXT("\nPROCESS NAME:  %s"), pe32.szExeFile);
		_tprintf(TEXT("\n-------------------------------------------------------"));
		if (_tcscmp(pe32.szExeFile, pProgExe) != 0)
			continue;

		if (!ProcessIdToSessionId(pe32.th32ProcessID, &currentSessionId))
			continue;

		if (currentSessionId != sessionId)
			continue;

		// Retrieve the priority class.
//		dwPriorityClass = 0;
//		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
//		if (hProcess == NULL)
//			printError(TEXT("OpenProcess"));
//		else
//		{
//			dwPriorityClass = GetPriorityClass(hProcess);
//			if (!dwPriorityClass)
//				printError(TEXT("GetPriorityClass"));
//			CloseHandle(hProcess);
//		}

		_tprintf(TEXT("\n  Process ID        = 0x%08X"), pe32.th32ProcessID);
		_tprintf(TEXT("\n  Thread count      = %d"), pe32.cntThreads);
		_tprintf(TEXT("\n  Parent process ID = 0x%08X"), pe32.th32ParentProcessID);
		_tprintf(TEXT("\n  Priority base     = %d"), pe32.pcPriClassBase);
//		if (dwPriorityClass)
//			_tprintf(TEXT("\n  Priority class    = %d"), dwPriorityClass);

		// List the modules and threads associated with this process
		//ListProcessModules(pe32.th32ProcessID);
		//ListProcessThreads(pe32.th32ProcessID);

	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return(TRUE);
}

 BOOL
	 IsWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor)
 {
	 BOOL  bRet = FALSE;
	 DWORD retCode;
	 OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0,{ 0 }, 0, 0 };
	 DWORDLONG        const dwlConditionMask = VerSetConditionMask(
		                                         VerSetConditionMask(
			                                       VerSetConditionMask(
				                                       0,  VER_MAJORVERSION, VER_GREATER_EQUAL),
			                                     VER_MINORVERSION, VER_GREATER_EQUAL),
		                                       VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

	 osvi.dwMajorVersion = wMajorVersion;
	 osvi.dwMinorVersion = wMinorVersion;
	 osvi.wServicePackMajor = wServicePackMajor;

	 bRet = VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask);
	 retCode = GetLastError();
	 return bRet;
 }

 
 BOOL
	 IsWindowsWin10OrGreater()
 {
	 return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINTHRESHOLD), LOBYTE(_WIN32_WINNT_WINTHRESHOLD), 0);
 }
void _tmain(int argc, TCHAR *argv[])
{

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	DWORD processId;
	DWORD currentSessionId;

	DWORD currentProcessId = GetCurrentProcessId();
	if (!ProcessIdToSessionId(currentProcessId, &currentSessionId))
		return ;

	Is_Win_Server();

	if (IsWindowsWin10OrGreater())
	{
		MessageBox(NULL, _T("You need at least Windows 10."), _T("Version Not Supported"), MB_OK);
	}

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// start the browser process now
 
	si.cb = sizeof(si);
	si.dwFlags = STARTF_FORCEONFEEDBACK | STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWNORMAL;

	 
	BOOL bStatus = CreateProcess(NULL,
		_T("C:\\Windows\\SystemApps\\Microsoft.MicrosoftEdge_8wekyb3d8bbwe\\MicrosoftEdge.exe www.baidu.com"),
		NULL,
		NULL,
		FALSE,
		NORMAL_PRIORITY_CLASS,
		NULL,
		NULL,
		&si,
		&pi);

	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	TCHAR test[] = _T("C:\\windows\\system32\\LaunchWinApp.exe www.baidu.com");
	// Start the child process. 
	BOOL bRet = CreateProcess(NULL,   // No module name (use command line)
		test,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi            // Pointer to PROCESS_INFORMATION structure
		 );
	if(!bRet)
	{
		printf("CreateProcess failed (%d).\n", GetLastError());
		return;
	}
	// Wait until child process exits.
	WaitForSingleObject(pi.hProcess, INFINITE);

	GetProcessId(_T("MicrosoftEdge.exe"), currentSessionId, &processId);
	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}


