// scanSSIDList.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <wlanapi.h>
#include <time.h>
#include <sstream>
#include <regex>
#include "../WifiInfoLib/WifiInfoLib.h"
#include "scanSSIDList.h"

// Need to link with Wlanapi.lib and Ole32.lib
#pragma comment(lib, "wlanapi.lib")
#pragma comment(lib, "ole32.lib")

WILBssInfoMap bssInfoMap;
BOOL bShowIE = FALSE;
std::vector<string> BSSIDFilter;

WCHAR strSaveTextFilename[128];
WCHAR strSaveCsvFilename[128];
#define SAVE_CSV_IN_EACH_SCAN (-1)

#define TITLE_OFFSET	(2)
#define VALUE_OFFSET	(24)

//int getSSIDList()
//{
//
//	// Declare and initialize variables.
//
//	HANDLE hClient = NULL;
//	DWORD dwMaxClient = 2;      //    
//	DWORD dwCurVersion = 0;
//	DWORD dwResult = 0;
//	DWORD dwRetVal = 0;
//	int iRet = 0;
//
//	WCHAR GuidString[39] = { 0 };
//
//	unsigned int i, j, k;
//
//	/* variables used for WlanEnumInterfaces  */
//
//	PWLAN_INTERFACE_INFO_LIST pIfList = NULL;
//	PWLAN_INTERFACE_INFO pIfInfo = NULL;
//
//	PWLAN_BSS_LIST pWlanBssList = NULL;
//	PWLAN_BSS_ENTRY pWlanBssEntry = NULL;
//
//	int iRSSI = 0;
//
//	DWORD dwPrevNotif = 0;
//
//	dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
//	if (dwResult != ERROR_SUCCESS) {
//		wprintf(L"WlanOpenHandle failed with error: %u\n", dwResult);
//		return 1;
//		// You can use FormatMessage here to find out why the function failed
//	}
//
//	dwResult = WlanEnumInterfaces(hClient, NULL, &pIfList);
//	if (dwResult != ERROR_SUCCESS) {
//		wprintf(L"WlanEnumInterfaces failed with error: %u\n", dwResult);
//		return 1;
//		// You can use FormatMessage here to find out why the function failed
//	}
//	else {
//		wprintf(L"Num Entries: %lu\n", pIfList->dwNumberOfItems);
//		wprintf(L"Current Index: %lu\n", pIfList->dwIndex);
//		for (i = 0; i < (int)pIfList->dwNumberOfItems; i++) {
//			pIfInfo = (WLAN_INTERFACE_INFO *)&pIfList->InterfaceInfo[i];
//			wprintf(L"  Interface Index[%u]:\t %lu\n", i, i);
//			iRet = StringFromGUID2(pIfInfo->InterfaceGuid, (LPOLESTR)&GuidString,
//				sizeof(GuidString) / sizeof(*GuidString));
//			// For c rather than C++ source code, the above line needs to be
//			// iRet = StringFromGUID2(&pIfInfo->InterfaceGuid, (LPOLESTR) &GuidString, 
//			//     sizeof(GuidString)/sizeof(*GuidString)); 
//			if (iRet == 0)
//				wprintf(L"StringFromGUID2 failed\n");
//			else {
//				wprintf(L"  InterfaceGUID[%d]: %ws\n", i, GuidString);
//			}
//			wprintf(L"  Interface Description[%d]: %ws", i,
//				pIfInfo->strInterfaceDescription);
//			wprintf(L"\n");
//			wprintf(L"  Interface State[%d]:\t ", i);
//			switch (pIfInfo->isState) {
//			case wlan_interface_state_not_ready:
//				wprintf(L"Not ready\n");
//				break;
//			case wlan_interface_state_connected:
//				wprintf(L"Connected\n");
//				break;
//			case wlan_interface_state_ad_hoc_network_formed:
//				wprintf(L"First node in a ad hoc network\n");
//				break;
//			case wlan_interface_state_disconnecting:
//				wprintf(L"Disconnecting\n");
//				break;
//			case wlan_interface_state_disconnected:
//				wprintf(L"Not connected\n");
//				break;
//			case wlan_interface_state_associating:
//				wprintf(L"Attempting to associate with a network\n");
//				break;
//			case wlan_interface_state_discovering:
//				wprintf(L"Auto configuration is discovering settings for the network\n");
//				break;
//			case wlan_interface_state_authenticating:
//				wprintf(L"In process of authenticating\n");
//				break;
//			default:
//				wprintf(L"Unknown state %ld\n", pIfInfo->isState);
//				break;
//			}
//			wprintf(L"\n");
//
//			WLAN_CALLBACK_CONTEXT callbackContext = { 0 };
//			callbackContext.interfaceGUID = pIfInfo->InterfaceGuid;
//			callbackContext.scanEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
//
//			//コールバックの登録
//			dwResult = WlanRegisterNotification(hClient, WLAN_NOTIFICATION_SOURCE_ACM, FALSE, (WLAN_NOTIFICATION_CALLBACK)WlanNotification, (PVOID)&callbackContext, NULL, &dwPrevNotif);
//			if (dwResult != ERROR_SUCCESS) {
//				wprintf(L"WlanRegisterNotification failed with error: %u\n", dwResult);
//				return 1;
//			}
//
//			//利用可能なネットワークをスキャン
//			dwResult = WlanScan(hClient, &pIfInfo->InterfaceGuid, NULL, NULL, NULL);
//			if (dwResult != ERROR_SUCCESS) {
//				wprintf(L"WlanScan failed with error: %u\n", dwResult);
//				return 1;
//			}
//
//			DWORD waitResult = WaitForSingleObject(callbackContext.scanEvent, 15000);
//
//			if (waitResult == WAIT_TIMEOUT) {
//				wprintf(L"No response was received after 15 seconds by calling WlanScan.\n");
//				dwRetVal = 1;
//			}
//			else if (waitResult == WAIT_OBJECT_0) {
//				dwResult = WlanGetNetworkBssList(hClient,
//					&pIfInfo->InterfaceGuid,
//					NULL,
//					dot11_BSS_type_any,
//					TRUE,
//					NULL,
//					&pWlanBssList);
//
//				if (dwResult != ERROR_SUCCESS) {
//					wprintf(L"WlanGetNetworkBssList failed with error: %u\n",
//						dwResult);
//					dwRetVal = 1;
//					// You can use FormatMessage to find out why the function failed
//				}
//				else {
//					wprintf(L"PWLAN_BSS_LIST for this interface\n");
//
//					wprintf(L"  Num Entries: %lu\n\n", pWlanBssList->dwNumberOfItems);
//
//					for (j = 0; j < pWlanBssList->dwNumberOfItems; j++) {
//						pWlanBssEntry =
//							(WLAN_BSS_ENTRY*)& pWlanBssList->wlanBssEntries[j];
//
//						wprintf(L"  SSID[%u]:\t\t ", j);
//						if (pWlanBssEntry->dot11Ssid.uSSIDLength == 0)
//							wprintf(L"\n");
//						else {
//							for (k = 0; k < pWlanBssEntry->dot11Ssid.uSSIDLength; k++) {
//								wprintf(L"%c", (int)pWlanBssEntry->dot11Ssid.ucSSID[k]);
//							}
//							wprintf(L"\n");
//						}
//
//						wprintf(L"  BSSID[%u]:\t\t ", j);
//						wchar_t buff[128];
//						swprintf_s(buff, 128, L"%02X:%02X:%02X:%02X:%02X:%02X",
//							pWlanBssEntry->dot11Bssid[0],
//							pWlanBssEntry->dot11Bssid[1],
//							pWlanBssEntry->dot11Bssid[2],
//							pWlanBssEntry->dot11Bssid[3],
//							pWlanBssEntry->dot11Bssid[4],
//							pWlanBssEntry->dot11Bssid[5]);
//						wprintf(L"%s\n", buff);
//
//						wprintf(L"  RSSI[%u]:\t\t %idBm\n", j, pWlanBssEntry->lRssi);
//						wprintf(L"  Ch(Freq)[%u]:\t\t %ich(%1.3fGHz)\n", j,
//							getChannelFromFrequency(pWlanBssEntry->ulChCenterFrequency),
//							(float)pWlanBssEntry->ulChCenterFrequency / 1000000.0);
//
//						ULONG ulIEOffset = pWlanBssEntry->ulIeOffset;
//						ULONG ulSizeIEBlob = pWlanBssEntry->ulIeSize;
//						unsigned char* pOrig = (unsigned char*)pWlanBssEntry + ulIEOffset;
//						unsigned char* pIEBlob = (unsigned char*)MALLOC(ulSizeIEBlob);
//
//						memcpy_s(pIEBlob, ulSizeIEBlob, pOrig, ulSizeIEBlob);
//
//						ULONG ulTotalBytes = 0;
//						unsigned char ElementID;
//						unsigned char length;
//						unsigned char* pTempBuf = pIEBlob;
//						WLAN_BSS_LOAD_ELEMENT* pBssLoad;
//						unsigned char nHTOpCh = -1;
//						unsigned char nHTOpChWidth = 20;
//						while (ulTotalBytes <= ulSizeIEBlob) {
//							ElementID = *(pTempBuf + ulTotalBytes);
//							ulTotalBytes++;
//							length = *(pTempBuf + ulTotalBytes);
//							ulTotalBytes++;
//							// BSS Load (see 9.4.2.28)
//							if (ElementID == 11) {
//								pBssLoad = (WLAN_BSS_LOAD_ELEMENT*)(pTempBuf + ulTotalBytes);
//								wprintf(L"  ElementID[%i]:\t", ElementID);
//								for (int i = 0; i < length; i++) {
//									wprintf(L" %02X", *(pTempBuf + ulTotalBytes));
//									ulTotalBytes++;
//								}
//								wprintf(L"\n");
//								wprintf(L"    StationCount: %X\n", pBssLoad->stationCount);
//								unsigned char cu = pBssLoad->channelUtilization;
//								unsigned int cuPerCent = 100 * (unsigned int)cu / 255;
//								wprintf(L"    Channel Utilization: %X(%d%%)\n", cu, cuPerCent);
//								wprintf(L"    Available Admission Capability: %X\n", pBssLoad->availableAdmissionCapacity);
//							}
//							// HT Operation (see 9.4.2.57)
//							else if (ElementID == 61) {
//								nHTOpCh = *(pTempBuf + ulTotalBytes);
//								wprintf(L"  Ch(Freq)[%u]:\t\t %ich(HT Operation)\n", j, nHTOpCh);
//								unsigned char nTmp = *(pTempBuf + ulTotalBytes + 1);
//								if (nTmp & 0x03) {
//									nHTOpChWidth = 40;
//								}
//								else {
//									nHTOpChWidth = 20;
//								}
//								if (nHTOpCh < 20) {
//									wprintf(L"  Ch width[%u]:\t\t %iMHz(HT Operation)\n", j, nHTOpChWidth);
//								}
//								ulTotalBytes += length;
//							}
//							// VHT Operation (see 9.4.2.159)
//							else if (ElementID == 192) {
//								unsigned char nVHTOpChWidth = 40;
//								unsigned char nWidth = *(pTempBuf + ulTotalBytes);
//								if (nWidth == 0) {
//									if (nHTOpChWidth == 20) {
//										nVHTOpChWidth = 20;
//									}
//									else {
//										nVHTOpChWidth = 40;
//									}
//								}
//								else if (nWidth == 1) {
//									unsigned char CCFS0 = *(pTempBuf + ulTotalBytes + 1);
//									unsigned char CCFS1 = *(pTempBuf + ulTotalBytes + 2);
//									unsigned char diffAbs = abs(CCFS1 - CCFS0);
//									if (CCFS1 == 0) {
//										nVHTOpChWidth = 80;
//									}
//									else if ((CCFS1 > 0) && (diffAbs == 8)) {
//										nVHTOpChWidth = 160;
//									}
//									else if ((CCFS1 > 0) && (diffAbs > 16)) {
//										nVHTOpChWidth = 254;
//									}
//									else {
//										nVHTOpChWidth = 255;
//									}
//								}
//								// deprecated...
//								else if (nWidth == 2) {
//									nVHTOpChWidth = 160;
//								}
//								// deprecated...
//								else if (nWidth == 3) {
//									nVHTOpChWidth = 254;
//								}
//								wprintf(L"  Ch width[%u]:\t\t %iMHz(VHT Operation)\n", j, nVHTOpChWidth);
//								ulTotalBytes += length;
//							}
//							else {
//								if (bShowIE) {
//									wprintf(L"  \tElementID[%i]:\t", ElementID);
//									for (int i = 0; i < length; i++) {
//										wprintf(L" %02X", *(pTempBuf + ulTotalBytes));
//										ulTotalBytes++;
//									}
//									wprintf(L"\n");
//								}
//								else {
//									ulTotalBytes += length;
//								}
//							}
//						}
//						wprintf(L"\n");
//						FREE(pIEBlob);
//					}
//				}
//			}
//			else {
//				wprintf(L"Unknown error waiting for response. Error Code: %x\n", waitResult);
//				dwRetVal = 1;
//			}
//		}
//	}
//
//	//通知の解除
//	if (hClient != NULL) {
//		dwResult = WlanRegisterNotification(hClient, WLAN_NOTIFICATION_SOURCE_NONE, TRUE, NULL, NULL, NULL, &dwPrevNotif);
//	}
//	if (pWlanBssList != NULL) {
//		WlanFreeMemory(pWlanBssList);
//		pWlanBssList = NULL;
//	}
//
//	if (pIfList != NULL) {
//		WlanFreeMemory(pIfList);
//		pIfList = NULL;
//	}
//
//	return dwRetVal;
//}

static DWORD WIL_Sample_Show(WILBssInfoMap bssInfoMap, BOOL bShowIE, const std::vector<string> filter)
{
	int nCount = 0;

	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

	wprintf(L"+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

	for (auto itr = bssInfoMap.begin(); itr != bssInfoMap.end(); ++itr) {
		if (!filter.empty()) {
			BOOL bFilterMatch = FALSE;
			ULONG64 ulMacAddress = itr->first;
			for (auto itr = filter.begin(); itr != filter.end(); ++itr) {
				string s = (string)*itr;
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
		PWLAN_BSS_ENTRY pWlanBssEntry = &pWilBssInfo->wlanBssEntry;

		wchar_t buff[128];

		CONSOLE_SCREEN_BUFFER_INFO csbi;
		COORD st;
		GetConsoleScreenBufferInfo(h, &csbi);

		// SSID
		WifiInfoLib_GetSSIDString(buff, 128, pWilBssInfo);
		st.X = TITLE_OFFSET;
		st.Y = csbi.dwCursorPosition.Y;
		SetConsoleCursorPosition(h, st);
		wprintf(L"SSID[%d]:", nCount);
		st.X = VALUE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"%s\n", buff);
		//if (pWlanBssEntry->dot11Ssid.uSSIDLength == 0) {
		//	wprintf(L"\n");
		//}
		//else {
		//	for (int k = 0; k < (int)pWlanBssEntry->dot11Ssid.uSSIDLength; k++) {
		//		wprintf(L"%c", (int)pWlanBssEntry->dot11Ssid.ucSSID[k]);
		//	}
		//	wprintf(L"\n");
		//}

		// BSSID
		WifiInfoLib_GetBSSIDString(buff, 128, pWilBssInfo);
		st.X = TITLE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"BSSID[%d]:", nCount);
		st.X = VALUE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"%s\n", buff);
		//swprintf_s(buff, 128, L"%02X:%02X:%02X:%02X:%02X:%02X",
		//	pWlanBssEntry->dot11Bssid[0],
		//	pWlanBssEntry->dot11Bssid[1],
		//	pWlanBssEntry->dot11Bssid[2],
		//	pWlanBssEntry->dot11Bssid[3],
		//	pWlanBssEntry->dot11Bssid[4],
		//	pWlanBssEntry->dot11Bssid[5]);
		//wprintf(L"%s\n", buff);

		// BSS Type
		WifiInfoLib_GetBSSTypeString(buff, 128, pWilBssInfo);
		st.X = TITLE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"BSSType[%d]:", nCount);
		st.X = VALUE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"%s\n", buff);

		// Number of BSSIDs
		LONG nNumberOfBssids = 0;
		WifiInfoLib_GetNumberOfBssids(&nNumberOfBssids, pWilBssInfo);
		st.X = TITLE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"Number of BSSID[%d]:", nCount);
		st.X = VALUE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"%d\n", nNumberOfBssids);

		// Cipher
		WifiInfoLib_GetCipherString(buff, 128, pWilBssInfo);
		st.X = TITLE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"Cipher[%d]:", nCount);
		st.X = VALUE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"%s\n", buff);

		// Auth
		WifiInfoLib_GetAuthAlgorithmString(buff, 128, pWilBssInfo);
		st.X = TITLE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"Auth[%d]:", nCount);
		st.X = VALUE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"%s\n", buff);

		// RSSI
		LONG nRSSI = 0;
		WifiInfoLib_GetRSSI(&nRSSI, pWilBssInfo);
		st.X = TITLE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"RSSI[%d]:", nCount);
		st.X = VALUE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"%lddBm\n", nRSSI);
		//wprintf(L"  RSSI[%u]:\t\t%idBm\n", nCount, pWlanBssEntry->lRssi);

		// Link Quality
		LONG nLinkQuality = 0;
		WifiInfoLib_GetLinkQuality(&nLinkQuality, pWilBssInfo);
		st.X = TITLE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"Link quality[%d]:", nCount);
		st.X = VALUE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"%ld\n", nLinkQuality);

		// Signal Quality
		LONG nSignalQuality = 0;
		FLOAT fRssi = 0.0f;
		WifiInfoLib_GetSignalQuality(&nSignalQuality, &fRssi, pWilBssInfo);
		st.X = TITLE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"Signal quality[%d]:", nCount);
		st.X = VALUE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"%ld(%3.1fdBm)\n", nSignalQuality, fRssi);

		// Beacon Period
		SHORT nBeaconPeriod = 0;
		WifiInfoLib_GetBeaconPeriod(&nBeaconPeriod, pWilBssInfo);
		st.X = TITLE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"Beacon period[%d]:", nCount);
		st.X = VALUE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"%dTU\n", nBeaconPeriod);

		// Max Rate
		LONG nMaxRate = 0;
		WifiInfoLib_GetMaxSupportedRate(&nMaxRate, pWilBssInfo);
		st.X = TITLE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"Max rate[%d]:", nCount);
		st.X = VALUE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"%ldMbps\n", nMaxRate);

		// Channel & Frequency
		INT nCh = 0;
		FLOAT fFreq = 0.0;
		WifiInfoLib_GetChannel(&nCh, &fFreq, pWilBssInfo);
		st.X = TITLE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"Ch(Freq)[%d]:", nCount);
		st.X = VALUE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"%dch(%1.3fGHz)\n", nCh, fFreq);
		//wprintf(L"  Ch(Freq)[%u]:\t\t%ich(%1.3fGHz)\n", nCount,
		//	getChannelFromFrequency(pWlanBssEntry->ulChCenterFrequency),
		//	(float)pWlanBssEntry->ulChCenterFrequency / 1000000.0);

		// Elements
		st.X = TITLE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"Elements[%u]:", nCount);

		if (pWilBssInfo->stationCount != 0xffff) {
			st.X = VALUE_OFFSET;
			SetConsoleCursorPosition(h, st);
			wprintf(L"StationCount: %u\n", pWilBssInfo->stationCount);
		}

		if (pWilBssInfo->channelUtilization != 0xff) {
			unsigned char cu = pWilBssInfo->channelUtilization;
			unsigned int cuPerCent = 100 * (unsigned int)cu / 255;
			st.X = VALUE_OFFSET;
			SetConsoleCursorPosition(h, st);
			wprintf(L"Channel Utilization: %u(%d%%)\n", cu, cuPerCent);
		}

		st.X = VALUE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"Channel number: %ich(HT Operation)\n", pWilBssInfo->HTOperationChannel);

		st.X = VALUE_OFFSET;
		SetConsoleCursorPosition(h, st);
		// 2.4GHz Wi-Fi
		if (pWilBssInfo->HTOperationChannel < 20) {
			wprintf(L"Channel width: %dMHz(HT Operation)\n", pWilBssInfo->HTOperationChannelWidth);
		}
		// 5GHz Wi-Fi
		else {
			wprintf(L"Channel width: %dMHz(VHT Operation)\n", pWilBssInfo->VHTOperationChannelWidth);
		}

		if (bShowIE) {
			ULONG ulSize = pWilBssInfo->elementDataBlobSize;
			unsigned char* pTempBuf = pWilBssInfo->elementDataBlob;
			ULONG ulTotalBytes = 0;
			while (ulTotalBytes < ulSize) {
				unsigned char ElementID = *(pTempBuf + ulTotalBytes);
				ulTotalBytes++;
				unsigned char length = *(pTempBuf + ulTotalBytes);
				ulTotalBytes++;

				st.X = TITLE_OFFSET;
				SetConsoleCursorPosition(h, st);
				wprintf(L"ElementID[%i]:", ElementID);
				st.X = VALUE_OFFSET - 1;
				SetConsoleCursorPosition(h, st);
				for (int i = 0; i < length; i++) {
					wprintf(L" %02X", *(pTempBuf + ulTotalBytes + i));
				}
				ulTotalBytes += length;
				wprintf(L"\n");
			}
		}

		PSYSTEMTIME pSt = &(pWilBssInfo->lastUpdate);
		st.X = TITLE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"Last update:");
		st.X = VALUE_OFFSET;
		SetConsoleCursorPosition(h, st);
		wprintf(L"%04d/%02d/%02d %02d:%02d(Local time)\n",
			pSt->wYear, pSt->wMonth, pSt->wDay, pSt->wHour, pSt->wMinute);

		wprintf(L"+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

		nCount++;
	}

	wprintf(L"\nBSSID Entries: %lu\n", (unsigned long)bssInfoMap.size());
	// BSSIDフィルターが定義されている場合、表示するBSSIDを追加
	if (!filter.empty()) {
		wprintf(L"BSSID filter:");
		for (auto itr = filter.begin(); itr != filter.end(); ++itr) {
			string s = (string)*itr;
			printf(" %s", s.c_str());
		}
		wprintf(L"\n");
	}
	wprintf(L"\n===========================================================================\n\n");

	return 0;
}

int getIntValue(WCHAR* strBuffW)
{
	int nInt = _wtoi(strBuffW);
	if (nInt == 0) {
		if (wcscmp(strBuffW, L"0") != 0) {
			nInt = -1;
		}
	}
	return nInt;
}

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		// Handle the CTRL-C signal.
		case CTRL_C_EVENT:
		// CTRL-CLOSE: confirm that the user wants to exit.
		case CTRL_CLOSE_EVENT:
		// Pass other signals to the next handler.
		case CTRL_BREAK_EVENT:
		case CTRL_LOGOFF_EVENT:
		case CTRL_SHUTDOWN_EVENT:
			WifiInfoLib_Write(strSaveTextFilename, bssInfoMap, BSSIDFilter);
			WifiInfoLib_CsvWrite(strSaveCsvFilename, bssInfoMap, BSSIDFilter);
			return FALSE;
/*
		// CTRL-CLOSE: confirm that the user wants to exit.
		case CTRL_CLOSE_EVENT:
			Beep(600, 200);
			printf("Ctrl-Close event\n\n");
			return TRUE;

		// Pass other signals to the next handler.
		case CTRL_BREAK_EVENT:
			Beep(900, 200);
			printf("Ctrl-Break event\n\n");
			return FALSE;

		case CTRL_LOGOFF_EVENT:
			Beep(1000, 200);
			printf("Ctrl-Logoff event\n\n");
			return FALSE;

		case CTRL_SHUTDOWN_EVENT:
			Beep(750, 500);
			printf("Ctrl-Shutdown event\n\n");
			return FALSE;
*/
		default:
			return FALSE;
	}
}

void ShowVersion(BOOL bShowDetail)
{
	wprintf(L"Version 1.0.0 beta");
	if (bShowDetail) {
		wprintf(L" (build at %hs on %hs)", __TIME__, __DATE__);
	}
	wprintf(L"\n");
}

void ShowUsage(void)
{
	wprintf(L"Usage: scanSSIDList.exe [-ml 20 | -mt 5 | -td 5 | -ie | -bs F8B7975F6C9E,FC4AE92050F7] | -lg 1\n");
	wprintf(L"   -ml: Max loop count(default: 1)\n");
	wprintf(L"   -mt: Max time in minutes(default: 0)\n");
	wprintf(L"   -td: Time difference to next scan in seconds(default: 5)\n");
	wprintf(L"   -ie: Show information elements\n");
	wprintf(L"   -bs: Filter with BSSID(MAC address).\n");
	wprintf(L"        To filter with multiple BSSIDs, use a comma-separated list with no spaces.\n");
	wprintf(L"   -lg: Save the BSSID list logs in CSV format in every N minutes(default: Save only at the end)\n");
	wprintf(L"        -1 is a magic number to save CSV logs in each scan. Use with -bs option is recommended to avoid huge file.\n");
	wprintf(L"   -h:  Reshow this message\n\n");
	wprintf(L" The BSSID list will be saved at the end of this application,\n");
	wprintf(L" even when this would be forced to quit with Ctrl+C.\n");
}

int wmain(int argc, WCHAR* argv[])
{
	MSG msg;
	int timerCount = 1;
	int maxLoopCount = 1;
	int maxTimeInMinutes = 0;
	int timeDiff = 5;
	int saveCSVTime = 0;
	BOOL bMaxLoop = FALSE;
	BOOL bMaxTime = FALSE;
	UINT_PTR TID_LOOPTIMER = 1;
	UINT_PTR TID_CSVTIMER = 2;

	setlocale(LC_ALL, "Japanese");

	if (!SetConsoleCtrlHandler(CtrlHandler, TRUE)) {
		wprintf(L"SetConsoleCtrlHandler failure.\n");
	}
	BSSIDFilter.clear();

	SYSTEMTIME st;
	GetLocalTime(&st);
	swprintf_s(strSaveTextFilename, 128, L"WILlog_%04d%02d%02d_%02d%02d%02d.txt",
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	swprintf_s(strSaveCsvFilename, 128, L"WILlog_%04d%02d%02d_%02d%02d%02d.csv",
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	for (int i = 1; i < argc; i++) {
		if (_wcsnicmp(argv[i], L"-usage", MAX_PATH) == 0 ||
			_wcsnicmp(argv[i], L"-help", MAX_PATH) == 0 ||
			_wcsnicmp(argv[i], L"-h", MAX_PATH) == 0 ||
			_wcsnicmp(argv[i], L"--help", MAX_PATH) == 0) {
			ShowUsage();
			return 0;
		}
		else if (_wcsnicmp(argv[i], L"-v", MAX_PATH) == 0 ||
			_wcsnicmp(argv[i], L"-V", MAX_PATH) == 0 ||
			_wcsnicmp(argv[i], L"--v", MAX_PATH) == 0 ||
			_wcsnicmp(argv[i], L"--V", MAX_PATH) == 0 ||
			_wcsnicmp(argv[i], L"-version", MAX_PATH) == 0 ||
			_wcsnicmp(argv[i], L"version", MAX_PATH) == 0) {
			ShowVersion(FALSE);
			return 0;
		}
		else if (_wcsnicmp(argv[i], L"-v2", MAX_PATH) == 0) {
			ShowVersion(TRUE);
			return 0;
		}
		else if (_wcsnicmp(argv[i], L"-ml", MAX_PATH) == 0) {
			if ((i + 1) >= argc) {
				wprintf(L"Input parameter error\n");
				return -1;
			}
			int nn = getIntValue(argv[i + 1]);
			if (nn < 0) {
				wprintf(L"Input parameter error\n");
				return -1;
			}
			maxLoopCount = nn;
			bMaxLoop = TRUE;
			i++;
		}
		else if (_wcsnicmp(argv[i], L"-mt", MAX_PATH) == 0) {
			if ((i + 1) >= argc) {
				wprintf(L"Input parameter error\n");
				return -1;
			}
			int nn = getIntValue(argv[i + 1]);
			if (nn < 0) {
				wprintf(L"Input parameter error\n");
				return -1;
			}
			maxTimeInMinutes = nn;
			bMaxTime = TRUE;
			i++;
		}
		else if (_wcsnicmp(argv[i], L"-td", MAX_PATH) == 0) {
			if ((i + 1) >= argc) {
				wprintf(L"Input parameter error\n");
				return -1;
			}
			int nn = getIntValue(argv[i + 1]);
			if (nn < 0) { // debug
				wprintf(L"Input parameter error\n");
				return -1;
			}
			timeDiff = nn;
			i++;
		}
		else if (_wcsnicmp(argv[i], L"-bs", MAX_PATH) == 0) {
			if ((i + 1) >= argc) {
				wprintf(L"Input parameter error\n");
				return -1;
			}

			size_t size = wcslen(argv[i + 1]) + 1;
			char* pBuff = new char[size];
			size_t convertedChars = 0;
			wcstombs_s(&convertedChars, pBuff, size, argv[i + 1], _TRUNCATE);
			string s;
			stringstream ss = stringstream(pBuff);
			while (getline(ss, s, ',')) {
				s = regex_replace(s, regex(":"), "");
				s = regex_replace(s, regex("-"), "");
				BSSIDFilter.push_back(s);
			}
			delete[] pBuff;
			i++;
		}
		else if (_wcsnicmp(argv[i], L"-lg", MAX_PATH) == 0) {
			if ((i + 1) >= argc) {
				wprintf(L"Input parameter error\n");
				return -1;
			}
			int nn = getIntValue(argv[i + 1]);
			if ((nn < -1) || (nn == 0)) {
				wprintf(L"Input parameter error\n");
				return -1;
			}
			saveCSVTime = nn;
			i++;
		}
		else if (_wcsnicmp(argv[i], L"-ie", MAX_PATH) == 0) {
			bShowIE = TRUE;
		}
	}

	time_t start, finish;

	time(&start);

	HANDLE hClient = NULL;
	PWLAN_INTERFACE_INFO_LIST pInfoList = NULL;
	bssInfoMap.clear();

	WifiInfoLib_Open(&hClient, &pInfoList);

	WifiInfoLib_Scan(hClient, pInfoList);
	WifiInfoLib_GetWifiInfo(hClient, pInfoList, &bssInfoMap);
	//WifiInfoLib_GetBSSIDInfo(hClient, pInfoList, &bssInfoMap);
	WIL_Sample_Show(bssInfoMap, bShowIE, BSSIDFilter);

	if ((argc > 1) && ((maxLoopCount > 1) || (maxTimeInMinutes > 0))) {
		BOOL bToQuit = FALSE;
		UINT unTimer = (UINT)timeDiff * 1000;
		UINT unTimerCsv = (UINT)saveCSVTime * 60 * 1000;
		TID_LOOPTIMER = SetTimer(NULL, TID_LOOPTIMER, unTimer, NULL);
		TID_CSVTIMER = SetTimer(NULL, TID_CSVTIMER, unTimerCsv, NULL);
		if (saveCSVTime > 0) {
			TID_CSVTIMER = SetTimer(NULL, TID_CSVTIMER, unTimerCsv, NULL);
			WifiInfoLib_CsvWrite(strSaveCsvFilename, bssInfoMap, BSSIDFilter);
		}
		while (GetMessage(&msg, NULL, 0, 0)) {
			if (msg.message == WM_TIMER) {
				if (msg.wParam == TID_LOOPTIMER) {
					WifiInfoLib_Scan(hClient, pInfoList);
					WifiInfoLib_GetWifiInfo(hClient, pInfoList, &bssInfoMap);
					//WifiInfoLib_GetBSSIDInfo(hClient, pInfoList, &bssInfoMap);
					WIL_Sample_Show(bssInfoMap, bShowIE, BSSIDFilter);
					if (saveCSVTime == SAVE_CSV_IN_EACH_SCAN) {
						WifiInfoLib_CsvWrite(strSaveCsvFilename, bssInfoMap, BSSIDFilter);
					}

					timerCount++;
					if (bMaxLoop) {
						if (timerCount > (maxLoopCount - 1)) {
							KillTimer(NULL, TID_LOOPTIMER);
							bToQuit = TRUE;
							break;
						}
					}
					if (bMaxTime) {
						time(&finish);
						double elapsed_time = difftime(finish, start);
						elapsed_time = elapsed_time / 60.0;
						if (elapsed_time > maxTimeInMinutes) {
							KillTimer(NULL, TID_LOOPTIMER);
							bToQuit = TRUE;
							break;
						}
					}
				}
				else if (msg.wParam == TID_CSVTIMER) {
					if (saveCSVTime > 0) {
						WifiInfoLib_CsvWrite(strSaveCsvFilename, bssInfoMap, BSSIDFilter);
					}
				}
			}
		}
		if (!bToQuit) {
			return (int)msg.wParam;
		}
	}

	KillTimer(NULL, TID_CSVTIMER);
	WifiInfoLib_Write(strSaveTextFilename, bssInfoMap, BSSIDFilter);
	WifiInfoLib_CsvWrite(strSaveCsvFilename, bssInfoMap, BSSIDFilter);
	bssInfoMap.clear();
	WifiInfoLib_Close(hClient, pInfoList);
	SetConsoleCtrlHandler(CtrlHandler, FALSE);

	return 0;
}

// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します
