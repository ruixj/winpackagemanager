

#include <Windows.h>
#include <appmodel.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <AppxPackaging.h>  // For Appx Packaging APIs
#include <shlwapi.h>
#pragma once
typedef std::basic_string<TCHAR> tstring;

int ShowUsage();

int ShowUsage()
{
	_tprintf(L"Usage: GetPackagePathByFullName <fullname> [<fullname>...]\n");
	return 1;
}


HRESULT ReadManifestApplications(
	_In_ IAppxManifestApplicationsEnumerator* applications)
{
	HRESULT hr = S_OK;
	BOOL hasCurrent = FALSE;
	UINT32 applicationsCount = 0;

	hr = applications->GetHasCurrent(&hasCurrent);

	while (SUCCEEDED(hr) && hasCurrent)
	{
		IAppxManifestApplication* application = NULL;
		LPWSTR applicationName = NULL;

		hr = applications->GetCurrent(&application);
		if (SUCCEEDED(hr))
		{
			application->GetStringValue(L"Executable", &applicationName);
		}
		if (SUCCEEDED(hr))
		{
			applicationsCount++;
			wprintf(L"Application #%u: %s\n", applicationsCount, applicationName);
		}

		if (SUCCEEDED(hr))
		{
			hr = applications->MoveNext(&hasCurrent);
		}
		if (application != NULL)
		{
			application->Release();
			application = NULL;
		}
		CoTaskMemFree(applicationName);
	}

	wprintf(L"Package contains %u application(s)\n", applicationsCount);
	return hr;
}


int __cdecl wmain(__in int argc, __in_ecount(argc) WCHAR * argv[])
{
	if (argc <= 1)
		return ShowUsage();

	for (int i = 1; i<argc; ++i)
	{
		PCWSTR fullName = argv[i];
		UINT32 length = 0;
		LONG rc = GetPackagePathByFullName(fullName, &length, NULL);
		if (rc != ERROR_INSUFFICIENT_BUFFER)
		{
			_tprintf(L"Error %d in GetPackagePathByFullName\n", rc);
			return 2;
		}

		PWSTR path = (PWSTR)malloc(length * sizeof(WCHAR));
		if (path == NULL)
		{
			_tprintf(L"Error allocating memory\n");
			return 3;
		}

		rc = GetPackagePathByFullName(fullName, &length, path);
		if (rc != ERROR_SUCCESS)
			_tprintf(L"Error %d retrieving Package's path\n", rc);
		else
			_tprintf(L"Path = %s\n", path);
		tstring tstrFullName = path;
		free(path);

		tstrFullName += L"\\AppxManifest.xml";
		HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

		if (SUCCEEDED(hr))
		{
 
			IAppxManifestReader* manifestReader = NULL;

			IAppxFactory* appxFactory = NULL;
			IStream* inputStream = NULL;
			IAppxManifestApplicationsEnumerator* applications = NULL;

			// Create a new Appx factory
			hr = CoCreateInstance(
				__uuidof(AppxFactory),
				NULL,
				CLSCTX_INPROC_SERVER,
				__uuidof(IAppxFactory),
				(LPVOID*)(&appxFactory));

			// Create a stream over the input Appx package
			if (SUCCEEDED(hr))
			{
				hr = SHCreateStreamOnFileEx(
								tstrFullName.c_str(),
								STGM_READ | STGM_SHARE_EXCLUSIVE,
								0, // default file attributes
								FALSE, // do not create new file
								NULL, // no template
								&inputStream);
			}
			if (SUCCEEDED(hr))
			{
				hr = appxFactory->CreateManifestReader(inputStream,&manifestReader);
			}

			if (SUCCEEDED(hr))
			{
				wprintf(L"\nReading <Application> elements\n");
				if (SUCCEEDED(hr))
				{
					hr = manifestReader->GetApplications(&applications);
				}
				if (SUCCEEDED(hr))
				{
					hr = ReadManifestApplications(applications);
				}
			}
			if (applications != NULL)
			{
				applications->Release();
				applications = NULL;
			}
 
			if (manifestReader != NULL)
			{
				manifestReader->Release();
				manifestReader = NULL;
			}

			if (SUCCEEDED(hr))
			{
				wprintf(L"\nManifest read successfully.\n");
			}
			else
			{
				wprintf(L"\nFailed to read manifest with HRESULT 0x%08X.\n", hr);
			}
		}
	}

	return 0;
}
