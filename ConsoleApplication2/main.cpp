#define _UNICODE 1
#define UNICODE 1
#include <tchar.h>
#include <Windows.h>
#include <appmodel.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

int ShowUsage();
void ShowProcessPackageId(__in const UINT32 pid, __in HANDLE process);
void ShowPackageId(__in const PACKAGE_ID * packageId);

int ShowUsage()
{
	_tprintf(L"Usage: GetPackageId <pid> [<pid>...]\n");
	return 1;
}

int __cdecl wmain(__in int argc, __in_ecount(argc) WCHAR * argv[])
{
	if (argc <= 1)
		return ShowUsage();

	for (int i = 0; i<argc; ++i)
	{
		UINT32 pid = wcstoul(argv[i], NULL, 10);
		if (pid > 0)
		{
			HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
			if (process == NULL)
				_tprintf(L"Error %d in OpenProcess (pid=%u)\n", GetLastError(), pid);
			else
			{
				ShowProcessPackageId(pid, process);
				CloseHandle(process);
			}
		}
	}
	return 0;
}

void ShowProcessPackageId(__in const UINT32 pid, __in HANDLE process)
{
	_tprintf(L"Process %u (handle=%p)\n", pid, process);

	UINT32 length = 0;
	LONG rc = GetPackageId(process, &length, NULL);
	if (rc != ERROR_INSUFFICIENT_BUFFER)
	{
		if (rc == APPMODEL_ERROR_NO_PACKAGE)
			_tprintf(L"Process has no package identity\n");
		else
			_tprintf(L"Error %d in GetPackageId\n", rc);
		return;
	}

	BYTE * buffer = (PBYTE)malloc(length);
	if (buffer == NULL)
	{
		_tprintf(L"Error allocating memory\n");
		return;
	}

	rc = GetPackageId(process, &length, buffer);
	if (rc != ERROR_SUCCESS)
		_tprintf(L"Error %d retrieving PackageId\n", rc);
	else
		ShowPackageId((PACKAGE_ID *)buffer);

	free(buffer);


	_tprintf(L"Process %u (handle=%p)\n", pid, process);

    length = 0;
	rc = GetPackageFullName(process, &length, NULL);
	if (rc != ERROR_INSUFFICIENT_BUFFER)
	{
		if (rc == APPMODEL_ERROR_NO_PACKAGE)
			_tprintf(L"Process has no package identity\n");
		else
			_tprintf(L"Error %d in GetPackageFullName\n", rc);
		return;
	}

	PWSTR fullName = (PWSTR)malloc(length * sizeof(*fullName));
	if (fullName == NULL)
	{
		_tprintf(L"Error allocating memory\n");
		return;
	}

	rc = GetPackageFullName(process, &length, fullName);
	if (rc != ERROR_SUCCESS)
		_tprintf(L"Error %d retrieving PackageFullName\n", rc);
	else
		_tprintf(L"%s\n", fullName);

	free(fullName);
}

void ShowPackageId(__in const PACKAGE_ID * packageId)
{
	_tprintf(L"    Name        : %s\n", packageId->name);
	_tprintf(L"    Publisher   : <NULL>\n");
	_tprintf(L"    PublisherId : %s\n", packageId->publisherId);
	_tprintf(L"    Version     : %hu.%hu.%hu.%hu\n",
		packageId->version.Major,
		packageId->version.Minor,
		packageId->version.Build,
		packageId->version.Revision);
	_tprintf(L"    Architecture: %u\n", packageId->processorArchitecture);
	if (packageId->resourceId != NULL)
		_tprintf(L"    Resource    : %s\n", packageId->resourceId);


	UINT32 length = 0;
	LONG rc = GetPackagePath(packageId, 0, &length, NULL);
	if (rc != ERROR_INSUFFICIENT_BUFFER)
	{
		_tprintf(L"Error %d in GetPackagePath\n", rc);
		return ;
	}

	PWSTR path = (PWSTR)malloc(length * sizeof(WCHAR));
	if (path == NULL)
	{
		_tprintf(L"Error allocating memory\n");
		return ;
	}

	rc = GetPackagePath(packageId, 0, &length, path);
	if (rc != ERROR_SUCCESS)
		_tprintf(L"Error %d retrieving Package's path\n", rc);
	else
		_tprintf(L"Path = %s\n", path);

	free(path);



}
