/**
 * @file swiQmi.h
 *
 * Helper functions for starting and accessing QMI services and some QMI utilities functions.  This
 * module should not be used outside the QMI modem services implementation.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#ifndef SWI_QMI_INCLUDE_GUARD
#define SWI_QMI_INCLUDE_GUARD

#include "legato.h"

#define SIERRA
#include "qmi_includes.h"

// todo: This macro should be moved somewhere else
// todo: It appears this macro may no longer be needed, since the compiler warnings/errors have
//       changed, but keep it for now.  May be moved or deleted later.
// Handle unused function parameters to prevent compiler warnings/errors.
#define LE_UNUSEDPARAM( x ) (void)x

//--------------------------------------------------------------------------------------------------
/**
 * Number of services supported
 */
//--------------------------------------------------------------------------------------------------
#define NB_SERVICES_MAX     16

//--------------------------------------------------------------------------------------------------
/**
 * Modem communication timeout in milliseconds.
 */
//--------------------------------------------------------------------------------------------------
#define COMM_DEFAULT_PLATFORM_TIMEOUT       3000
#define COMM_UICC_TIMEOUT                   10000
#define COMM_LONG_PLATFORM_TIMEOUT          10000
#define COMM_NETWORK_TIMEOUT                30000
#define COMM_PLATFORM_TIMEOUT_60_SECONDS    60000
#define COMM_NETWORK_SCAN_TIMEOUT           300000

// Voice call hang up timeout.
// Voice call releasing process between the network and the UE can take up to 90s in bad network
// condition. 10 sec of security is added.
// According to 3GPP spec TS24.008 section 5.4.3, after sending a "DISCONNECT" message, the UE
// starts a timer T305 and waits for "RELEASE" message from the network. Upon expiry of this timer,
// the UE sends a "RELEASE" message and starts timer T308. If "RELEASE COMPLETE" is not received,
// the UE retransmits the "RELEASE" message at first expiry of timer T308, and waits for a second
// expiry of timer T308. Timer T305 and T308 are 30s each.
#define COMM_HANGUP_VOICE_CALL_TIMEOUT      100000

//--------------------------------------------------------------------------------------------------
/**
 * QMI services version adaptation helpers
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * DMS: Incompatibility for some environment messages.
 *
 * - dms_swi_set_environment_req_msg_v01
 * - dms_swi_get_environment_resp_msg_v01
 */
//--------------------------------------------------------------------------------------------------
#if (DMS_V01_IDL_MAJOR_VERS >= 0x01) && (DMS_V01_IDL_MINOR_VERS >= 0x23)

# define DMS_SWI_ENVIRONMENT(gETsET, mEMBER) \
    gETsET ## _ ## mEMBER

#else

# define DMS_SWI_ENVIRONMENT(gETsET, mEMBER) \
    mEMBER

#endif

//--------------------------------------------------------------------------------------------------
/**
 * Macro to declare (and initialize) a QMI message.
 */
//--------------------------------------------------------------------------------------------------
#define SWIQMI_DECLARE_MESSAGE(tYPE, mSG)  \
    tYPE mSG;                           \
    memset(&(mSG), 0, sizeof(tYPE))

//--------------------------------------------------------------------------------------------------
/**
 * The types of QMI services available.
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    QMI_SERVICE_NAS = 0,    ///< Network Access Service
    QMI_SERVICE_WMS,        ///< Wireless Messaging Service
    QMI_SERVICE_DMS,        ///< Device Management Service
    QMI_SERVICE_WDS,        ///< Wireless Data Service
    QMI_SERVICE_VOICE,      ///< Voice Service
    QMI_SERVICE_LOC,        ///< Location Service
    QMI_SERVICE_MGS,        ///< M2M General Service
    QMI_SERVICE_REALTIME,   ///< Realtime Service
    QMI_SERVICE_UIM,        ///< User Identity Module
    QMI_SERVICE_LWM2M,      ///< LWM2M Service
    QMI_SERVICE_CAT,        ///< Card Application Toolkit
    QMI_SERVICE_SFS,        ///< Secure File System Service
    QMI_SERVICE_AUDIO,      ///< M2M Audio Service
    QMI_SERVICE_SWI_BSP,    ///< BSP services
    QMI_SERVICE_UIMRMT,     ///< User Identity Module Remote services
    QMI_SERVICE_SWI_LOC,    ///< SWI location service
    QMI_SERVICE_PBM,        ///< Phonebook Manager
    QMI_SERVICE_SWI_SAR,    ///< SWI SAR service
    QMI_SERVICE_COUNT       ///< This must always be the last element in this enum.
}
swiQmi_Service_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure used to save the QMI service initialization
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    qmi_idl_service_object_type wdsServiceObj;  ///< WDS service object
    qmi_service_info info[NB_SERVICES_MAX];     ///< WDS service information
    int infoIndex;                              ///< index in info area to use
    unsigned int numServices;                   ///< Service number
    uint32_t usedInfoIndex;                     ///< index in info area in used
}   swiQmi_Param_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure used to save the QMI client info
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    qmi_client_type                 serviceHandle; ///< Client handle with QMI service
    qmi_client_type                 notifier;      ///< Notifier handle with QMI service
    qmi_cci_os_signal_type          osParams;      ///< OS-specific parameters
}
swiQmi_ClientInfo_t;

//--------------------------------------------------------------------------------------------------
/**
 * Prototype for QMI indication handler functions.  All such functions must look like this.
 */
//--------------------------------------------------------------------------------------------------
typedef void (*swiQmi_IndicationHandler_t)
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to the handler
);


//--------------------------------------------------------------------------------------------------
/**
 * Initializes all required QMI services.

 * @return
 *      - LE_OK on success
 *      - LE_FAULT on error
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t swiQmi_InitServices
(
    swiQmi_Service_t serviceType    ///< [IN] The service type to initialize
);


//--------------------------------------------------------------------------------------------------
/**
 * Gets the client handle for a specific QMI service.  The client handle can then be used later to
 * send/receive qmi messages for the specific service.
 *
 * @return
 *      - The client service if successful.
 *      - NULL if not successful.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED qmi_client_type swiQmi_GetClientHandle
(
    swiQmi_Service_t serviceType  ///< [IN] The service type to get the client handle for
);


//--------------------------------------------------------------------------------------------------
/**
 * Add an indication handler to be called when the specified QMI service has an indication with the
 * matching message ID.  The desired indications must be setup for the QMI service before any
 * handlers will be called.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on errors
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t swiQmi_AddIndicationHandler
(
    swiQmi_IndicationHandler_t handler,         ///< Indication handler.
    swiQmi_Service_t serviceType,               ///< The service to register the handler with.
    unsigned int msgId,                         ///< The message ID of the indication to receive.
    void* contextPtr                            ///< Value to pass to handler when it is called.
);


//--------------------------------------------------------------------------------------------------
/**
 * Remove an indication handler.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED void swiQmi_RemoveIndicationHandler
(
    swiQmi_IndicationHandler_t handler,         ///< Indication handler.
    swiQmi_Service_t serviceType,               ///< The service.
    unsigned int msgId                          ///< The message ID.
);


//--------------------------------------------------------------------------------------------------
/**
 * Initializes a QMI service.

 * @return
 *      - LE_OK on success or if the services were already initialized.
 *      - LE_FAULT on error
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t swiQmi_InitQmiService
(
    swiQmi_Service_t serviceType,                   ///< [IN] The service type to initialize
    swiQmi_ClientInfo_t* clientInfoPtr,             ///< [OUT] Client service instance info
    swiQmi_Param_t   *paramPtr                      ///< [OUT] QMI service parameters
);

//--------------------------------------------------------------------------------------------------
/**
 * Release a QMI service.
 *
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED void swiQmi_ReleaseService
(
    qmi_client_type handle
);


//--------------------------------------------------------------------------------------------------
/**
 * Check the QMI response for any errors
 *
 * This provides common handling for checking the response codes of qmi_client_send_msg_sync() and
 * the associated QMI response message.
 *
 * @note This function replaces both the deprecated functions swiQmi_CheckResponseCode() and
 * swiQmi_OEMCheckResponseCode().
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on error
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t swiQmi_CheckResponse
(
    const char* msgIdStr,           ///< [IN] The string for the QMI message
    qmi_client_error_type rc,       ///< [IN] Response from the send function
    int32_t result,                 ///< [IN] Result from QMI response message
    int32_t error                   ///< [IN] Error from QMI response message
);


//--------------------------------------------------------------------------------------------------
/**
 * Check the QMI response codes for any errors
 *
 * This provides common handling for checking the response codes of qmi_client_send_msg_sync() and
 * the associated QMI response message.
 *
 * @warning This function is deprecated, and swiQmi_CheckResponse() should be used for new code.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on error
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t swiQmi_CheckResponseCode
(
    const char* msgIdStr,           ///< [IN] The string for the QMI message
    qmi_client_error_type rc,       ///< [IN] Response from the send function
    qmi_response_type_v01 resp      ///< [IN] Response value in the QMI response message
);


//--------------------------------------------------------------------------------------------------
/**
 * Check the QMI OEM response codes for any errors
 *
 * This provides common handling for checking the response codes of qmi_client_send_msg_sync() and
 * the associated QMI response message.
 *
 * @warning This function is deprecated, and swiQmi_CheckResponse() should be used for new code.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on error
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t swiQmi_OEMCheckResponseCode
(
    const char* msgIdStr,           ///< [IN] The string for the QMI message
    qmi_client_error_type rc,       ///< [IN] Response from the send function
#if (QAPI_SWI_COMMON_V01_IDL_MINOR_VERS <= 1)
    oem_response_type_v01 resp      ///< [IN] Response value in the QMI response message
#else
    qmi_response_type_v01 resp      ///< [IN] Response value in the QMI response message
#endif
);

#endif // SWI_QMI_INCLUDE_GUARD
