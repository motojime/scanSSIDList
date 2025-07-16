// WifiInfoLib.h : 
//

#pragma once

#include <unordered_map>
#include <vector>
#include <string>
using namespace std;

#define MALLOC(x) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

#define WIL_SUCCESS							0x0000
#define WIL_OPEN_HANDLE_ERROR				0x0001
#define WIL_CLOSE_HANDLE_ERROR				0x0002
#define WIL_ENUM_INTERFACES_ERROR			0x0003
#define WIL_REGISTER_NOTIFICATION_ERROR		0x0004
#define WIL_SCAN_ERROR						0x0005
#define WIL_SCAN_TIMEOUT					0x0006
#define WIL_GET_BSSLIST_ERROR				0x0007
#define WIL_PARAMETER_ERROR					0x0008
#define WIL_NO_WIFI_INTERFACE_EXIST			0x0009
#define WIL_INSUFFICIENT_BUFFER				0x000a
#define WIL_FILE_OPEN_ERROR					0x000b
#define WIL_QUERY_INTERFACE_ERROR			0x000c
#define WIL_LOCAL_ERROR						0xffff

#define WIL_RADIO_POWER_ON					0x01
#define WIL_RADIO_POWER_OFF					0x02

#define WIL_WRITE_BUFFER_SIZE               2048

typedef struct _WLAN_CALLBACK_CONTEXT {
	GUID interfaceGUID;
	HANDLE scanEvent;
} WLAN_CALLBACK_CONTEXT;

#pragma pack(1)
// BSS Load (see 9.4.2.28)
// Element 11
typedef struct _WLAN_BSS_LOAD_ELEMENT {
	unsigned short stationCount;
	unsigned char channelUtilization;
	unsigned short availableAdmissionCapacity;
} WLAN_BSS_LOAD_ELEMENT, *PWLAN_BSS_LOAD_ELEMENT;

// HT Capabilities (see 9.4.2.56)
// Element 45
typedef struct _WLAN_HT_CAPABILITIES_ELEMENT {
	unsigned short HTCapabilityInformation;
	unsigned char AMPDUParameters;
	unsigned long long supportedMCSSet1;
	unsigned long long supportedMCSSet2;
	unsigned short HTExtendedCapabilities;
	unsigned long transmitBeamformingCapabilities;
	unsigned char ASELCapabilities;
} WLAN_HT_CAPABILITIES_ELEMENT, *PWLAN_HT_CAPABILITIES_ELEMENT;

// HT Operation (see 9.4.2.57)
// Element 61
typedef struct _WLAN_HT_OPERATION_ELEMENT {
	unsigned char primaryChannel;
	unsigned char HTOperationInformation1;
	unsigned short HTOperationInformation2;
	unsigned short HTOperationInformation3;
	unsigned long long basicHTMCSSet1;
	unsigned long long basicHTMCSSet2;
} WLAN_HT_OPERATION_ELEMENT, *PWLAN_HT_OPERATION_ELEMENT;
#pragma pack()

//WifiInfoLib
#define ELEMENT_DATA_MAX_SIZE (2324)

typedef struct _WIL_BSS_INFO {
	SYSTEMTIME lastUpdate;

	WLAN_AVAILABLE_NETWORK wlanAvailableNetwork;
	WLAN_BSS_ENTRY wlanBssEntry;

	unsigned short stationCount;
	unsigned char channelUtilization;
	unsigned char HTOperationChannel;
	unsigned char HTOperationChannelWidth;
	unsigned char VHTOperationChannelWidth;
	unsigned short HTCapabilitiesRxHighestSupportedDataRate;

	ULONG elementDataBlobSize;
	unsigned char elementDataBlob[ELEMENT_DATA_MAX_SIZE];
} WIL_BSS_INFO, *PWIL_BSS_INFO;

typedef unordered_map<ULONG64, WIL_BSS_INFO> WILBssInfoMap;
typedef WILBssInfoMap* PWILBssInfoMap;

typedef unordered_map<DWORD, wstring> WILResultWStringMap;

static void WlanNotification(WLAN_NOTIFICATION_DATA *wlanNotifData, VOID *p);
int getChannelFromFrequency(ULONG ulChFreq);

DWORD WifiInfoLib_ShowResult(DWORD dwResult);
DWORD WifiInfoLib_GetResultString(DWORD dwResult, LPWSTR lpszString, DWORD InBufSize);
DWORD WifiInfoLib_GetLastError();

DWORD WifiInfoLib_Open(PHANDLE phClientHandle, PWLAN_INTERFACE_INFO_LIST *ppIfList);
DWORD WifiInfoLib_Close(HANDLE hClientHandle, PWLAN_INTERFACE_INFO_LIST pIfList);
DWORD WifiInfoLib_Scan(HANDLE hClient, PWLAN_INTERFACE_INFO_LIST pIfList);
DWORD WifiInfoLib_QueryRadioPower(PINT pValue, const HANDLE &hClient, const PWLAN_INTERFACE_INFO &pIfInfo);
DWORD WifiInfoLib_Write(LPCWSTR lpFilename, WILBssInfoMap bssInfoMap, const std::vector<string> filter);
DWORD WifiInfoLib_CsvWrite(LPCWSTR lpFilename, WILBssInfoMap bssInfoMap, const std::vector<string> filter);

DWORD WifiInfoLib_GetWifiInfo(HANDLE hClient, PWLAN_INTERFACE_INFO_LIST pIfList, PWILBssInfoMap pBssInfoMap);

DWORD WifiInfoLib_GetSSIDString(wchar_t* buff, size_t bufLen, PWIL_BSS_INFO pWilBssInfo);
DWORD WifiInfoLib_GetBSSIDString(wchar_t* buff, size_t bufLen, PWIL_BSS_INFO pWilBssInfo);
DWORD WifiInfoLib_GetRSSI(PLONG pValue, PWIL_BSS_INFO pWilBssInfo);
DWORD WifiInfoLib_GetLinkQuality(PLONG pValue, PWIL_BSS_INFO pWilBssInfo);
DWORD WifiInfoLib_GetSignalQuality(PLONG pValue, PFLOAT pRssi, PWIL_BSS_INFO pWilBssInfo);
DWORD WifiInfoLib_GetMaxSupportedRate(PLONG pValue, PWIL_BSS_INFO pWilBssInfo);
DWORD WifiInfoLib_GetChannel(PINT pCh, PFLOAT pFreq, PWIL_BSS_INFO pWilBssInfo);
DWORD WifiInfoLib_GetNumberOfBssids(PLONG pValue, PWIL_BSS_INFO pWilBssInfo);
DWORD WifiInfoLib_GetBeaconPeriod(PSHORT pValue, PWIL_BSS_INFO pWilBssInfo);

DWORD WifiInfoLib_GetCipherString(wchar_t* buff, size_t bufLen, PWIL_BSS_INFO pWilBssInfo);
DWORD WifiInfoLib_GetAuthAlgorithmString(wchar_t* buff, size_t bufLen, PWIL_BSS_INFO pWilBssInfo);
DWORD WifiInfoLib_GetBSSTypeString(wchar_t* buff, size_t bufLen, PWIL_BSS_INFO pWilBssInfo);

void Element11(unsigned char *pBuff, PWIL_BSS_INFO pWILBssInfo);
void Element45(unsigned char *pBuff, PWIL_BSS_INFO pWILBssInfo);
void Element61(unsigned char *pBuff, PWIL_BSS_INFO pWILBssInfo);
void Element192(unsigned char *pBuff, PWIL_BSS_INFO pWILBssInfo);

void createResultStringMap(void);
DWORD getSSIDString(char* buff, int bufLen, PWIL_BSS_INFO pWilBssInfo);
DWORD getSSIDSJISString(char* buff, int bufLen, PWIL_BSS_INFO pWilBssInfo);
DWORD getBSSIDString(char* buff, int bufLen, PWIL_BSS_INFO pWilBssInfo);
DWORD getCipherString(char* buff, int bufLen, PWIL_BSS_INFO pWilBssInfo);
DWORD getAuthAlgorithmString(char* buff, int bufLen, PWIL_BSS_INFO pWilBssInfo);
DWORD getBSSTypeString(char* buff, int bufLen, PWIL_BSS_INFO pWilBssInfo);

// deprecated
DWORD WifiInfoLib_GetBSSIDInfo(HANDLE hClient, PWLAN_INTERFACE_INFO_LIST pIfList, PWILBssInfoMap pWILBssInfoMap);
