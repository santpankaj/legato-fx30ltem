

/*
 * ====================== WARNING ======================
 *
 * THE CONTENTS OF THIS FILE HAVE BEEN AUTO-GENERATED.
 * DO NOT MODIFY IN ANY WAY.
 *
 * ====================== WARNING ======================
 */


#ifndef LE_WIFICLIENT_INTERFACE_H_INCLUDE_GUARD
#define LE_WIFICLIENT_INTERFACE_H_INCLUDE_GUARD


#include "legato.h"

// Interface specific includes
#include "le_wifiDefs_server.h"


//--------------------------------------------------------------------------------------------------
/**
 * Get the server service reference
 */
//--------------------------------------------------------------------------------------------------
le_msg_ServiceRef_t le_wifiClient_GetServiceRef
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the client session reference for the current message
 */
//--------------------------------------------------------------------------------------------------
le_msg_SessionRef_t le_wifiClient_GetClientSessionRef
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Initialize the server and advertise the service.
 */
//--------------------------------------------------------------------------------------------------
void le_wifiClient_AdvertiseService
(
    void
);


//--------------------------------------------------------------------------------------------------
/**
 * Reference type for AccessPoint that is returned by the WiFi Scan.
 */
//--------------------------------------------------------------------------------------------------
typedef struct le_wifiClient_AccessPoint* le_wifiClient_AccessPointRef_t;


//--------------------------------------------------------------------------------------------------
/**
 * WiFi Client Events.
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LE_WIFICLIENT_EVENT_CONNECTED = 0,
        ///< WiFi Client Connected
    LE_WIFICLIENT_EVENT_DISCONNECTED = 1,
        ///< WiFi Client Disconnected
    LE_WIFICLIENT_EVENT_SCAN_DONE = 2,
        ///< WiFi Scan result for available Access Points available
    LE_WIFICLIENT_EVENT_SCAN_FAILED = 3
        ///< WiFi Scan failed
}
le_wifiClient_Event_t;


//--------------------------------------------------------------------------------------------------
/**
 * WiFi Client Security Protocol for connection
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LE_WIFICLIENT_SECURITY_NONE = 0,
        ///< no security.
    LE_WIFICLIENT_SECURITY_WEP = 1,
        ///< Using WEP.
    LE_WIFICLIENT_SECURITY_WPA_PSK_PERSONAL = 2,
        ///< Using WPA
    LE_WIFICLIENT_SECURITY_WPA2_PSK_PERSONAL = 3,
        ///< Using WPA2
    LE_WIFICLIENT_SECURITY_WPA_EAP_PEAP0_ENTERPRISE = 4,
        ///< Using WPA Enterprise
    LE_WIFICLIENT_SECURITY_WPA2_EAP_PEAP0_ENTERPRISE = 5
        ///< Using WPA2 Enterprise
}
le_wifiClient_SecurityProtocol_t;


//--------------------------------------------------------------------------------------------------
/**
 * Reference type used by Add/Remove functions for EVENT 'le_wifiClient_NewEvent'
 */
//--------------------------------------------------------------------------------------------------
typedef struct le_wifiClient_NewEventHandler* le_wifiClient_NewEventHandlerRef_t;


//--------------------------------------------------------------------------------------------------
/**
 * Handler for WiFi Client changes
 */
//--------------------------------------------------------------------------------------------------
typedef void (*le_wifiClient_NewEventHandlerFunc_t)
(
    le_wifiClient_Event_t event,
        ///< Handles the wifi events
    void* contextPtr
        ///<
);


//--------------------------------------------------------------------------------------------------
/**
 * Add handler function for EVENT 'le_wifiClient_NewEvent'
 *
 * This event provide information on WiFi Client event changes.
 *
 */
//--------------------------------------------------------------------------------------------------
le_wifiClient_NewEventHandlerRef_t le_wifiClient_AddNewEventHandler
(
    le_wifiClient_NewEventHandlerFunc_t handlerPtr,
        ///< [IN]
    void* contextPtr
        ///< [IN]
);



//--------------------------------------------------------------------------------------------------
/**
 * Remove handler function for EVENT 'le_wifiClient_NewEvent'
 */
//--------------------------------------------------------------------------------------------------
void le_wifiClient_RemoveNewEventHandler
(
    le_wifiClient_NewEventHandlerRef_t handlerRef
        ///< [IN]
);



//--------------------------------------------------------------------------------------------------
/**
 * This function starts the WIFI device.
 *
 * @return
 *      - LE_FAULT if the function failed.
 *      - LE_BUSY if the WIFI device is already started.
 *      - LE_OK if the function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_wifiClient_Start
(
    void
);



//--------------------------------------------------------------------------------------------------
/**
 * This function stops the WIFI device.
 *
 * @return
 *      - LE_FAULT if the function failed.
 *      - LE_DUPLICATE if the WIFI device is already stopped.
 *      - LE_OK if the function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_wifiClient_Stop
(
    void
);



//--------------------------------------------------------------------------------------------------
/**
 * Start Scanning for WiFi Access points
 * Will result in event LE_WIFICLIENT_EVENT_SCAN_DONE when the scan results are available.
 *
 * @return
 *      - LE_FAULT if the function failed.
 *      - LE_OK if the function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_wifiClient_Scan
(
    void
);



//--------------------------------------------------------------------------------------------------
/**
 * Get the first WiFi Access Point found.
 * Will return the Access Points in the order of found.
 *
 * @return
 *      - WiFi Access Point reference if ok.
 *      - NULL if no Access Point reference available.
 */
//--------------------------------------------------------------------------------------------------
le_wifiClient_AccessPointRef_t le_wifiClient_GetFirstAccessPoint
(
    void
);



//--------------------------------------------------------------------------------------------------
/**
 * Get the next WiFi Access Point.
 * Will return the Access Points in the order of found.
 * This function must be called in the same context as the GetFirstAccessPoint
 *
 * @return
 *      - WiFi Access Point reference if ok.
 *      - NULL if no Access Point reference available.
 */
//--------------------------------------------------------------------------------------------------
le_wifiClient_AccessPointRef_t le_wifiClient_GetNextAccessPoint
(
    void
);



//--------------------------------------------------------------------------------------------------
/**
 * Get the signal strength of the AccessPoint
 *
 * @return
 *      - signal strength in dBm. Example -30 = -30dBm
 *      - if no signal available it will return OxFFFF
 */
//--------------------------------------------------------------------------------------------------
int16_t le_wifiClient_GetSignalStrength
(
    le_wifiClient_AccessPointRef_t accessPointRef
        ///< [IN] WiFi Access Point reference.
);



//--------------------------------------------------------------------------------------------------
/**
 * Get the Basic Service set identifier (BSSID) of the AccessPoint
 *
 * @return LE_FAULT         Function failed.
 * @return LE_BAD_PARAMETER Some parameter is invalid.
 * @return LE_OVERFLOW      bssid buffer is too small to contain the BSSID
 * @return LE_OK            Function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_wifiClient_GetBssid
(
    le_wifiClient_AccessPointRef_t accessPointRef,
        ///< [IN] WiFi Access Point reference.
    char* bssid,
        ///< [OUT] The BSSID
    size_t bssidSize
        ///< [IN]
);



//--------------------------------------------------------------------------------------------------
/**
 * Get the Service set identification (SSID) of the AccessPoint
 * @note that the SSID does not have to be human readable ASCII values, but often has.
 *
 * @return LE_FAULT         Function failed.
 * @return LE_BAD_PARAMETER Some parameter is invalid.
 * @return LE_OVERFLOW      ssid buffer is too small to contain the SSID
 * @return LE_OK            Function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_wifiClient_GetSsid
(
    le_wifiClient_AccessPointRef_t accessPointRef,
        ///< [IN] WiFi Access Point reference.
    uint8_t* ssidPtr,
        ///< [OUT] The SSID returned as a octet array.
    size_t* ssidSizePtr
        ///< [INOUT]
);



//--------------------------------------------------------------------------------------------------
/**
 * Set the passphrase used to generate the PSK.
 *
 * @note the difference between le_wifiClient_SetPreSharedKey() and this function
 *
 * @return
 *      - LE_FAULT if the function failed.
 *      - LE_BAD_PARAMETER if parameter is invalid.
 *      - LE_OK if the function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_wifiClient_SetPassphrase
(
    le_wifiClient_AccessPointRef_t accessPointRef,
        ///< [IN] WiFi Access Point reference.
    const char* LE_NONNULL PassPhrase
        ///< [IN] pass-phrase for PSK
);



//--------------------------------------------------------------------------------------------------
/**
 * Set the Pre Shared Key, PSK.
 * @note the difference between le_wifiClient_SetPassphrase() and this function
 *
 * @return
 *      - LE_FAULT if the function failed.
 *      - LE_BAD_PARAMETER if parameter is invalid.
 *      - LE_OK if the function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_wifiClient_SetPreSharedKey
(
    le_wifiClient_AccessPointRef_t accessPointRef,
        ///< [IN] WiFi Access Point reference.
    const char* LE_NONNULL PreSharedKey
        ///< [IN] PSK. Note the difference between PSK and Pass Phrase.
);



//--------------------------------------------------------------------------------------------------
/**
 * Set the security protocol for connection
 *
 * @return
 *      - LE_FAULT if the function failed.
 *      - LE_BAD_PARAMETER if parameter is invalid.
 *      - LE_OK if the function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_wifiClient_SetSecurityProtocol
(
    le_wifiClient_AccessPointRef_t accessPointRef,
        ///< [IN] WiFi Access Point reference.
    le_wifiClient_SecurityProtocol_t securityProtocol
        ///< [IN] Security Mode
);



//--------------------------------------------------------------------------------------------------
/**
 * WPA-Enterprise requires a username and password to authenticate.
 * This function sets these parameters.
 *
 * @return
 *      - LE_FAULT if the function failed.
 *      - LE_BAD_PARAMETER if parameter is invalid.
 *      - LE_OK if the function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_wifiClient_SetUserCredentials
(
    le_wifiClient_AccessPointRef_t accessPointRef,
        ///< [IN] WiFi Access Point reference.
    const char* LE_NONNULL userName,
        ///< [IN] UserName used for WPA-Enterprise.
    const char* LE_NONNULL password
        ///< [IN] Password used for WPA-Enterprise.
);



//--------------------------------------------------------------------------------------------------
/**
 * Set the WEP key (WEP)
 *
 * @return
 *      - LE_FAULT if the function failed.
 *      - LE_OK if the function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_wifiClient_SetWepKey
(
    le_wifiClient_AccessPointRef_t accessPointRef,
        ///< [IN] WiFi Access Point reference.
    const char* LE_NONNULL wepKey
        ///< [IN] The WEP key
);



//--------------------------------------------------------------------------------------------------
/**
 * This function specifies whether the target Access Point is hiding its presence from clients or
 * not. When an Access Point is hidden, it cannot be discovered by a scan process.
 *
 * @note By default, this attribute is not set which means that the client is unable to connect to
 * a hidden access point. When enabled, the client will be able to connect to the access point
 * whether it is hidden or not.
 *
 * @return
 *      - LE_BAD_PARAMETER if parameter is invalid.
 *      - LE_OK if the function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_wifiClient_SetHiddenNetworkAttribute
(
    le_wifiClient_AccessPointRef_t accessPointRef,
        ///< [IN] WiFi Access Point reference.
    bool hidden
        ///< [IN] If TRUE, the WIFI client will be able to connect to a hidden access point.
);



//--------------------------------------------------------------------------------------------------
/**
 * This function creates a reference to an Access Point given its SSID.
 * If an Access Point is hidden, it will not show up in the scan. So, its SSID must be known
 * in advance in order to create a reference.
 *
 * @note This function fails if called while scan is running.
 *
 * @return AccessPoint reference to the current
 */
//--------------------------------------------------------------------------------------------------
le_wifiClient_AccessPointRef_t le_wifiClient_Create
(
    const uint8_t* SsidPtr,
        ///< [IN] The SSID as a octet array.
    size_t SsidSize
        ///< [IN]
);



//--------------------------------------------------------------------------------------------------
/**
 * Deletes an accessPointRef.
 *
 * @note The handle becomes invalid after it has been deleted.
 * @return
 *      - LE_BAD_PARAMETER if accessPointRef was not found.
 *      - LE_BUSY if the function was called during a scan.
 *      - LE_OK if the function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_wifiClient_Delete
(
    le_wifiClient_AccessPointRef_t accessPointRef
        ///< [IN] WiFi Access Point reference.
);



//--------------------------------------------------------------------------------------------------
/**
 * Connect to the WiFi Access Point.
 * All authentication must be set prior to calling this function.
 *
 * @return
 *      - LE_BAD_PARAMETER if parameter is invalid.
 *      - LE_OK if the function succeeded.
 *
 * @note For PSK credentials see le_wifiClient_SetPassphrase() or le_wifiClient_SetPreSharedKey() .
 * @note For WPA-Enterprise credentials see le_wifiClient_SetUserCredentials()
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_wifiClient_Connect
(
    le_wifiClient_AccessPointRef_t accessPointRef
        ///< [IN] WiFi Access Point reference.
);



//--------------------------------------------------------------------------------------------------
/**
 * Disconnect from the current connected WiFi Access Point.
 *
 * @return
 *      - LE_FAULT if the function failed.
 *      - LE_OK if the function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_wifiClient_Disconnect
(
    void
);


#endif // LE_WIFICLIENT_INTERFACE_H_INCLUDE_GUARD