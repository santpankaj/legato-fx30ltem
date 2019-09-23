/**
 * @file swiQmi.c
 *
 * Helper functions for starting and accessing QMI services and some QMI utilities functions.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "swiQmi.h"

// Include macros for printing out values
#include "le_print.h"

//--------------------------------------------------------------------------------------------------
/**
 * Flag to indicate if the QMI services has been initialized.
 */
//--------------------------------------------------------------------------------------------------
static bool Initialized[QMI_SERVICE_COUNT] = { false };


//--------------------------------------------------------------------------------------------------
/**
 * Service client information.
 */
//--------------------------------------------------------------------------------------------------
static swiQmi_ClientInfo_t QmiClientInfo[QMI_SERVICE_COUNT];

//--------------------------------------------------------------------------------------------------
/**
 * The maximum number of indication handlers per service.
 */
//--------------------------------------------------------------------------------------------------
#define MAX_NUM_HANDLERS_PER_SERVICE            17

//--------------------------------------------------------------------------------------------------
/**
 *
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    swiQmi_IndicationHandler_t      handler;    // The handler to call.
    unsigned int                    msgId;      // The message Id for this handler.
    void*                           contextPtr; // Context value that was passed to the handler
}
MsgHandler_t;


//--------------------------------------------------------------------------------------------------
/**
 * Array of user indication handlers.  For each service users may register handlers that are called
 * when a matching QMI indication is received.
 */
//--------------------------------------------------------------------------------------------------
static MsgHandler_t MsgHandlers[QMI_SERVICE_COUNT][MAX_NUM_HANDLERS_PER_SERVICE] = { { {NULL, 0} } };


//--------------------------------------------------------------------------------------------------
/**
 * Max number of retries for checking if service is up
 */
//--------------------------------------------------------------------------------------------------
#define MAX_RETRIES 40

//--------------------------------------------------------------------------------------------------
/**
 * Milliseconds to wait for a service to start
 */
//--------------------------------------------------------------------------------------------------
#define SERVICE_START_DELAY 500

//--------------------------------------------------------------------------------------------------
/**
 * Start a QMI service
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on error
 */
//--------------------------------------------------------------------------------------------------
static le_result_t StartService(
    qmi_idl_service_object_type serviceObject,     ///< [IN]  Service object for the desired service
    qmi_client_ind_cb indicationHandler,           ///< [IN]  Handler function for indications
    swiQmi_ClientInfo_t* clientInfoPtr                ///< [OUT] Client service instance info
)
{
    int i;
    qmi_client_error_type rc;
    unsigned int numEntries = 1;
    unsigned int numServices;

    // todo: since numEntries is 1, does this need to be any larger than 1 ?
    qmi_service_info serviceInfo[10];


    rc = qmi_client_notifier_init(serviceObject, &(clientInfoPtr->osParams), &(clientInfoPtr->notifier));
    if (rc != QMI_NO_ERR)
    {
        LE_ERROR("client notifier init failed, rc=%d", rc);
        return LE_FAULT;
    }

    /* Check if the service is up, if not wait on a signal */
    // todo: this code can probably be cleaned up, i.e. why not just wait for the
    //       signal right away, instead of querying in a loop.
    for (i = 0; i < MAX_RETRIES; i++)
    {
        QMI_CCI_OS_SIGNAL_CLEAR(&(clientInfoPtr->osParams));

        rc = qmi_client_get_service_list(serviceObject, NULL, NULL, &numServices);
        LE_INFO("qmi_client_get_service_list rc=%d, numServices=%d", rc ,numServices);
        if (rc == QMI_NO_ERR)
            break;

        // todo: doesn't osParams need to be initialized, or the result checked ?
        QMI_CCI_OS_SIGNAL_WAIT(&(clientInfoPtr->osParams), SERVICE_START_DELAY);
    }

    if (rc != QMI_NO_ERR)
    {
        LE_ERROR("qmi_client_get_service_list time limit exceeded");
        qmi_client_release(clientInfoPtr->notifier);
        return LE_FAULT;
    }

    // Notifier client is no longer used, release it
    LE_DEBUG("Release notifier");
    qmi_client_release(clientInfoPtr->notifier);

    /* The server has come up, store the information in serviceInfo variable */
    rc = qmi_client_get_service_list(serviceObject, serviceInfo, &numEntries, &numServices);
    if (rc != QMI_NO_ERR)
    {
        LE_ERROR("qmi_client_get_service_list rc=%d numServices=%d", rc, numServices);
        return LE_FAULT;
    }

    rc = qmi_client_init(&serviceInfo[0], serviceObject,
                            indicationHandler, NULL, NULL, &(clientInfoPtr->serviceHandle));
    if (rc != QMI_NO_ERR)
    {
        uint32_t serviceId;
        if (QMI_NO_ERR == qmi_idl_get_service_id(serviceObject,&serviceId))
        {
            LE_ERROR("qmi_client_init service %X failed rc=%d",serviceId, rc);
        }
        else
        {
            LE_ERROR("qmi_client_init failed rc=%d", rc);
        }
        return LE_FAULT;
    }

    // service was successfully started
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Convert a QMI service to a string.
 *
 * @return a string describing the QMI service
 */
//--------------------------------------------------------------------------------------------------
static const char* QmiServiceTypeToString
(
    swiQmi_Service_t serviceType ///< Service Type to convert
)
{
    switch(serviceType)
    {
        case QMI_SERVICE_NAS:       return "QMI_SERVICE_NAS";
        case QMI_SERVICE_WMS:       return "QMI_SERVICE_WMS";
        case QMI_SERVICE_DMS:       return "QMI_SERVICE_DMS";
        case QMI_SERVICE_WDS:       return "QMI_SERVICE_WDS";
        case QMI_SERVICE_VOICE:     return "QMI_SERVICE_VOICE";
        case QMI_SERVICE_AUDIO:     return "QMI_SERVICE_AUDIO";
        case QMI_SERVICE_LOC:       return "QMI_SERVICE_LOC";
        case QMI_SERVICE_MGS:       return "QMI_SERVICE_MGS";
        case QMI_SERVICE_REALTIME:  return "QMI_SERVICE_REALTIME";
        case QMI_SERVICE_UIM:       return "QMI_SERVICE_UIM";
        case QMI_SERVICE_LWM2M:     return "QMI_SERVICE_LWM2M";
        case QMI_SERVICE_CAT:       return "QMI_SERVICE_CAT";
        case QMI_SERVICE_SFS:       return "QMI_SERVICE_SFS";
        case QMI_SERVICE_SWI_BSP:   return "QMI_SERVICE_SWI_BSP";
        case QMI_SERVICE_UIMRMT:    return "QMI_SERVICE_UIMRMT";
        case QMI_SERVICE_SWI_LOC:   return "QMI_SERVICE_SWI_LOC";
        case QMI_SERVICE_SWI_SAR:   return "QMI_SERVICE_SWI_SAR";
        case QMI_SERVICE_PBM:       return "QMI_SERVICE_PBM";
        case QMI_SERVICE_COUNT:     break;
    }

    LE_FATAL("Unable to convert QMI service type %d", serviceType);
    return "";
}

//--------------------------------------------------------------------------------------------------
// todo: doesn't seem to work correctly, i.e. it hangs; not even sure if it is needed, but keep
//       it here for now.  Needs more investigation.
//--------------------------------------------------------------------------------------------------
#if 0
static le_result_t StopService
(
    qmi_client_type client,                    ///< [IN]
    qmi_client_type notifier                   ///< [IN]
)
{
    qmi_client_release(client);
    qmi_client_release(notifier);
    return LE_OK;
}
#endif

//--------------------------------------------------------------------------------------------------
/**
 * Handler for all indications.
 */
//--------------------------------------------------------------------------------------------------
static void WdsIndicationsHandler
(
    qmi_client_type                clientHandle,
    unsigned int                   msg_id,
    void                           *ind_buf,
    unsigned int                   ind_buf_len,
    void                           *ind_cb_data
)
{
    LE_UNUSEDPARAM(ind_cb_data);

    // Determine the service for this indication.
    swiQmi_Service_t serviceType = QMI_SERVICE_WDS;


    LE_PRINT_VALUE("0x%X", serviceType);
    LE_PRINT_VALUE("0x%X", msg_id);

    LE_DEBUG("clientHandle %p",clientHandle);

    // Call all available handlers for this message ID.
    uint i;
    for (i = 0; i < MAX_NUM_HANDLERS_PER_SERVICE; i++)
    {
        if ((MsgHandlers[serviceType][i]).handler != NULL)
        {
            if ((MsgHandlers[serviceType][i]).msgId == msg_id)
            {
                {
                    (MsgHandlers[serviceType][i]).handler(  ind_buf, ind_buf_len,
                                                            clientHandle
                                                         );
                }
            }
        }
        else
        {
            // No more handlers.
            break;
        }
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * Handler for all indications.
 */
//--------------------------------------------------------------------------------------------------
static void IndicationsHandler
(
    qmi_client_type                clientHandle,
    unsigned int                   msg_id,
    void                           *ind_buf,
    unsigned int                   ind_buf_len,
    void                           *ind_cb_data
)
{
    LE_UNUSEDPARAM(ind_cb_data);

    // Determine the service for this indication.
    swiQmi_Service_t serviceType;

    for (serviceType = 0; serviceType < QMI_SERVICE_COUNT; serviceType++)
    {
        if (clientHandle == QmiClientInfo[serviceType].serviceHandle)
        {
            break;
        }
    }

    if (serviceType >= QMI_SERVICE_COUNT)
    {
        LE_ERROR("Unsupported client handle");
        return;
    }

    LE_PRINT_VALUE("0x%X", serviceType);
    LE_PRINT_VALUE("0x%X", msg_id);

    LE_DEBUG("clientHandle %p",clientHandle);

    // Call all available handlers for this message ID.
    uint i;
    for (i = 0; i < MAX_NUM_HANDLERS_PER_SERVICE; i++)
    {
        if ((MsgHandlers[serviceType][i]).handler != NULL)
        {
            if ((MsgHandlers[serviceType][i]).msgId == msg_id)
            {
                (MsgHandlers[serviceType][i]).handler(  ind_buf, ind_buf_len,
                                                        (MsgHandlers[serviceType][i]).contextPtr
                                                     );
            }
        }
        else
        {
            // No more handlers.
            break;
        }
    }
}

static const char * QmiErrorTypeToString
(
    qmi_client_error_type errorType
)
{
    switch(errorType)
    {
        case QMI_NO_ERR:
            return "";
        case QMI_INTERNAL_ERR:
            return "Internal";
        case QMI_SERVICE_ERR:
            return "Service";
        case QMI_TIMEOUT_ERR:
            return "Timeout";
        default:
            return "x";
    }
}

static void PrintServiceDetails
(
    swiQmi_Service_t            serviceType,
    qmi_idl_service_object_type serviceObject
)
{
    LE_INFO("Service.%d: library_version.%d idl_version.%d service_id.0x%X idl_minor_version.%d",
            serviceType,
            serviceObject->library_version,
            serviceObject->idl_version,
            serviceObject->service_id,
            serviceObject->idl_minor_version);
}



//--------------------------------------------------------------------------------------------------
/**
 * Initializes a service.

 * @return
 *      - LE_OK on success
 *      - LE_FAULT on error
 */
//--------------------------------------------------------------------------------------------------
static le_result_t InitService
(
    swiQmi_Service_t serviceType    // The service to initialize.
)
{
    // Get the service type object.
    qmi_idl_service_object_type serviceObject;
    const char * serviceTypeStr = QmiServiceTypeToString(serviceType);

    LE_INFO("Init QMI service %s (%d)", serviceTypeStr,
                                        serviceType);

    switch (serviceType)
    {
        case QMI_SERVICE_NAS:
            serviceObject = nas_get_service_object_v01();
            break;

        case QMI_SERVICE_WMS:
            serviceObject = wms_get_service_object_v01();
            break;

        case QMI_SERVICE_DMS:
            serviceObject = dms_get_service_object_v01();
            break;

        case QMI_SERVICE_VOICE:
            serviceObject = voice_get_service_object_v02();
            break;

        case QMI_SERVICE_UIM:
            serviceObject = uim_get_service_object_v01();
            break;

        case QMI_SERVICE_CAT:
            serviceObject = cat_get_service_object_v02();
            break;

        case QMI_SERVICE_LOC:
            serviceObject = loc_get_service_object_v02();
            break;

#if defined(qapi_m2m_general_get_service_object_v01)
        case QMI_SERVICE_MGS:
            serviceObject = qapi_m2m_general_get_service_object_v01();
            break;
#endif

#if defined(sierra_m2m_audio_api_get_service_object_v01)
        case QMI_SERVICE_AUDIO:
            serviceObject = sierra_m2m_audio_api_get_service_object_v01();
            break;
#endif

#if defined(qapi_m2m_realtime_get_service_object_v01)
        case QMI_SERVICE_REALTIME:
            serviceObject = qapi_m2m_realtime_get_service_object_v01();
            break;
#endif

#if defined(qapi_lwm2m_api_get_service_object_v01)
        case QMI_SERVICE_LWM2M:
            serviceObject = qapi_lwm2m_api_get_service_object_v01();
            break;
#endif

#if defined(qapi_sfs_api_get_service_object_v01)
        case QMI_SERVICE_SFS:
            serviceObject = qapi_sfs_api_get_service_object_v01();
            break;
#endif

#if defined(sierra_swi_bsp_api_get_service_object_v01)
        case QMI_SERVICE_SWI_BSP:
            serviceObject = sierra_swi_bsp_api_get_service_object_v01();
            break;
#endif

#if defined(uim_remote_get_service_object_v01)
        case QMI_SERVICE_UIMRMT:
            serviceObject = uim_remote_get_service_object_v01();
            break;
#endif

#if defined(swiloc_get_service_object_v01)
        case QMI_SERVICE_SWI_LOC:
            serviceObject = swiloc_get_service_object_v01();
            break;
#endif

#if defined(sar_get_service_object_v01)
        case QMI_SERVICE_SWI_SAR:
            serviceObject = sar_get_service_object_v01();
            break;
#endif

        case QMI_SERVICE_PBM:
            serviceObject = pbm_get_service_object_v01();
            break;

        default:
            LE_ERROR("Service type not recognized: %s (%d)", serviceTypeStr,
                                                             serviceType);
            return LE_FAULT;
    }

    if (!serviceObject)
    {
        LE_ERROR("Could not get service object: %s (%d)", serviceTypeStr,
                                                          serviceType);
        return LE_FAULT;
    }

    PrintServiceDetails(serviceType, serviceObject);

    return StartService(serviceObject, IndicationsHandler, &QmiClientInfo[serviceType]);
}


//--------------------------------------------------------------------------------------------------
/**
 * Initializes all the QMI services.

 * @return
 *      - LE_OK on success or if the services were already initialized.
 *      - LE_FAULT on error
 */
//--------------------------------------------------------------------------------------------------
le_result_t swiQmi_InitServices
(
    swiQmi_Service_t serviceType    ///< [IN] The service type to initialize
)
{
    if (Initialized[serviceType])  // Only initialize the services once.
    {
        return LE_OK;
    }

    if (InitService(serviceType) != LE_OK)
    {
        return LE_FAULT;
    }

    Initialized[serviceType] = true;

    return LE_OK;
}



//--------------------------------------------------------------------------------------------------
/**
 * Initializes a QMI service.

 * @return
 *      - LE_OK on success or if the services were already initialized.
 *      - LE_FAULT on error
 */
//--------------------------------------------------------------------------------------------------
le_result_t swiQmi_InitQmiService
(
    swiQmi_Service_t serviceType,                   ///< [IN] The service type to initialize
    swiQmi_ClientInfo_t* clientInfoPtr,            ///< [OUT] Client service instance info
    swiQmi_Param_t   *paramPtr                      ///< [OUT] QMI service parameters
)
{
    if (serviceType != QMI_SERVICE_WDS)
    {
        return LE_FAULT;
    }

    qmi_client_error_type rc;
    static bool initQmux = false;

    if (initQmux == false)
    {
        rc = qmi_cci_qmux_xport_register(QMI_CLIENT_QMUX_RMNET_INSTANCE_1);
        rc = qmi_cci_qmux_xport_register(QMI_CLIENT_QMUX_RMNET_INSTANCE_2);
        rc = qmi_cci_qmux_xport_register(QMI_CLIENT_QMUX_RMNET_INSTANCE_3);
        initQmux = true;
    }

    if (paramPtr->wdsServiceObj == NULL)
    {
        int i = 0;
        unsigned int num_entries = NB_SERVICES_MAX;
        uint32_t service_id = 0;

        /* The fields of paramPtr are resetted one by one, infoIndex is coming from the caller */
        memset(&(paramPtr->wdsServiceObj), 0, sizeof(qmi_idl_service_object_type));
        memset(paramPtr->info, 0, NB_SERVICES_MAX*sizeof(qmi_service_info));
        paramPtr->numServices = 0;

        memset(&(clientInfoPtr->serviceHandle), 0, sizeof(qmi_client_type));

        /*Get the wds service object */
        paramPtr->wdsServiceObj = wds_get_service_object_v01();

        /* Obtain QMI service id for debug purposes */
        qmi_idl_get_service_id(paramPtr->wdsServiceObj, &service_id);

        /* Verify that service object did not return NULL */
        if (!paramPtr->wdsServiceObj)
        {
            LE_DEBUG("wds_service_obj failed");
            return LE_FAULT;
        }

        rc = qmi_client_notifier_init(paramPtr->wdsServiceObj,
                                     &(clientInfoPtr->osParams),
                                     &(clientInfoPtr->notifier));

        if (rc != QMI_NO_ERR)
        {
            LE_DEBUG("client notifier init failed(%d)", rc);
            return LE_FAULT;
        }

        /* Check if the service is up, if not wait on a signal */
        /* Max. wait time = 5 sec */
        for (i = 0; i < MAX_RETRIES; i++)
        {
            QMI_CCI_OS_SIGNAL_CLEAR(&(clientInfoPtr->osParams));

            rc = qmi_client_get_service_list(paramPtr->wdsServiceObj, NULL,
                                             NULL, &paramPtr->numServices);

            LE_DEBUG("qmi_client_get_service_list returned(%d) "
                   "num_services=%d", rc, paramPtr->numServices);

            if (rc == QMI_NO_ERR)
                break;

            QMI_CCI_OS_SIGNAL_WAIT(&(clientInfoPtr->osParams), 1000);
        }

        if (rc != QMI_NO_ERR)
        {
            LE_ERROR("qmi_client_get_service_list time limit exceeded");
            qmi_client_release(clientInfoPtr->notifier);
            return LE_FAULT;
        }

        // Notifier is no longer used, release it here
        LE_DEBUG("Release notifier");
        qmi_client_release(clientInfoPtr->notifier);

        /* The server has come up, store the information in info variable */
        rc = qmi_client_get_service_list(paramPtr->wdsServiceObj, paramPtr->info,
                                         &num_entries, &paramPtr->numServices);

        LE_INFO("qmi_client_get_service_list num_entries %d "
                   "num_services=%d\n", num_entries, paramPtr->numServices);

        if (rc != QMI_NO_ERR)
        {
            LE_ERROR("qmi_client_get_service_list returned(%d) "
                   "num_services=%d\n", rc, paramPtr->numServices);
            return rc;
        }
    }

#if defined(SIERRA_MDM9X28) || defined(SIERRA_MDM9X40)
    {
        rc = qmi_client_init_instance(
            paramPtr->wdsServiceObj,
            QMI_CLIENT_INSTANCE_ANY,
            WdsIndicationsHandler,
            NULL,
            NULL,
            10000,
            &(clientInfoPtr->serviceHandle));
#else
    if (paramPtr->infoIndex < paramPtr->numServices)
    {
        /* Initialize a connection to first QMI control port */
        rc = qmi_client_init(   &paramPtr->info[paramPtr->infoIndex],
                                paramPtr->wdsServiceObj,
                                WdsIndicationsHandler,
                                NULL,
                                NULL,
                                &(clientInfoPtr->serviceHandle));
#endif

        if (rc != QMI_NO_ERR)
        {
            LE_ERROR("Error: connection not Initialized...Error Code:%d", rc);
            rc = qmi_client_release(clientInfoPtr->serviceHandle);

            if (rc < 0)
            {
                LE_ERROR("Release not successful");
            }
            else
            {
                LE_DEBUG("Qmi client release successful");
            }
        }
        else
        {
            LE_DEBUG("Connection Initialized....User Handle:%p", clientInfoPtr->serviceHandle);
        }
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Release a QMI service.
 *
 */
//--------------------------------------------------------------------------------------------------
void swiQmi_ReleaseService
(
    qmi_client_type handle
)
{
    qmi_client_error_type rc;

    rc = qmi_client_release(handle);

    if (rc < 0)
    {
        LE_ERROR("Release not successful");
    }
}

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
qmi_client_type swiQmi_GetClientHandle
(
    swiQmi_Service_t serviceType  ///< [IN] The service type to get the client handle for.
)
{
    if ( (serviceType >= QMI_SERVICE_COUNT) || (serviceType < 0) )
    {
        return NULL;
    }

    return QmiClientInfo[serviceType].serviceHandle;
}


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
le_result_t swiQmi_AddIndicationHandler
(
    swiQmi_IndicationHandler_t handler,         ///< Indication handler.
    swiQmi_Service_t serviceType,               ///< The service to register the handler with.
    unsigned int msgId,                         ///< The message ID of the indication to receive.
    void* contextPtr                            ///< Value to pass to handler when it is called.
)
{
    LE_ASSERT(serviceType < QMI_SERVICE_COUNT);

    uint i;
    for (i = 0; i < MAX_NUM_HANDLERS_PER_SERVICE; i++)
    {
        if (MsgHandlers[serviceType][i].handler == NULL)
        {
            MsgHandlers[serviceType][i].handler = handler;
            MsgHandlers[serviceType][i].msgId = msgId;
            MsgHandlers[serviceType][i].contextPtr = contextPtr;
            return LE_OK;
        }
    }

    LE_CRIT("Too many handlers. MAX_NUM_HANDLERS_PER_SERVICE is %d.",
            MAX_NUM_HANDLERS_PER_SERVICE);
    return LE_FAULT;
}


//--------------------------------------------------------------------------------------------------
/**
 * Remove an indication handler.
 */
//--------------------------------------------------------------------------------------------------
void swiQmi_RemoveIndicationHandler
(
    swiQmi_IndicationHandler_t handler,         ///< Indication handler.
    swiQmi_Service_t serviceType,               ///< The service.
    unsigned int msgId                          ///< The message ID.
)
{
    LE_ASSERT(serviceType < QMI_SERVICE_COUNT);

    uint i;
    for (i = 0; i < MAX_NUM_HANDLERS_PER_SERVICE; i++)
    {
        if ((MsgHandlers[serviceType][i].handler == handler) &&
            (MsgHandlers[serviceType][i].msgId == msgId))
        {
            MsgHandlers[serviceType][i].handler = NULL;
            MsgHandlers[serviceType][i].msgId = 0;
            return;
        }
    }
}


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
le_result_t swiQmi_CheckResponse
(
    const char* msgIdStr,           ///< [IN] The string for the QMI message
    qmi_client_error_type rc,       ///< [IN] Response from the send function
    int32_t result,                 ///< [IN] Result from QMI response message
    int32_t error                   ///< [IN] Error from QMI response message
)
{
    // todo: Should we also check if resp.error == QMI_ERR_NONE_V01. I don't think its necessary.
    if ( (rc==QMI_NO_ERR) && (result==QMI_RESULT_SUCCESS_V01) )
    {
        LE_DEBUG("%s sent to Modem", msgIdStr);
        return LE_OK;
    }
    else
    {
        LE_ERROR("Sending %s failed: rc=%i (%s), resp.result=%i.[0x%02x], resp.error=%i.[0x%02x]",
                    msgIdStr, rc, QmiErrorTypeToString(rc), result, result, error, error);
        LE_ERROR_IF(((rc == QMI_IDL_LIB_MESSAGE_ID_NOT_FOUND)         ||
                     (rc == QMI_IDL_LIB_UNRECOGNIZED_SERVICE_VERSION) ||
                     (rc == QMI_IDL_LIB_INCOMPATIBLE_SERVICE_VERSION)),
                    "Modem is running an incompatible version of client library!");
        return LE_FAULT;
    }
}


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
le_result_t swiQmi_CheckResponseCode
(
    const char* msgIdStr,           ///< [IN] The string for the QMI message
    qmi_client_error_type rc,       ///< [IN] Response from the send function
    qmi_response_type_v01 resp      ///< [IN] Response value in the QMI response message
)
{
    return swiQmi_CheckResponse(msgIdStr, rc, resp.result, resp.error);
}


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
le_result_t swiQmi_OEMCheckResponseCode
(
    const char* msgIdStr,           ///< [IN] The string for the QMI message
    qmi_client_error_type rc,       ///< [IN] Response from the send function
#if (QAPI_SWI_COMMON_V01_IDL_MINOR_VERS <= 1)
    oem_response_type_v01 resp      ///< [IN] Response value in the QMI response message
#else
    qmi_response_type_v01 resp      ///< [IN] Response value in the QMI response message
#endif
)
{
    return swiQmi_CheckResponse(msgIdStr, rc, resp.result, resp.error);
}
