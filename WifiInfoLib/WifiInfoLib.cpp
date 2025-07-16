// WifiInfoLib.cpp : 
//
#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <wlanapi.h>
#include <wchar.h>
#include <string>
#include <tchar.h>
#include "WifiInfoLib.h"

DWORD WIL_errno;
WILResultWStringMap resultWStringMap;

//無線に関する通知を行うコールバック関数
static void WlanNotification(WLAN_NOTIFICATION_DATA *wlanNotifData, VOID *p)
{
	//ロケール指定
	WCHAR strReason[WIL_WRITE_BUFFER_SIZE];
	WLAN_CALLBACK_CONTEXT* pCallbackContext = (WLAN_CALLBACK_CONTEXT*)p;

	if (pCallbackContext == NULL) {
		return;
	}

	if (memcmp(&pCallbackContext->interfaceGUID, &wlanNotifData->InterfaceGuid, sizeof(GUID)) != 0) {
		return;
	}

	//通知元をauto configuration module(ACM)に設定
	if (wlanNotifData->NotificationSource == WLAN_NOTIFICATION_SOURCE_ACM) {
		PWLAN_CONNECTION_NOTIFICATION_DATA pConnNotifData = NULL;
		//WCHAR *notificationMessage;

		switch (wlanNotifData->NotificationCode) {
		case wlan_notification_acm_scan_complete: //スキャン完了
			//notificationMessage = (WCHAR*)L"wlan_notification_acm_scan_complete";
			SetEvent(pCallbackContext->scanEvent);
			break;
		case wlan_notification_acm_scan_fail: //スキャン失敗
			//notificationMessage = (WCHAR*)L"wlan_notification_acm_scan_fail";
			pConnNotifData = (PWLAN_CONNECTION_NOTIFICATION_DATA)wlanNotifData->pData;
			if (pConnNotifData->wlanReasonCode != ERROR_SUCCESS) {
				SetEvent(pCallbackContext->scanEvent);
				WlanReasonCodeToString(pConnNotifData->wlanReasonCode, WIL_WRITE_BUFFER_SIZE, strReason, NULL);
			}
			break;
		case wlan_notification_acm_scan_list_refresh: //ネットワーク一覧が更新された
			//notificationMessage = (WCHAR*)L"wlan_notification_acm_scan_list_refresh";
			break;
		default:
			//notificationMessage = (WCHAR*)TEXT("Unknown:");
			break;
		}

		//		wprintf(L"%ls\n", notificationMessage);
	}

}

// 周波数からチャネルに変換する
int getChannelFromFrequency(ULONG ulChFreq)
{
	int nCh = -1;
	const ULONG ulBase24 = 2412000;
	const ULONG ulMax24 = 2472000;

	const ULONG ulBaseW5253 = 5180000;
	const ULONG ulMaxW5253 = 5320000;
	const ULONG ulBaseChW5253 = 36;

	const ULONG ulBaseW56 = 5500000;
	const ULONG ulMaxW56 = 5700000;
	const ULONG ulBaseChW56 = 100;

	const ULONG ulBaseNA = 5745000;
	const ULONG ulMaxNA = 5825000;
	const ULONG ulBaseChNA = 149;

	// 2.4GHz
	if (ulChFreq <= ulMax24) {
		nCh = (int)(ulChFreq - ulBase24) / 5000 + 1;
	}
	// 5GHz - W52/W53
	else if (ulChFreq <= ulMaxW5253) {
		nCh = 4 * ((int)(ulChFreq - ulBaseW5253) / 20000) + ulBaseChW5253;
	}
	// 5GHz - W56
	else if (ulChFreq <= ulMaxW56) {
		nCh = 4 * ((int)(ulChFreq - ulBaseW56) / 20000) + ulBaseChW56;
	}
	// 5GHz - North America
	else if (ulChFreq <= ulMaxNA) {
		nCh = 4 * ((int)(ulChFreq - ulBaseNA) / 20000) + ulBaseChNA;
	}

	return nCh;
}
void createResultStringMap()
{
	resultWStringMap.clear();

	resultWStringMap[WIL_SUCCESS] = L"WIL_SUCCESS: Success.";
	resultWStringMap[WIL_OPEN_HANDLE_ERROR] = L"WIL_OPEN_HANDLE_ERROR: Unable to open native Wi-Fi handle.";
	resultWStringMap[WIL_CLOSE_HANDLE_ERROR] = L"WIL_CLOSE_HANDLE_ERROR: Unable to close native Wi-Fi handle.";
	resultWStringMap[WIL_ENUM_INTERFACES_ERROR] = L"WIL_ENUM_INTERFACES_ERROR: Failed to enumerate all of the wireless LAN interfaces.";
	resultWStringMap[WIL_REGISTER_NOTIFICATION_ERROR] = L"WIL_REGISTER_NOTIFICATION_ERROR: Failed to register or unregister notification callback.";
	resultWStringMap[WIL_SCAN_ERROR] = L"WIL_SCAN_ERROR: Failed to initiate Wi-Fi network scan.";
	resultWStringMap[WIL_SCAN_TIMEOUT] = L"WIL_SCAN_TIMEOUT: Cannnot complete scan Wi-Fi network.";
	resultWStringMap[WIL_GET_BSSLIST_ERROR] = L"WIL_GET_BSSLIST_ERROR: Cannnot acquire BSS list.";
	resultWStringMap[WIL_PARAMETER_ERROR] = L"WIL_PARAMETER_ERROR: Parameter error.";
	resultWStringMap[WIL_NO_WIFI_INTERFACE_EXIST] = L"WIL_NO_WIFI_INTERFACE_EXIST: Wi-Fi interface not found.";
	resultWStringMap[WIL_INSUFFICIENT_BUFFER] = L"WIL_INSUFFICIENT_BUFFER: Insuffcient buffer error.";
	resultWStringMap[WIL_FILE_OPEN_ERROR] = L"WIL_FILE_OPEN_ERROR: Cannot open file.";
	resultWStringMap[WIL_LOCAL_ERROR] = L"WIL_LOCAL_ERROR: WifiInfoLib internal error happened.";
}

DWORD WifiInfoLib_ShowResult(DWORD dwResult)
{
	wprintf(L"%u: ", WifiInfoLib_GetLastError());
	if (resultWStringMap.find(dwResult) != resultWStringMap.end()) {
		wprintf(L"%s\n", resultWStringMap[dwResult].c_str());
	}
	else {
		wprintf(L"Unknown error %d happened.\n", dwResult);
	}
	return WIL_SUCCESS;
}

DWORD WifiInfoLib_GetResultString(DWORD dwResult, LPWSTR lpszString, DWORD InBufSize)
{
	if (!lpszString) {
		return WIL_PARAMETER_ERROR;
	}

	wstring wstrTemp;
	if (resultWStringMap.find(dwResult) != resultWStringMap.end()) {
		wstrTemp = resultWStringMap[dwResult];
	}
	else {
		wstrTemp = L"Unknown error ";
		wstrTemp += to_wstring(dwResult);
		wstrTemp += L" happened.";
	}

	if (wstrTemp.length() > InBufSize) {
		return WIL_INSUFFICIENT_BUFFER;
	}

	wcscpy_s(lpszString, InBufSize, wstrTemp.c_str());

	return WIL_SUCCESS;
}

DWORD WifiInfoLib_GetLastError()
{
	return WIL_errno;
}

// Wi-Fi Info LibraryのOpen関数
// ライブラリー使用開始前に一度コールすること
DWORD WifiInfoLib_Open(PHANDLE phClientHandle, PWLAN_INTERFACE_INFO_LIST *ppIfList)
{
	DWORD dwMaxClient = 2;      //    
	DWORD dwCurVersion = 0;
	DWORD dwResult = 0;

	WIL_errno = 0;

	createResultStringMap();
	
	dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, phClientHandle);
	if (dwResult != ERROR_SUCCESS) {
		WIL_errno = dwResult;
		return WIL_OPEN_HANDLE_ERROR;
	}

	dwResult = WlanEnumInterfaces(*phClientHandle, NULL, ppIfList);
	if (dwResult != ERROR_SUCCESS) {
		WIL_errno = dwResult;
		return WIL_ENUM_INTERFACES_ERROR;
	}

	if ((*ppIfList)->dwNumberOfItems <= 0) {
		WIL_errno = WIL_LOCAL_ERROR;
		return WIL_NO_WIFI_INTERFACE_EXIST;
	}

	//PLOGI << "WIL initialized.";

	return WIL_SUCCESS;
}

// Wi-Fi Info LibraryのClose関数
// ライブラリー使用終了時にコールすること
DWORD WifiInfoLib_Close(HANDLE hClientHandle, PWLAN_INTERFACE_INFO_LIST pIfList)
{
	DWORD dwResult = 0;

	WIL_errno = 0;

	if (hClientHandle != NULL) {
		dwResult = WlanCloseHandle(hClientHandle, NULL);
		if (dwResult != ERROR_SUCCESS) {
			WIL_errno = dwResult;
			return WIL_CLOSE_HANDLE_ERROR;
		}
	}

	if (pIfList != NULL) {
		WlanFreeMemory(pIfList);
		pIfList = NULL;
	}

	//PLOGI << "WIL closed.";

	return WIL_SUCCESS;
}

// Wi-Fi Info LibraryのScan関数
// 無線ネットワークのスキャンを行い、スキャン処理が終了するまで待つ
DWORD WifiInfoLib_Scan(HANDLE hClient, PWLAN_INTERFACE_INFO_LIST pIfList)
{
	DWORD dwResult = 0;
	DWORD dwRetVal = 0;
	DWORD dwPrevNotif = 0;
	BOOL bNotifySet = FALSE;

	WIL_errno = 0;

	PWLAN_INTERFACE_INFO pIfInfo = NULL;

	for (int i = 0; i < (int)pIfList->dwNumberOfItems; i++) {
		pIfInfo = (WLAN_INTERFACE_INFO *)&pIfList->InterfaceInfo[i];

		INT nPower = 0;
		dwResult = WifiInfoLib_QueryRadioPower(&nPower, hClient, pIfInfo);
		if (dwResult != ERROR_SUCCESS) {
			WIL_errno = dwResult;
			return WIL_QUERY_INTERFACE_ERROR;
		}
		if (nPower == WIL_RADIO_POWER_OFF)
		{
			continue;
		}

		WLAN_CALLBACK_CONTEXT callbackContext = { 0 };
		callbackContext.interfaceGUID = pIfInfo->InterfaceGuid;
		callbackContext.scanEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		//コールバックの登録
		dwResult = WlanRegisterNotification(hClient, WLAN_NOTIFICATION_SOURCE_ACM, FALSE, (WLAN_NOTIFICATION_CALLBACK)WlanNotification, (PVOID)&callbackContext, NULL, &dwPrevNotif);
		if (dwResult != ERROR_SUCCESS) {
			WIL_errno = dwResult;
			return WIL_REGISTER_NOTIFICATION_ERROR;
		}
		bNotifySet = TRUE;

		//利用可能なネットワークをスキャン
		dwResult = WlanScan(hClient, &pIfInfo->InterfaceGuid, NULL, NULL, NULL);
		if (dwResult != ERROR_SUCCESS) {
			WIL_errno = dwResult;
			return WIL_SCAN_ERROR;
		}

		// コールバックが呼ばれるまで待つ
		DWORD waitResult = WaitForSingleObject(callbackContext.scanEvent, 15000);

		if (waitResult == WAIT_TIMEOUT) {
			WIL_errno = WIL_LOCAL_ERROR;
			dwRetVal = WIL_SCAN_TIMEOUT;
		}
		else if (waitResult == WAIT_OBJECT_0) {
			dwRetVal = WIL_SUCCESS;
		}

		//通知の解除
		if (bNotifySet) {
			dwResult = WlanRegisterNotification(hClient, WLAN_NOTIFICATION_SOURCE_NONE, TRUE, NULL, NULL, NULL, &dwPrevNotif);
			bNotifySet = FALSE;
		}

		if (dwRetVal != WIL_SUCCESS) {
			return dwRetVal;
		}
	}

	return WIL_SUCCESS;
}

DWORD WifiInfoLib_QueryRadioPower(PINT pValue, const HANDLE &hClient, const PWLAN_INTERFACE_INFO &pIfInfo)
{
	DWORD dwResult = 0;
	DWORD radioStateSize = sizeof(WLAN_RADIO_STATE);
	PWLAN_RADIO_STATE pRadioState = NULL;
	WLAN_OPCODE_VALUE_TYPE opCode = wlan_opcode_value_type_invalid;

	*pValue = WIL_RADIO_POWER_ON;

	dwResult = WlanQueryInterface(hClient, &pIfInfo->InterfaceGuid, wlan_intf_opcode_radio_state, NULL,
		&radioStateSize, (PVOID *)&pRadioState, &opCode);
	if (dwResult != ERROR_SUCCESS) {
		WIL_errno = dwResult;
		return WIL_QUERY_INTERFACE_ERROR;
	}

	for (int i = 0; i < (int)pRadioState->dwNumberOfPhys; i++)
	{
		PWLAN_PHY_RADIO_STATE pPhyRadioState = &pRadioState->PhyRadioState[i];
		if ((pPhyRadioState->dot11HardwareRadioState == dot11_radio_state_off) ||
			(pPhyRadioState->dot11SoftwareRadioState == dot11_radio_state_off))
		{
			*pValue = WIL_RADIO_POWER_OFF;
			return ERROR_SUCCESS;
		}
	}

	return ERROR_SUCCESS;
}

// BSS Load (see 9.4.2.28)
void Element11(unsigned char *pBuff, PWIL_BSS_INFO pWILBssInfo)
{
	PWLAN_BSS_LOAD_ELEMENT pBssLoad;
	pBssLoad = (PWLAN_BSS_LOAD_ELEMENT)pBuff;
	pWILBssInfo->stationCount = pBssLoad->stationCount;
	pWILBssInfo->channelUtilization = pBssLoad->channelUtilization;
}

// HT Capabilities (see 9.4.2.56)
void Element45(unsigned char *pBuff, PWIL_BSS_INFO pWILBssInfo)
{
	PWLAN_HT_CAPABILITIES_ELEMENT pHTCapabilities;
	pHTCapabilities = (PWLAN_HT_CAPABILITIES_ELEMENT)pBuff;
	pWILBssInfo->HTCapabilitiesRxHighestSupportedDataRate = (pHTCapabilities->supportedMCSSet2 >> 16) & 0x03ff;
}

// HT Operation (see 9.4.2.57)
void Element61(unsigned char *pBuff, PWIL_BSS_INFO pWILBssInfo)
{
	PWLAN_HT_OPERATION_ELEMENT pHTOperation;
	pHTOperation = (PWLAN_HT_OPERATION_ELEMENT)pBuff;
	pWILBssInfo->HTOperationChannel = pHTOperation->primaryChannel;
	if (pHTOperation->HTOperationInformation1 & 0x04) {
		pWILBssInfo->HTOperationChannelWidth = 40;
		pWILBssInfo->VHTOperationChannelWidth = 40;
	}
	else {
		pWILBssInfo->HTOperationChannelWidth = 20;
		pWILBssInfo->VHTOperationChannelWidth = 20;
	}

	//unsigned char ucHTOpCh = *pBuff;
	//unsigned char ucHTOpChWidth;
	//unsigned char nTmp = *(pBuff + 1);
	//if (nTmp & 0x04) {
	//	ucHTOpChWidth = 40;
	//}
	//else {
	//	ucHTOpChWidth = 20;
	//}
	//pWILBssInfo->HTOperationChannel = ucHTOpCh;
	//pWILBssInfo->HTOperationChannelWidth = ucHTOpChWidth;
}

// VHT Operation (see 9.4.2.159)
// Folloe Table 9-253 description
void Element192(unsigned char *pBuff, PWIL_BSS_INFO pWILBssInfo)
{
	unsigned char ucWidth = *pBuff;
	unsigned char ucHTOpChWidth = pWILBssInfo->HTOperationChannelWidth;
	unsigned char ucVHTOpChWidth;

	if (ucWidth == 0) {
		if (ucHTOpChWidth == 20) {
			ucVHTOpChWidth = 20;
		}
		else {
			ucVHTOpChWidth = 40;
		}
	}
	else if (ucWidth == 1) {
		unsigned char CCFS0 = *(pBuff + 1);
		unsigned char CCFS1 = *(pBuff + 2);
		unsigned char diffAbs = abs(CCFS1 - CCFS0);
		if (CCFS1 == 0) {
			ucVHTOpChWidth = 80;
		}
		else if ((CCFS1 > 0) && (diffAbs == 8)) {
			ucVHTOpChWidth = 160;
		}
		else if ((CCFS1 > 0) && (diffAbs > 16)) {
			ucVHTOpChWidth = 254;
		}
		else {
			ucVHTOpChWidth = 255;
		}
	}
	// deprecated...
	else if (ucWidth == 2) {
		ucVHTOpChWidth = 160;
	}
	// deprecated...
	else if (ucWidth == 3) {
		ucVHTOpChWidth = 254;
	}

	pWILBssInfo->VHTOperationChannelWidth = ucVHTOpChWidth;
}

// deprecated
// Wi-Fi Info LibraryのBSSID情報の取得関数
// 引数のWILBssInfoMapに取得した情報を蓄積する
DWORD WifiInfoLib_GetBSSIDInfo(HANDLE hClient, PWLAN_INTERFACE_INFO_LIST pIfList, PWILBssInfoMap pBssInfoMap)
{
	DWORD dwResult = 0;
	DWORD dwRetVal = 0;

	PWLAN_INTERFACE_INFO pIfInfo = NULL;
	PWLAN_BSS_LIST pWlanBssList = NULL;
	PWLAN_BSS_ENTRY pWlanBssEntry = NULL;

	for (int i = 0; i < (int)pIfList->dwNumberOfItems; i++) {
		pIfInfo = (WLAN_INTERFACE_INFO *)&pIfList->InterfaceInfo[i];

		INT nPower = 0;
		dwResult = WifiInfoLib_QueryRadioPower(&nPower, hClient, pIfInfo);
		if (dwResult != ERROR_SUCCESS) {
			WIL_errno = dwResult;
			return WIL_QUERY_INTERFACE_ERROR;
		}
		if (nPower == WIL_RADIO_POWER_OFF)
		{
			continue;
		}

		dwResult = WlanGetNetworkBssList(hClient,
			&pIfInfo->InterfaceGuid,
			NULL,
			dot11_BSS_type_any,
			TRUE,
			NULL,
			&pWlanBssList);

		if (dwResult != ERROR_SUCCESS) {
			if (pWlanBssList != NULL) {
				WlanFreeMemory(pWlanBssList);
				pWlanBssList = NULL;
			}
			WIL_errno = dwResult;
			return WIL_GET_BSSLIST_ERROR;
		}
		else {
			for (int j = 0; j < (int)pWlanBssList->dwNumberOfItems; j++) {
				pWlanBssEntry = (WLAN_BSS_ENTRY*)& pWlanBssList->wlanBssEntries[j];

				// Map保存時のユニークIDにMACアドレスを使用
				ULONG64 ulMacAddress = 0;
				for (int n = 0; n < 6; n++) {
					ULONG64 ulTmp = pWlanBssEntry->dot11Bssid[n];
					ulTmp = ulTmp << (8 * (5 - n));
					ulMacAddress += ulTmp;
				}

				WIL_BSS_INFO WILBssInfo;
				SecureZeroMemory(WILBssInfo.elementDataBlob, ELEMENT_DATA_MAX_SIZE);
				GetLocalTime(&(WILBssInfo.lastUpdate));

				// APIで取得したWLAN_BSS_ENTRYはそのまま保存
				WILBssInfo.wlanBssEntry = *pWlanBssEntry;
				WILBssInfo.stationCount = 0xffff;
				WILBssInfo.channelUtilization = 0xff;

				// Elementのオフセットとサイズ
				ULONG ulIEOffset = pWlanBssEntry->ulIeOffset;
				ULONG ulSizeIEBlob = pWlanBssEntry->ulIeSize;
				LONG_PTR address = (LONG_PTR)pWlanBssEntry + ulIEOffset;
				unsigned char* pOrig = (unsigned char*)((LONG_PTR)pWlanBssEntry + ulIEOffset);

				// 解析用バッファに一時保存
				unsigned char* pIEBlob = (unsigned char*)MALLOC(ulSizeIEBlob);
				memcpy_s(pIEBlob, ulSizeIEBlob, pOrig, ulSizeIEBlob);
				// MAP保存用バッファにも保存
				memcpy_s(WILBssInfo.elementDataBlob, ELEMENT_DATA_MAX_SIZE, pOrig, ulSizeIEBlob);
				WILBssInfo.elementDataBlobSize = ulSizeIEBlob;

				ULONG ulTotalBytes = 0;
				unsigned char ElementID;
				unsigned char length;

				// Elementの解析
				//  Element ID:           1 Octet
				//  Length:               1 Octet
				//  Element ID Extension: 1 Octet Curently only used for Element ID 255
				//  Information:          variable
				while (ulTotalBytes < ulSizeIEBlob) {
					ElementID = *(pIEBlob + ulTotalBytes);
					ulTotalBytes++;
					length = *(pIEBlob + ulTotalBytes);
					ulTotalBytes++;

					// BSS Load (see 9.4.2.28)
					if (ElementID == 11) {
						Element11((unsigned char*)((LONG_PTR)pIEBlob + ulTotalBytes), &WILBssInfo);
					}
					// HT Capabilities (see 9.4.2.56)
					else if (ElementID == 45) {
						Element45((unsigned char*)((LONG_PTR)pIEBlob + ulTotalBytes), &WILBssInfo);
					}
					// HT Operation (see 9.4.2.57)
					else if (ElementID == 61) {
						Element61((unsigned char*)((LONG_PTR)pIEBlob + ulTotalBytes), &WILBssInfo);
					}
					// VHT Operation (see 9.4.2.159)
					else if (ElementID == 192) {
						Element192((unsigned char*)((LONG_PTR)pIEBlob + ulTotalBytes), &WILBssInfo);
					}
					ulTotalBytes += length;
				}
				FREE((LPVOID)pIEBlob);

				// MACアドレスをキーにMAPに保存
				(*pBssInfoMap)[ulMacAddress] = WILBssInfo;
			}
		}

		if (pWlanBssList != NULL) {
			WlanFreeMemory(pWlanBssList);
			pWlanBssList = NULL;
		}
	}

	return WIL_SUCCESS;
}

DWORD WifiInfoLib_GetWifiInfo(HANDLE hClient, PWLAN_INTERFACE_INFO_LIST pIfList, PWILBssInfoMap pBssInfoMap)
{
	DWORD dwResult = 0;
	DWORD dwRetVal = 0;

	PWLAN_INTERFACE_INFO pIfInfo = NULL;
	PWLAN_AVAILABLE_NETWORK_LIST pWlanAvailableNetworkList = NULL;
	PWLAN_AVAILABLE_NETWORK pWlanAvailableNetwork = NULL;
	PWLAN_BSS_LIST pWlanBssList = NULL;
	PWLAN_BSS_ENTRY pWlanBssEntry = NULL;

	for (int i = 0; i < (int)pIfList->dwNumberOfItems; i++) {
		pIfInfo = (WLAN_INTERFACE_INFO *)&pIfList->InterfaceInfo[i];

		INT nPower = 0;
		dwResult = WifiInfoLib_QueryRadioPower(&nPower, hClient, pIfInfo);
		if (dwResult != ERROR_SUCCESS) {
			WIL_errno = dwResult;
			return WIL_QUERY_INTERFACE_ERROR;
		}
		if (nPower == WIL_RADIO_POWER_OFF)
		{
			continue;
		}

		dwResult = WlanGetAvailableNetworkList(hClient,
			&pIfInfo->InterfaceGuid,
			0,
			NULL,
			&pWlanAvailableNetworkList);

		if (dwResult != ERROR_SUCCESS) {
			if (pWlanAvailableNetworkList != NULL) {
				WlanFreeMemory(pWlanAvailableNetworkList);
				pWlanAvailableNetworkList = NULL;
			}
			WIL_errno = dwResult;
			return WIL_GET_BSSLIST_ERROR;
		}
		else {
			for (int j = 0; j < (int)pWlanAvailableNetworkList->dwNumberOfItems; j++) {
				pWlanAvailableNetwork = (WLAN_AVAILABLE_NETWORK*)&pWlanAvailableNetworkList->Network[j];

				dwResult = WlanGetNetworkBssList(hClient,
					&pIfInfo->InterfaceGuid,
					&pWlanAvailableNetwork->dot11Ssid,
					pWlanAvailableNetwork->dot11BssType,
					pWlanAvailableNetwork->bSecurityEnabled,
					NULL,
					&pWlanBssList);

				if (dwResult != ERROR_SUCCESS) {
					if (pWlanAvailableNetworkList != NULL) {
						WlanFreeMemory(pWlanAvailableNetworkList);
						pWlanAvailableNetworkList = NULL;
					}
					if (pWlanBssList != NULL) {
						WlanFreeMemory(pWlanBssList);
						pWlanBssList = NULL;
					}
					WIL_errno = dwResult;
					return WIL_GET_BSSLIST_ERROR;
				}
				else {
					for (int j = 0; j < (int)pWlanBssList->dwNumberOfItems; j++) {
						pWlanBssEntry = (WLAN_BSS_ENTRY*)& pWlanBssList->wlanBssEntries[j];

						// Map保存時のユニークIDにMACアドレスを使用
						ULONG64 ulMacAddress = 0;
						for (int n = 0; n < 6; n++) {
							ULONG64 ulTmp = pWlanBssEntry->dot11Bssid[n];
							ulTmp = ulTmp << (8 * (5 - n));
							ulMacAddress += ulTmp;
						}

						WIL_BSS_INFO WILBssInfo;
						SecureZeroMemory(WILBssInfo.elementDataBlob, ELEMENT_DATA_MAX_SIZE);
						GetLocalTime(&(WILBssInfo.lastUpdate));

						// APIで取得したWLAN_AVAILABLE_NETWORK、WLAN_BSS_ENTRYはそのまま保存
						WILBssInfo.wlanAvailableNetwork = *pWlanAvailableNetwork;
						WILBssInfo.wlanBssEntry = *pWlanBssEntry;
						WILBssInfo.stationCount = 0xffff;
						WILBssInfo.channelUtilization = 0xff;
						WILBssInfo.HTOperationChannelWidth = 20;
						WILBssInfo.VHTOperationChannelWidth = 20;

						// Elementのオフセットとサイズ
						ULONG ulIEOffset = pWlanBssEntry->ulIeOffset;
						ULONG ulSizeIEBlob = pWlanBssEntry->ulIeSize;
						LONG_PTR address = (LONG_PTR)pWlanBssEntry + ulIEOffset;
						unsigned char* pOrig = (unsigned char*)((LONG_PTR)pWlanBssEntry + ulIEOffset);

						// 解析用バッファに一時保存
						unsigned char* pIEBlob = (unsigned char*)MALLOC(ulSizeIEBlob);
						memcpy_s(pIEBlob, ulSizeIEBlob, pOrig, ulSizeIEBlob);
						// MAP保存用バッファにも保存
						memcpy_s(WILBssInfo.elementDataBlob, ELEMENT_DATA_MAX_SIZE, pOrig, ulSizeIEBlob);
						WILBssInfo.elementDataBlobSize = ulSizeIEBlob;

						ULONG ulTotalBytes = 0;
						unsigned char ElementID;
						unsigned char length;

						// Elementの解析
						//  Element ID:           1 Octet
						//  Length:               1 Octet
						//  Element ID Extension: 1 Octet Curently only used for Element ID 255
						//  Information:          variable
						while (ulTotalBytes < ulSizeIEBlob) {
							ElementID = *(pIEBlob + ulTotalBytes);
							ulTotalBytes++;
							length = *(pIEBlob + ulTotalBytes);
							ulTotalBytes++;

							// BSS Load (see 9.4.2.28)
							if (ElementID == 11) {
								Element11((unsigned char*)((LONG_PTR)pIEBlob + ulTotalBytes), &WILBssInfo);
							}
							// HT Capabilities (see 9.4.2.56)
							else if (ElementID == 45) {
								Element45((unsigned char*)((LONG_PTR)pIEBlob + ulTotalBytes), &WILBssInfo);
							}
							// HT Operation (see 9.4.2.57)
							else if (ElementID == 61) {
								Element61((unsigned char*)((LONG_PTR)pIEBlob + ulTotalBytes), &WILBssInfo);
							}
							// VHT Operation (see 9.4.2.159)
							else if (ElementID == 192) {
								Element192((unsigned char*)((LONG_PTR)pIEBlob + ulTotalBytes), &WILBssInfo);
							}
							ulTotalBytes += length;
						}
						FREE((LPVOID)pIEBlob);

						// MACアドレスをキーにMAPに保存
						(*pBssInfoMap)[ulMacAddress] = WILBssInfo;
					}
				}

				if (pWlanBssList != NULL) {
					WlanFreeMemory(pWlanBssList);
					pWlanBssList = NULL;
				}
			}
		}

		if (pWlanAvailableNetworkList != NULL) {
			WlanFreeMemory(pWlanAvailableNetworkList);
			pWlanAvailableNetworkList = NULL;
		}
	}


	return WIL_SUCCESS;
}

// Wi-Fi Info Libraryの取得したBSSID情報のマップをファイルに保存する関数
DWORD WifiInfoLib_Write(LPCWSTR lpFilename, WILBssInfoMap bssInfoMap, const std::vector<string> filter)
{
	HANDLE hFile;
	DWORD dwWriteSize;
	int nWritten;
	char buff[WIL_WRITE_BUFFER_SIZE];

	hFile = CreateFile(lpFilename, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		WIL_errno = GetLastError();
		return WIL_FILE_OPEN_ERROR;
		//PLOGE << "File cannnot be opend.";
	}
	SetFilePointer(hFile, 0, NULL, FILE_END);

	// ヘッダー情報
	nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "===========================================================================\n");
	WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);
	nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "BSSID Entries: %lu\n", (unsigned long)bssInfoMap.size());
	WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);

	// BSSIDフィルターが定義されている場合、表示するBSSIDを追加
	if (!filter.empty()) {
		nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "BSSID filter:");
		for (auto itr = filter.begin(); itr != filter.end(); ++itr) {
			string s = (string)*itr;
			nWritten += sprintf_s(buff + nWritten, WIL_WRITE_BUFFER_SIZE - nWritten, " %s", s.c_str());
		}
		nWritten += sprintf_s(buff + nWritten, WIL_WRITE_BUFFER_SIZE - nWritten, "\n");
		WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);
	}

	// おおよそのスキャン日時
	SYSTEMTIME st;
	GetLocalTime(&st);
	nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "Scanned at: %04d/%02d/%02d %02d:%02d(Local time)\n",
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
	WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);
	nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "===========================================================================\n\n");
	WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);

	for (auto itr = bssInfoMap.begin(); itr != bssInfoMap.end(); ++itr) {
		// BSSIDフィルターが定義されている場合、MACアドレスで確認
		if (!filter.empty()) {
			BOOL bFilterMatch = FALSE;
			ULONG64 ulMacAddress = itr->first;
			for (auto itr2 = filter.begin(); itr2 != filter.end(); ++itr2) {
				string s = (string)*itr2;
				ULONG64 ulFilter = stoull(s, nullptr, 16);
				if (ulFilter == ulMacAddress) {
					bFilterMatch = TRUE;
				}
			}

			if (!bFilterMatch) {
				continue;
			}
		}

		WIL_BSS_INFO* pWilBssInfo = (WIL_BSS_INFO*)&itr->second;
		//PWLAN_BSS_ENTRY pWlanBssEntry = &pWilBssInfo->wlanBssEntry;

		nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
		WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);

		char strBuff[128];

		// SSID
		getSSIDString(strBuff, 128, pWilBssInfo);
		nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "  SSID:           %s\n", strBuff);
		//if (pWlanBssEntry->dot11Ssid.uSSIDLength == 0) {
		//	nWritten += sprintf_s(buff + nWritten, WIL_WRITE_BUFFER_SIZE - nWritten, "\n");
		//}
		//else {
		//	for (int k = 0; k < (int)pWlanBssEntry->dot11Ssid.uSSIDLength; k++) {
		//		nWritten += sprintf_s(buff + nWritten, WIL_WRITE_BUFFER_SIZE - nWritten, "%c", (int)pWlanBssEntry->dot11Ssid.ucSSID[k]);
		//	}
		//	nWritten += sprintf_s(buff + nWritten, WIL_WRITE_BUFFER_SIZE - nWritten, "\n");
		//}
		WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);

		// BSSID
		getBSSIDString(strBuff, 128, pWilBssInfo);
		nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "  BSSID:          %s\n", strBuff);
		//nWritten += sprintf_s(buff + nWritten, WIL_WRITE_BUFFER_SIZE - nWritten, "%02X:%02X:%02X:%02X:%02X:%02X",
		//	pWlanBssEntry->dot11Bssid[0],
		//	pWlanBssEntry->dot11Bssid[1],
		//	pWlanBssEntry->dot11Bssid[2],
		//	pWlanBssEntry->dot11Bssid[3],
		//	pWlanBssEntry->dot11Bssid[4],
		//	pWlanBssEntry->dot11Bssid[5]);
		//nWritten += sprintf_s(buff + nWritten, WIL_WRITE_BUFFER_SIZE - nWritten, "\n");
		WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);

		// BSS Type
		getBSSTypeString(strBuff, 128, pWilBssInfo);
		nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "  BSS Type:       %s\n", strBuff);
		WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);

		// Number of BSSIDs
		LONG nNumberOfBssids = 0;
		WifiInfoLib_GetNumberOfBssids(&nNumberOfBssids, pWilBssInfo);
		nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "  Same SSIDs:     %d\n", nNumberOfBssids);
		WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);

		// Cipher
		getCipherString(strBuff, 128, pWilBssInfo);
		nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "  Cipher:         %s\n", strBuff);
		WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);

		// Auth
		getAuthAlgorithmString(strBuff, 128, pWilBssInfo);
		nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "  Auth:           %s\n", strBuff);
		WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);

		// RSSI
		LONG nRSSI = 0;
		WifiInfoLib_GetRSSI(&nRSSI, pWilBssInfo);
		nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "  RSSI:           %lddBm\n", nRSSI);
		WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);

		// Link Quality
		LONG nLinkQuality = 0;
		WifiInfoLib_GetLinkQuality(&nLinkQuality, pWilBssInfo);
		nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "  Link quality:   %ld\n", nLinkQuality);
		WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);

		// Beacon Period
		SHORT nBeaconPeriod = 0;
		WifiInfoLib_GetBeaconPeriod(&nBeaconPeriod, pWilBssInfo);
		nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "  Beacon period:  %ld\n", nBeaconPeriod);
		WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);

		// Channel & Frequency
		INT nCh = 0;
		FLOAT fFreq = 0.0;
		WifiInfoLib_GetChannel(&nCh, &fFreq, pWilBssInfo);
		nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "  Ch(Freq):       %dch(%1.3fGHz)\n", nCh, fFreq);
		WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);

		// Elements
		nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "  Elements:\n");

		// Station count from element id 11
		if (pWilBssInfo->stationCount != 0xffff) {
			nWritten += sprintf_s(buff + nWritten, WIL_WRITE_BUFFER_SIZE - nWritten, "                  StationCount: %u\n", pWilBssInfo->stationCount);
		}

		// Channel utilization from element id 11
		if (pWilBssInfo->channelUtilization != 0xff) {
			unsigned char cu = pWilBssInfo->channelUtilization;
			unsigned int cuPerCent = 100 * (unsigned int)cu / 255;
			nWritten += sprintf_s(buff + nWritten, WIL_WRITE_BUFFER_SIZE - nWritten, "                  Channel Utilization: %u(%d%%)\n", cu, cuPerCent);
		}

		// Channel number
		nWritten += sprintf_s(buff + nWritten, WIL_WRITE_BUFFER_SIZE - nWritten, "                  Channel number: %uch(HT Operation)\n", pWilBssInfo->HTOperationChannel);
		// 2.4GHz Wi-Fi
		if (pWilBssInfo->HTOperationChannel < 20) {
			nWritten += sprintf_s(buff + nWritten, WIL_WRITE_BUFFER_SIZE - nWritten, "                  Channel width: %uMHz(HT Operation)\n", pWilBssInfo->HTOperationChannelWidth);
		}
		// 5GHz Wi-Fi
		else {
			nWritten += sprintf_s(buff + nWritten, WIL_WRITE_BUFFER_SIZE - nWritten, "                  Channel width: %uMHz(VHT Operation)\n", pWilBssInfo->VHTOperationChannelWidth);
		}
		WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);

		// All elements
		ULONG ulSize = pWilBssInfo->elementDataBlobSize;
		unsigned char* pTempBuf = pWilBssInfo->elementDataBlob;
		ULONG ulTotalBytes = 0;
		while (ulTotalBytes < ulSize) {
			unsigned char ElementID = *(pTempBuf + ulTotalBytes);
			ulTotalBytes++;
			unsigned char length = *(pTempBuf + ulTotalBytes);
			ulTotalBytes++;

			nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "  ElementID[%03i]:", ElementID);
			for (int i = 0; i < length; i++) {
				nWritten += sprintf_s(buff + nWritten, WIL_WRITE_BUFFER_SIZE - nWritten, " %02X", *(pTempBuf + ulTotalBytes + i));
			}
			ulTotalBytes += length;
			nWritten += sprintf_s(buff + nWritten, WIL_WRITE_BUFFER_SIZE - nWritten, "\n");
			WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);
		}

		// 最後に情報取得できた時間
		PSYSTEMTIME pSt = &(pWilBssInfo->lastUpdate);
		nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "  Last update:    %04d/%02d/%02d %02d:%02d(Local time)\n",
			pSt->wYear, pSt->wMonth, pSt->wDay, pSt->wHour, pSt->wMinute);
		WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);
	}

	nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");
	WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);

	CloseHandle(hFile);

	return WIL_SUCCESS;
}

DWORD WifiInfoLib_CsvWrite(LPCWSTR lpFilename, WILBssInfoMap bssInfoMap, const std::vector<string> filter)
{
	HANDLE hFile;
	DWORD dwWriteSize;
	int nWritten;
	char buff[WIL_WRITE_BUFFER_SIZE];

	vector<string> csvHeader{"LastUpdate", "SSID", "BSSID", "BSSType", "Cipher", "Auth", "RSSI", "LinkQuality", 
		"BeaconPeriod", "Ch", "Width", "StationCount", "ChUtilization", "Elements"};

	hFile = CreateFile(lpFilename, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		WIL_errno = GetLastError();
		return WIL_FILE_OPEN_ERROR;
		//PLOGE << "File cannnot be opend.";
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		SetFilePointer(hFile, 0, NULL, FILE_END);
	}
	else {
		nWritten = 0;
		for (auto itr = csvHeader.begin(); itr != csvHeader.end(); ++itr) {
			string s = (string)*itr;
			nWritten += sprintf_s(buff + nWritten, WIL_WRITE_BUFFER_SIZE - nWritten, "%s,", s.c_str());
		}
		nWritten += sprintf_s(buff + nWritten, WIL_WRITE_BUFFER_SIZE - nWritten, "\n");
		WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);
	}

	for (auto itr = bssInfoMap.begin(); itr != bssInfoMap.end(); ++itr) {
		// BSSIDフィルターが定義されている場合、MACアドレスで確認
		if (!filter.empty()) {
			BOOL bFilterMatch = FALSE;
			ULONG64 ulMacAddress = itr->first;
			for (auto itr2 = filter.begin(); itr2 != filter.end(); ++itr2) {
				string s = (string)*itr2;
				ULONG64 ulFilter = stoull(s, nullptr, 16);
				if (ulFilter == ulMacAddress) {
					bFilterMatch = TRUE;
				}
			}

			if (!bFilterMatch) {
				continue;
			}
		}

		WIL_BSS_INFO* pWilBssInfo = (WIL_BSS_INFO*)&itr->second;

		char strBuff[128];

		for (auto itr = csvHeader.begin(); itr != csvHeader.end(); ++itr) {
			string s = (string)*itr;

			if (s == "LastUpdate") {
				PSYSTEMTIME pSt = &(pWilBssInfo->lastUpdate);
				nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "%04d/%02d/%02d %02d:%02d:%02d,",
					pSt->wYear, pSt->wMonth, pSt->wDay, pSt->wHour, pSt->wMinute, pSt->wSecond);
				WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);
			}
			else if (s == "SSID") {
				getSSIDString(strBuff, 128, pWilBssInfo);
				nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "%s,", strBuff);
				WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);
			}
			else if (s == "BSSID") {
				getBSSIDString(strBuff, 128, pWilBssInfo);
				nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "%s,", strBuff);
				WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);
			}
			else if (s == "BSSType") {
				getBSSTypeString(strBuff, 128, pWilBssInfo);
				nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "%s,", strBuff);
				WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);
			}
			else if (s == "Cipher") {
				getCipherString(strBuff, 128, pWilBssInfo);
				nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "%s,", strBuff);
				WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);
			}
			else if (s == "Auth") {
				getAuthAlgorithmString(strBuff, 128, pWilBssInfo);
				nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "%s,", strBuff);
				WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);
			}
			else if (s == "RSSI") {
				LONG nRSSI = 0;
				WifiInfoLib_GetRSSI(&nRSSI, pWilBssInfo);
				nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "%ld,", nRSSI);
				WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);
			}
			else if (s == "LinkQuality") {
				LONG nLinkQuality = 0;
				WifiInfoLib_GetLinkQuality(&nLinkQuality, pWilBssInfo);
				nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "%ld,", nLinkQuality);
				WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);
			}
			else if (s == "BeaconPeriod") {
				SHORT nBeaconPeriod = 0;
				WifiInfoLib_GetBeaconPeriod(&nBeaconPeriod, pWilBssInfo);
				nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "%d,", nBeaconPeriod);
				WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);
			}
			else if (s == "Ch") {
				INT nCh = 0;
				WifiInfoLib_GetChannel(&nCh, NULL, pWilBssInfo);
				nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "%d,", nCh);
				WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);
			}
			else if (s == "Width") {
				unsigned char width;
				if (pWilBssInfo->HTOperationChannel < 20) {
					width = pWilBssInfo->HTOperationChannelWidth;
				}
				// 5GHz Wi-Fi
				else {
					width = pWilBssInfo->VHTOperationChannelWidth;
				}
				nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "%d,", width);
				WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);
			}
			else if (s == "StationCount") {
				if (pWilBssInfo->stationCount != 0xffff) {
					nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "%u,", pWilBssInfo->stationCount);
				}
				else {
					nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, ",");
				}
				WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);
			}
			else if (s == "ChUtilization") {
				if (pWilBssInfo->channelUtilization != 0xff) {
					unsigned char cu = pWilBssInfo->channelUtilization;
					unsigned int cuPerCent = 100 * (unsigned int)cu / 255;
					nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "%u(%d%%),", cu, cuPerCent);
				}
				else {
					nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, ",");
				}
				WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);
			}
			else if (s == "Elements") {
				ULONG ulSize = pWilBssInfo->elementDataBlobSize;
				unsigned char* pTempBuf = pWilBssInfo->elementDataBlob;
				ULONG ulTotalBytes = 0;
				while (ulTotalBytes < ulSize) {
					unsigned char ElementID = *(pTempBuf + ulTotalBytes);
					ulTotalBytes++;
					unsigned char length = *(pTempBuf + ulTotalBytes);
					ulTotalBytes++;

					nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, " ElementID[%i]:", ElementID);
					for (int i = 0; i < length; i++) {
						nWritten += sprintf_s(buff + nWritten, WIL_WRITE_BUFFER_SIZE - nWritten, " %02X", *(pTempBuf + ulTotalBytes + i));
					}
					ulTotalBytes += length;
					WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);
				}
			}
		}
		nWritten = sprintf_s(buff, WIL_WRITE_BUFFER_SIZE, "\n");
		WriteFile(hFile, buff, nWritten, &dwWriteSize, NULL);
	}

	CloseHandle(hFile);

	return WIL_SUCCESS;
}

DWORD getSSIDString(char* buff, int bufLen, PWIL_BSS_INFO pWilBssInfo)
{
	int count = 0;
	ULONG ulSSIDLength;
	PWLAN_BSS_ENTRY pWlanBssEntry = &pWilBssInfo->wlanBssEntry;
	ulSSIDLength = pWlanBssEntry->dot11Ssid.uSSIDLength;

	SecureZeroMemory(buff, bufLen);

	if (ulSSIDLength == 0) {
		count += sprintf_s(buff, bufLen, "");
	}
	else if ((int)ulSSIDLength > bufLen) {
		WIL_errno = WIL_LOCAL_ERROR;
		return WIL_INSUFFICIENT_BUFFER;
	}
	else {
		for (int k = 0; k < (int)ulSSIDLength; k++) {
			count += sprintf_s(buff + count, bufLen - count, "%c", (int)pWlanBssEntry->dot11Ssid.ucSSID[k]);
		}
	}

	return WIL_SUCCESS;
}

DWORD getSSIDSJISString(char* buff, int bufLen, PWIL_BSS_INFO pWilBssInfo)
{
	DWORD dwReturn;
	char* pBuff = new char[bufLen];
	dwReturn = getSSIDString(pBuff, (int)bufLen, pWilBssInfo);
	if (dwReturn) {
		delete[] pBuff;
		return dwReturn;
	}

	SecureZeroMemory(buff, bufLen);

	int nLengthUnicode = MultiByteToWideChar(CP_UTF8, 0, pBuff, -1, NULL, 0);
	wchar_t* pBuffUnicode = new wchar_t[nLengthUnicode];
	MultiByteToWideChar(CP_UTF8, 0, pBuff, -1, pBuffUnicode, nLengthUnicode);
	delete[] pBuff;

	int nLengthSJIS = WideCharToMultiByte(CP_THREAD_ACP, 0, pBuffUnicode, -1, NULL, 0, NULL, NULL);
	if (nLengthSJIS > bufLen) {
		delete[] pBuffUnicode;
		WIL_errno = WIL_LOCAL_ERROR;
		return WIL_INSUFFICIENT_BUFFER;
	}
	WideCharToMultiByte(CP_THREAD_ACP, 0, pBuffUnicode, nLengthUnicode + 1, buff, bufLen, NULL, NULL);

	delete[] pBuffUnicode;

	return WIL_SUCCESS;
}

DWORD WifiInfoLib_GetSSIDString(wchar_t* buff, size_t bufLen, PWIL_BSS_INFO pWilBssInfo)
{
	DWORD dwReturn;
	char* pBuff = new char[bufLen];
	dwReturn = getSSIDString(pBuff, (int)bufLen, pWilBssInfo);
	if (dwReturn) {
		return dwReturn;
	}

	SecureZeroMemory(buff, bufLen);

	int nLength = MultiByteToWideChar(CP_UTF8, 0, pBuff, -1, buff, (int)bufLen);
	delete[] pBuff;
	if (nLength == 0) {
		DWORD dwErr = GetLastError();
		if (dwErr == ERROR_INSUFFICIENT_BUFFER) {
			WIL_errno = dwErr;
			return WIL_INSUFFICIENT_BUFFER;
		}
		else {
			WIL_errno = dwErr;
			return WIL_PARAMETER_ERROR;
		}
	}

	//int count = 0;
	//PWLAN_BSS_ENTRY pWlanBssEntry = &pWilBssInfo->wlanBssEntry;

	//SecureZeroMemory(buff, bufLen);

	//if (pWlanBssEntry->dot11Ssid.uSSIDLength == 0) {
	//	count += swprintf_s(buff, bufLen, L"");
	//}
	//else {
	//	for (int k = 0; k < (int)pWlanBssEntry->dot11Ssid.uSSIDLength; k++) {
	//		count += swprintf_s(buff + count, bufLen - count, L"%c", (int)pWlanBssEntry->dot11Ssid.ucSSID[k]);
	//	}
	//	swprintf_s(buff + count, bufLen - count, L"");
	//}

	return WIL_SUCCESS;
}

DWORD getBSSIDString(char* buff, int bufLen, PWIL_BSS_INFO pWilBssInfo)
{
	PWLAN_BSS_ENTRY pWlanBssEntry = &pWilBssInfo->wlanBssEntry;

	SecureZeroMemory(buff, bufLen);

	sprintf_s(buff, bufLen, "%02X:%02X:%02X:%02X:%02X:%02X",
		pWlanBssEntry->dot11Bssid[0],
		pWlanBssEntry->dot11Bssid[1],
		pWlanBssEntry->dot11Bssid[2],
		pWlanBssEntry->dot11Bssid[3],
		pWlanBssEntry->dot11Bssid[4],
		pWlanBssEntry->dot11Bssid[5]);

	return WIL_SUCCESS;
}

DWORD WifiInfoLib_GetBSSIDString(wchar_t* buff, size_t bufLen, PWIL_BSS_INFO pWilBssInfo)
{
	PWLAN_BSS_ENTRY pWlanBssEntry = &pWilBssInfo->wlanBssEntry;

	SecureZeroMemory(buff, bufLen);

	swprintf_s(buff, bufLen, L"%02X:%02X:%02X:%02X:%02X:%02X",
		pWlanBssEntry->dot11Bssid[0],
		pWlanBssEntry->dot11Bssid[1],
		pWlanBssEntry->dot11Bssid[2],
		pWlanBssEntry->dot11Bssid[3],
		pWlanBssEntry->dot11Bssid[4],
		pWlanBssEntry->dot11Bssid[5]);

	return WIL_SUCCESS;
}

DWORD WifiInfoLib_GetRSSI(PLONG pValue, PWIL_BSS_INFO pWilBssInfo)
{
	PWLAN_BSS_ENTRY pWlanBssEntry = &pWilBssInfo->wlanBssEntry;
	if (pValue == NULL) {
		WIL_errno = WIL_LOCAL_ERROR;
		return WIL_PARAMETER_ERROR;
	}
	*pValue = pWlanBssEntry->lRssi;
	return WIL_SUCCESS;
}

DWORD WifiInfoLib_GetLinkQuality(PLONG pValue, PWIL_BSS_INFO pWilBssInfo)
{
	PWLAN_BSS_ENTRY pWlanBssEntry = &pWilBssInfo->wlanBssEntry;
	if (pValue == NULL) {
		WIL_errno = WIL_LOCAL_ERROR;
		return WIL_PARAMETER_ERROR;
	}
	*pValue = pWlanBssEntry->uLinkQuality;
	return WIL_SUCCESS;
}

DWORD WifiInfoLib_GetSignalQuality(PLONG pValue, PFLOAT pRssi, PWIL_BSS_INFO pWilBssInfo)
{
	PWLAN_AVAILABLE_NETWORK pWlanAvailableNetwork = &pWilBssInfo->wlanAvailableNetwork;
	*pValue = pWlanAvailableNetwork->wlanSignalQuality;
	*pRssi = 0.5f * (*pValue) - 100;
	return WIL_SUCCESS;
}

DWORD WifiInfoLib_GetMaxSupportedRate(PLONG pValue, PWIL_BSS_INFO pWilBssInfo)
{
	PWLAN_BSS_ENTRY pWlanBssEntry = &pWilBssInfo->wlanBssEntry;
	if (pValue == NULL) {
		WIL_errno = WIL_LOCAL_ERROR;
		return WIL_PARAMETER_ERROR;
	}

	PWLAN_RATE_SET pWlanRateSet = &pWlanBssEntry->wlanRateSet;
	ULONG ulLength = pWlanRateSet->uRateSetLength;
	USHORT rate_in_mbps;

	if (ulLength == 0) {
		rate_in_mbps = 0;
	}
	else {
		USHORT usMax = (pWlanRateSet->usRateSet[0] & 0x7FFF);
		for (unsigned int i = 1; i < ulLength; i++) {
			USHORT usData = (pWlanRateSet->usRateSet[i] & 0x7FFF);
			if (usMax < usData) {
				usMax = usData;
			}
		}
		rate_in_mbps = usMax / 2;
	}

	*pValue = rate_in_mbps;
	return WIL_SUCCESS;
}

DWORD WifiInfoLib_GetChannel(PINT pCh, PFLOAT pFreq, PWIL_BSS_INFO pWilBssInfo)
{
	PWLAN_BSS_ENTRY pWlanBssEntry = &pWilBssInfo->wlanBssEntry;

	if (pCh == NULL) {
		WIL_errno = WIL_LOCAL_ERROR;
		return WIL_PARAMETER_ERROR;
	}
	*pCh = getChannelFromFrequency(pWlanBssEntry->ulChCenterFrequency);

	if (pFreq) {
		*pFreq = (float)(pWlanBssEntry->ulChCenterFrequency) / 1000000.0f;
	}
	return WIL_SUCCESS;
}

DWORD WifiInfoLib_GetNumberOfBssids(PLONG pValue, PWIL_BSS_INFO pWilBssInfo)
{
	PWLAN_AVAILABLE_NETWORK pWlanAvailableNetwork = &pWilBssInfo->wlanAvailableNetwork;
	*pValue = pWlanAvailableNetwork->uNumberOfBssids;
	return WIL_SUCCESS;
}

DWORD WifiInfoLib_GetBeaconPeriod(PSHORT pValue, PWIL_BSS_INFO pWilBssInfo)
{
	PWLAN_BSS_ENTRY pWlanBssEntry = &pWilBssInfo->wlanBssEntry;
	*pValue = pWlanBssEntry->usBeaconPeriod;
	return WIL_SUCCESS;
}

DWORD getAuthAlgorithmString(char* buff, int bufLen, PWIL_BSS_INFO pWilBssInfo)
{
	PWLAN_AVAILABLE_NETWORK pWlanAvailableNetwork = &pWilBssInfo->wlanAvailableNetwork;
	SecureZeroMemory(buff, bufLen);

	switch (pWlanAvailableNetwork->dot11DefaultAuthAlgorithm)
	{
	case DOT11_AUTH_ALGO_80211_OPEN:
		sprintf_s(buff, bufLen, "802.11 Open");
		break;
	case DOT11_AUTH_ALGO_80211_SHARED_KEY:
		sprintf_s(buff, bufLen, "802.11 Shared");
		break;
	case DOT11_AUTH_ALGO_WPA:
		sprintf_s(buff, bufLen, "WPA");
		break;
	case DOT11_AUTH_ALGO_WPA_PSK:
		sprintf_s(buff, bufLen, "WPA-PSK");
		break;
	case DOT11_AUTH_ALGO_WPA_NONE:
		sprintf_s(buff, bufLen, "WPA-None");
		break;
	case DOT11_AUTH_ALGO_RSNA:
		sprintf_s(buff, bufLen, "RSNA");
		break;
	case DOT11_AUTH_ALGO_RSNA_PSK:
		sprintf_s(buff, bufLen, "RSNA with PSK");
		break;
	default:
		sprintf_s(buff, bufLen, "Other (0x%x)", pWlanAvailableNetwork->dot11DefaultAuthAlgorithm);
		break;
	}

	return WIL_SUCCESS;
}

DWORD WifiInfoLib_GetAuthAlgorithmString(wchar_t* buff, size_t bufLen, PWIL_BSS_INFO pWilBssInfo)
{
	DWORD dwReturn;
	SecureZeroMemory(buff, bufLen);
	char* pBuff = new char[bufLen];
	dwReturn = getAuthAlgorithmString(pBuff, (int)bufLen, pWilBssInfo);
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, buff, strlen(pBuff) + 1, pBuff, _TRUNCATE);
	delete[] pBuff;

	return WIL_SUCCESS;
}

DWORD getCipherString(char* buff, int bufLen, PWIL_BSS_INFO pWilBssInfo)
{
	PWLAN_AVAILABLE_NETWORK pWlanAvailableNetwork = &pWilBssInfo->wlanAvailableNetwork;
	SecureZeroMemory(buff, bufLen);

	switch (pWlanAvailableNetwork->dot11DefaultCipherAlgorithm)
	{
	case DOT11_CIPHER_ALGO_NONE:
		sprintf_s(buff, bufLen, "None");
		break;
	case DOT11_CIPHER_ALGO_WEP40:
		sprintf_s(buff, bufLen, "WEP-40");
		break;
	case DOT11_CIPHER_ALGO_TKIP:
		sprintf_s(buff, bufLen, "TKIP");
		break;
	case DOT11_CIPHER_ALGO_CCMP:
		sprintf_s(buff, bufLen, "CCMP");
		break;
	case DOT11_CIPHER_ALGO_WEP104:
		sprintf_s(buff, bufLen, "WEP-104");
		break;
	case DOT11_CIPHER_ALGO_WPA_USE_GROUP:
		sprintf_s(buff, bufLen, "WPA/RSN Use Group Key");
		break;
	//case DOT11_CIPHER_ALGO_RSN_USE_GROUP:
	//	sprintf_s(buff, bufLen, "RSN Use Group Key");
	//	break;
	case DOT11_CIPHER_ALGO_WEP:
		sprintf_s(buff, bufLen, "WEP");
		break;
	default:
		sprintf_s(buff, bufLen, "Other (0x%x)", pWlanAvailableNetwork->dot11DefaultCipherAlgorithm);
		break;
	}

	return WIL_SUCCESS;
}

DWORD WifiInfoLib_GetCipherString(wchar_t* buff, size_t bufLen, PWIL_BSS_INFO pWilBssInfo)
{
	DWORD dwReturn;
	SecureZeroMemory(buff, bufLen);
	char* pBuff = new char[bufLen];
	dwReturn = getCipherString(pBuff, (int)bufLen, pWilBssInfo);
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, buff, strlen(pBuff) + 1, pBuff, _TRUNCATE);
	delete[] pBuff;

	//PWLAN_AVAILABLE_NETWORK pWlanNetworkEntry = &pWilBssInfo->wlanAvailableNetwork;

	//SecureZeroMemory(buff, bufLen);

	//switch (pWlanNetworkEntry->dot11DefaultCipherAlgorithm)
	//{
	//	case DOT11_CIPHER_ALGO_NONE:
	//		swprintf_s(buff, bufLen, L"None");
	//		break;
	//	case DOT11_CIPHER_ALGO_WEP40:
	//		swprintf_s(buff, bufLen, L"WEP40");
	//		break;
	//	case DOT11_CIPHER_ALGO_TKIP:
	//		swprintf_s(buff, bufLen, L"TKIP");
	//		break;
	//	case DOT11_CIPHER_ALGO_CCMP:
	//		swprintf_s(buff, bufLen, L"CCMP");
	//		break;
	//	case DOT11_CIPHER_ALGO_WEP104:
	//		swprintf_s(buff, bufLen, L"WEP104");
	//		break;
	//	case DOT11_CIPHER_ALGO_WPA_USE_GROUP:
	//		swprintf_s(buff, bufLen, L"WPA/RSN Use Group Key");
	//		break;
	//	//case DOT11_CIPHER_ALGO_RSN_USE_GROUP:
	//	//	swprintf_s(buff, bufLen, L"RSN Use Group Key");
	//	//	break;
	//	case DOT11_CIPHER_ALGO_WEP:
	//		swprintf_s(buff, bufLen, L"WEP");
	//		break;
	//	default:
	//		swprintf_s(buff, bufLen, L"Unknown Cipher");
	//		break;
	//}

	return WIL_SUCCESS;
}

DWORD getBSSTypeString(char* buff, int bufLen, PWIL_BSS_INFO pWilBssInfo)
{
	PWLAN_BSS_ENTRY pWlanBssEntry = &pWilBssInfo->wlanBssEntry;

	SecureZeroMemory(buff, bufLen);

	if (pWlanBssEntry->dot11BssType == dot11_BSS_type_infrastructure) {
		sprintf_s(buff, bufLen, "Infrastructure");
	}
	else if (pWlanBssEntry->dot11BssType == dot11_BSS_type_independent) {
		sprintf_s(buff, bufLen, "Ad hoc");
	}
	else {
		sprintf_s(buff, bufLen, "Unknown");
	}

	return WIL_SUCCESS;
}

DWORD WifiInfoLib_GetBSSTypeString(wchar_t* buff, size_t bufLen, PWIL_BSS_INFO pWilBssInfo)
{
	DWORD dwReturn;
	SecureZeroMemory(buff, bufLen);
	char* pBuff = new char[bufLen];
	dwReturn = getBSSTypeString(pBuff, (int)bufLen, pWilBssInfo);
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, buff, strlen(pBuff) + 1, pBuff, _TRUNCATE);
	delete[] pBuff;

	return WIL_SUCCESS;
}
