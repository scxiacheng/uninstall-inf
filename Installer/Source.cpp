#include <Windows.h>
#include <SetupAPI.h>
#include <cfgmgr32.h>
#include <tchar.h>
#pragma comment(lib, "Setupapi.lib")
#pragma comment(lib, "Cfgmgr32.lib")

BOOL UnInstallDevice(HDEVINFO hDevInfo, SP_DEVINFO_DATA* pDevInfoData)
{
	SP_DEVINFO_LIST_DETAIL_DATA DevInfoListDetailData = { 0 };
	DevInfoListDetailData.cbSize = sizeof(SP_DEVINFO_LIST_DETAIL_DATA);
	if (!SetupDiGetDeviceInfoListDetail(hDevInfo, &DevInfoListDetailData))
	{
		return FALSE;
	}

	TCHAR szBuffer[MAX_PATH] = {0x0};
	if (CR_SUCCESS != CM_Get_Device_ID_Ex(pDevInfoData->DevInst, szBuffer, MAX_PATH, 0, NULL))
	{
		return FALSE;
	}

	SP_CLASSINSTALL_HEADER ClassInstallHeader = { 0x0 };
	ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
	ClassInstallHeader.InstallFunction = DIF_SELECTDEVICE;
	if (!SetupDiSetClassInstallParams(hDevInfo, pDevInfoData, &ClassInstallHeader, sizeof(SP_CLASSINSTALL_HEADER)))
	{
		return FALSE;
	}

	if (!SetupDiCallClassInstaller(DIF_REMOVE, hDevInfo, pDevInfoData))
	{
		return FALSE;
	}
	SP_DEVINSTALL_PARAMS DevInstallParams = { 0x0 };
	DevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
	if (!SetupDiGetDeviceInstallParams(hDevInfo, pDevInfoData, &DevInstallParams))
	{
		return FALSE;
	}
	return TRUE;
}

BOOL UninstallINF(LPCTSTR lpszHardwareId)
{
	HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
	BOOL bSuccess = FALSE;
	__try
	{
		hDevInfo = SetupDiGetClassDevsEx(NULL, NULL, NULL, 6, NULL, NULL, NULL);
		if (INVALID_HANDLE_VALUE == hDevInfo)
		{
			__leave;
		}

		
		SP_DEVINFO_LIST_DETAIL_DATA DevInfoListDetailData = { 0 };
		DevInfoListDetailData.cbSize = sizeof(SP_DEVINFO_LIST_DETAIL_DATA);
		BOOL bRet = SetupDiGetDeviceInfoListDetail(hDevInfo, &DevInfoListDetailData);
		if (!bRet)
		{
			__leave;
		}

		SP_DEVINFO_DATA DevInfoData = { 0 };
		DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		for (int iMemberIndex = 0; SetupDiEnumDeviceInfo(hDevInfo, iMemberIndex, &DevInfoData); iMemberIndex++)
		{
			if (ERROR_NO_MORE_ITEMS == GetLastError())
			{
				break;
			}

			TCHAR szBuffer[MAX_PATH] = {0x0};
			if (CR_SUCCESS != CM_Get_Device_ID_Ex(DevInfoData.DevInst, szBuffer, MAX_PATH, 0, NULL))
			{
				continue;
			}

			DWORD PropertyRegDataType = 0;
			TCHAR szHardwareId[MAX_PATH] = { 0x0 };
			if (!SetupDiGetDeviceRegistryProperty(hDevInfo, &DevInfoData, SPDRP_HARDWAREID, &PropertyRegDataType, (PBYTE)szHardwareId, sizeof(szHardwareId), NULL))
			{
				continue;
			}

			if (_tcsicmp(lpszHardwareId, szHardwareId) == 0)
			{
				bSuccess = UnInstallDevice(hDevInfo, &DevInfoData);
				break;
			}

			TCHAR CompitibleIds[MAX_PATH] = { 0x0 };
			if (!SetupDiGetDeviceRegistryProperty(hDevInfo, &DevInfoData, SPDRP_COMPATIBLEIDS, &PropertyRegDataType, (PBYTE)CompitibleIds, sizeof(CompitibleIds), NULL))
			{
				continue;
			}
			TCHAR* pId = CompitibleIds;
			while (pId !=NULL)
			{
				if (_tcsicmp(pId, lpszHardwareId) == 0)
				{
					bSuccess = UnInstallDevice(hDevInfo, &DevInfoData);
					break;
				}
				++pId;
			}
		}
	}
	__finally
	{
		SetupDiDestroyDeviceInfoList(hDevInfo);
	}
	
	return bSuccess;
}

int main(int argc, char** argv)
{
	UninstallINF(L"hardwareid");
}