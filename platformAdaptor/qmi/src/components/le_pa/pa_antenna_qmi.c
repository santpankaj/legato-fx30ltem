/**
 * @file pa_antenna_qmi.c
 *
 * QMI implementation of antenna API.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "interfaces.h"
#include "pa_antenna.h"
#include "swiQmi.h"


//--------------------------------------------------------------------------------------------------
// Symbol and Enum definitions.
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Antenna identidier definition
 */
//--------------------------------------------------------------------------------------------------
typedef uint8_t pa_antenna_Id_t;

//--------------------------------------------------------------------------------------------------
/**
 * The QMI clients. Must be obtained by calling swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type RealtimeClient;

//--------------------------------------------------------------------------------------------------
//                                       Static declarations
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
/**
 * Antenna status Event
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t AntennaStatusEvent;

//--------------------------------------------------------------------------------------------------
/**
 * Antenna selected for the status indication event
 */
//--------------------------------------------------------------------------------------------------
static uint8_t AntennaSelectionMask = 0;

//--------------------------------------------------------------------------------------------------
/**
 * PA Antenna context
 */
//--------------------------------------------------------------------------------------------------
static struct
{
    le_antenna_Status_t currentStatus;

} PaAntennaCtx[LE_ANTENNA_MAX];



//--------------------------------------------------------------------------------------------------
/**
 * This function is called to convert internal antenna identifier to le_antenna_Type_t type used
 * by le_mon_antenna.c
 *
 * @return
 * - LE_OK on success
 * - LE_FAULT when the antenna identifier given in parameter is unknown
 *
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ConvertAntennaIdToType
(
    pa_antenna_Id_t    antennaId,
    le_antenna_Type_t* antennaTypePtr
)
{
    switch (antennaId)
    {
        case 1:
        {
            *antennaTypePtr = LE_ANTENNA_PRIMARY_CELLULAR;
        }
        break;

        case 2:
        {
            *antennaTypePtr = LE_ANTENNA_DIVERSITY_CELLULAR;
        }
        break;

        case 4:
        {
            *antennaTypePtr = LE_ANTENNA_GNSS;
        }
        break;

        default:
            return LE_FAULT;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function is called to convert the le_antenna_Type_t type used by le_mon_antenna.c into
 * the pa internal identifier (used by QMI)
 *
 * @return
 * - LE_OK on success
 * - LE_FAULT when the antenna type given in parameter is unknown
 * - LE_UNSUPPORTED when the antenna type is not supported on the platform
 *
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ConvertAntennaTypeToId
(
    le_antenna_Type_t antennaType,
    pa_antenna_Id_t* antennaIdPtr
)
{
    switch (antennaType)
    {
        case LE_ANTENNA_PRIMARY_CELLULAR:
        {
            *antennaIdPtr = 1;
        }
        break;

        case LE_ANTENNA_DIVERSITY_CELLULAR:
        {
            *antennaIdPtr = 2;
        }
        break;

        case LE_ANTENNA_GNSS:
        {
            *antennaIdPtr = 4;
        }
        break;

        default:
            return LE_FAULT;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function is used to get the open and short limits
 *
 * @return
 * - LE_OK on success
 * - LE_FAULT on failure
 *
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetLimits
(
    pa_antenna_Id_t          antennaId,
    uint32_t*                shortLimitPtr,
    uint32_t*                openLimitPtr
)
{
    swi_m2m_antenna_read_limit_req_msg_v01 limitReq = { 0 };
    swi_m2m_antenna_read_limit_resp_msg_v01 limitRsp = {{0}};

    limitReq.antenna_selection = antennaId;

    LE_DEBUG("antennaId %d", antennaId);

    /* Get the limits from the QMI service */
    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(RealtimeClient,
                                            QMI_SWI_M2M_ANTENNA_READ_LIMIT_REQ_V01,
                                            &limitReq, sizeof(limitReq),
                                            &limitRsp, sizeof(limitRsp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    /* Check the response */
    if ( swiQmi_OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_M2M_ANTENNA_READ_LIMIT_REQ_V01),
                                     clientMsgErr,
                                     limitRsp.resp) != LE_OK )
    {
        LE_ERROR("Cannot get limit of antenna %d", antennaId);
        return LE_FAULT;
    }

    /* Get the limits if the data are valid */
    if ( limitRsp.antenna_cfg_setting_valid )
    {
        if (limitRsp.antenna_cfg_setting.antenna_selection == antennaId)
        {
            *shortLimitPtr =  limitRsp.antenna_cfg_setting.short_limit;
            *openLimitPtr =  limitRsp.antenna_cfg_setting.open_limit;

            return LE_OK;
        }
    }

    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function is used to set the open and short limits
 *
 * @return
 * - LE_OK on success
 * - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SetLimits
(
    pa_antenna_Id_t          antennaId,
    uint32_t                 shortLimit,
    uint32_t                 openLimit
)
{
    swi_m2m_antenna_cfg_limit_req_msg_v01 limitReq = {{0}};
    swi_m2m_antenna_cfg_limit_resp_msg_v01 limitRsp = {{0}};

    limitReq.antenna_select_configure.antenna_selection = antennaId;
    limitReq.antenna_select_configure.short_limit = (uint16_t) shortLimit;
    limitReq.antenna_select_configure.open_limit = (uint16_t) openLimit;

    /* Set the limits using the QMI service */
    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(RealtimeClient,
                                            QMI_SWI_M2M_ANTENNA_CFG_LIMIT_REQ_V01,
                                            &limitReq, sizeof(limitReq),
                                            &limitRsp, sizeof(limitRsp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    /* Check the response */
    if ( swiQmi_OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_M2M_ANTENNA_CFG_LIMIT_REQ_V01),
                                     clientMsgErr,
                                     limitRsp.resp) != LE_OK )
    {
        LE_ERROR("Cannot set limit (%d,%d) of antenna %d", shortLimit, openLimit, antennaId);
        return LE_FAULT;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * New message handler function called by the QMI service indications.
 */
//--------------------------------------------------------------------------------------------------
static void NewMsgHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    swi_m2m_antenna_status_ind_msg_v01 statusInd;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        RealtimeClient, QMI_IDL_INDICATION, QMI_SWI_M2M_ANTENNA_STATUS_IND_V01,
        indBufPtr, indBufLen,
        &statusInd, sizeof(statusInd));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_SWI_M2M_ANTENNA_STATUS_IND_V01, error = %i", clientMsgErr);
        return;
    }

    LE_DEBUG("antenna_status_len %d", statusInd.antenna_status_len);

    // Inform the le_mon_antenna of the antenna status
    while ( statusInd.antenna_status_len )
    {
        pa_antenna_StatusInd_t antennaStatus;

        // Convert the QMI antenna identifier into le_antenna_Type_t
        le_result_t result = ConvertAntennaIdToType(
                                statusInd.antenna_status[statusInd.antenna_status_len-1].antenna,
                                &antennaStatus.antennaType );

        if ( result != LE_OK )
        {
            LE_ERROR("Unknown antenna Id %d",
                statusInd.antenna_status[statusInd.antenna_status_len-1].antenna);
        }
        else
        {
            // Check the status
            if (( statusInd.antenna_status[statusInd.antenna_status_len-1].status <
                                                                    LE_ANTENNA_LAST_STATUS ) &&
                ( statusInd.antenna_status[statusInd.antenna_status_len-1].status !=
                                            PaAntennaCtx[antennaStatus.antennaType].currentStatus ))
            {
                antennaStatus.status =
                        statusInd.antenna_status[statusInd.antenna_status_len-1].status;

                LE_DEBUG("Trigger antennaType %d status %d", antennaStatus.antennaType,
                                                                antennaStatus.status);

                // Update the current status in the context
                PaAntennaCtx[antennaStatus.antennaType].currentStatus = antennaStatus.status;

                // Trigger the event
                le_event_Report(AntennaStatusEvent, &antennaStatus,
                                sizeof(pa_antenna_StatusInd_t));
            }
        }

        statusInd.antenna_status_len--;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the periodic antenna check
 *
 *  @return
 * - LE_OK on success
 * - LE_FAULT on failure
 *
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SetAntennaStatusIndication
(
    void
)
{
    swi_m2m_antenna_cfg_poll_period_req_msg_v01 periodReq = {0};
    swi_m2m_antenna_cfg_poll_period_resp_msg_v01 periodeRsp = {{0}};

    periodReq.antenna_selection_mask = AntennaSelectionMask;
    periodReq.interval_s_valid = true;
    periodReq.interval_s = 5;

    LE_DEBUG("antenna_selection_mask %d", periodReq.antenna_selection_mask);

    // Set the antenna polling check
    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(RealtimeClient,
                                            QMI_SWI_M2M_ANTENNA_CFG_POLL_PERIOD_REQ_V01,
                                            &periodReq, sizeof(periodReq),
                                            &periodeRsp, sizeof(periodeRsp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    // Check the response
    if (swiQmi_OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_M2M_ANTENNA_CFG_POLL_PERIOD_REQ_V01),
                                        clientMsgErr,
                                        periodeRsp.resp) != LE_OK)
    {
        LE_ERROR("Could not register message indication");
        return LE_FAULT;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
//                                       Public declarations
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to set the short limit
 *
 * @return
 * - LE_OK on success
 * - LE_FAULT on failure
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_SetShortLimit
(
    le_antenna_Type_t    antennaType,
    uint32_t             shortLimit
)
{
    pa_antenna_Id_t antennaId;

    /* convert the LE antenna type into the QMI type */
    if ( ConvertAntennaTypeToId(antennaType, &antennaId) != LE_OK )
    {
        LE_ERROR("Unknown antenna type %d", antennaType);

        return LE_FAULT;
    }

    uint32_t currentShortLimit = 0;
    uint32_t currentOpenLimit = 0;

    /* Get the current limits */
    if ( GetLimits ( antennaId, &currentShortLimit, &currentOpenLimit ) == LE_OK )
    {
        /* Set the short limit and the current open limit */
        return SetLimits( antennaId, shortLimit, currentOpenLimit);
    }

    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to get the short limit
 *
 * @return
 * - LE_OK on success
 * - LE_FAULT on failure
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_GetShortLimit
(
    le_antenna_Type_t    antennaType,
    uint32_t*            shortLimitPtr
)
{
    pa_antenna_Id_t antennaId;

    /* convert the LE antenna type into the QMI type */
    if ( ConvertAntennaTypeToId(antennaType, &antennaId) != LE_OK )
    {
        LE_ERROR("Unknown antenna type %d", antennaType);

        return LE_FAULT;
    }

    uint32_t openLimit = 0;

    /* Get the open and short limits */
    return GetLimits ( antennaId, shortLimitPtr, &openLimit );
}

//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to set the open limit
 *
 * @return
 * - LE_OK on success
 * - LE_FAULT on failure
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_SetOpenLimit
(
    le_antenna_Type_t    antennaType,
    uint32_t             openLimit
)
{
    pa_antenna_Id_t antennaId;

    /* convert the LE antenna type into the QMI type */
    if ( ConvertAntennaTypeToId(antennaType, &antennaId) != LE_OK )
    {
        LE_ERROR("Unknown antenna type %d", antennaType);

        return LE_FAULT;
    }

    uint32_t currentShortLimit = 0;
    uint32_t currentOpenLimit = 0;

    /* Get the current limits */
    if ( GetLimits ( antennaId, &currentShortLimit, &currentOpenLimit ) == LE_OK )
    {
        /* set the new open limit and the current short limit */
        return SetLimits( antennaId, currentShortLimit, openLimit);
    }

    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to get the open limit
 *
 * @return
 * - LE_OK on success
 * - LE_FAULT on failure
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_GetOpenLimit
(
    le_antenna_Type_t    antennaType,
    uint32_t*            openLimitPtr
)
{
    pa_antenna_Id_t antennaId;

    /* convert the LE antenna type into the QMI type */
    if ( ConvertAntennaTypeToId(antennaType, &antennaId) != LE_OK )
    {
        LE_ERROR("Unknown antenna type %d", antennaType);

        return LE_FAULT;
    }

    uint32_t shortLimit = 0;

    /* Get the open and short limits */
    return GetLimits ( antennaId, &shortLimit, openLimitPtr );
}

//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to get the antenna status
 *
 * @return
 * - LE_OK on success
 * - LE_UNSUPPORTED request not supported
 * - LE_FAULT on failure
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_GetStatus
(
    le_antenna_Type_t    antennaType,
    le_antenna_Status_t* statusPtr
)
{
    pa_antenna_Id_t antennaId;

    /* convert the LE antenna type into the QMI type */
    if ( ConvertAntennaTypeToId(antennaType, &antennaId) != LE_OK )
    {
        LE_ERROR("Unknown antenna type %d", antennaType);
        return LE_FAULT;
    }

    swi_m2m_antenna_read_status_resp_msg_v01 statusRsp = { {0} };

    /* Get the limits from the QMI service */
    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(RealtimeClient,
                                            QMI_SWI_M2M_ANTENNA_READ_STATUS_REQ_V01,
                                            NULL, 0,
                                            &statusRsp, sizeof(statusRsp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    /* Check the response */
    if ( swiQmi_OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_M2M_ANTENNA_READ_STATUS_REQ_V01),
                                     clientMsgErr,
                                     statusRsp.resp) != LE_OK )
    {
        // test if Antenna detection is supported
        if ((QMI_NO_ERR==clientMsgErr) &&
            (QMI_ERR_OP_DEVICE_UNSUPPORTED_V01 == statusRsp.resp.error))
        {
            LE_ERROR("Antenna detection is not supported");
            return LE_UNSUPPORTED;
        }
        else
        {
            LE_ERROR("Cannot get status of antenna %d", antennaId);
            return LE_FAULT;
        }
    }

    /* check the status of the requested antenna */
    if ( statusRsp.antenna_status_valid )
    {
        while ( statusRsp.antenna_status_len )
        {
            if ( statusRsp.antenna_status[statusRsp.antenna_status_len-1].antenna == antennaId )
            {
                // Convert antenna QMI status to Legato status
                switch(statusRsp.antenna_status[statusRsp.antenna_status_len-1].status)
                {
                    case 0: // Shorted
                    {
                        *statusPtr = LE_ANTENNA_SHORT_CIRCUIT;
                        return LE_OK;
                    }
                    break;

                    case 1: // Normal
                    {
                        *statusPtr = LE_ANTENNA_CLOSE_CIRCUIT;
                        return LE_OK;
                    }
                    break;

                    case 2: // Open
                    {
                        *statusPtr = LE_ANTENNA_OPEN_CIRCUIT;
                        return LE_OK;
                    }
                    break;

                    case 3: // Over Current
                    {
                        *statusPtr = LE_ANTENNA_OVER_CURRENT;
                        return LE_OK;
                    }
                    break;

                    case 255: // "-1" Inactive
                    {
                        *statusPtr = LE_ANTENNA_INACTIVE;
                        return LE_OK;
                    }
                    break;

                    default:
                    {
                        LE_ERROR("Unknown antenna status %d"
                        , statusRsp.antenna_status[statusRsp.antenna_status_len-1].status);
                        return LE_FAULT;
                    }
                }
            }
            // Next antenna
            statusRsp.antenna_status_len--;
        }
    }

    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the external ADC used to monitor the requested antenna.
 *
 * @return
 *      - LE_OK on success
 *      - LE_UNSUPPORTED request not supported
 *      - LE_FAULT on other failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_SetExternalAdc
(
    le_antenna_Type_t    antennaType,   ///< Antenna type
    int8_t               adcId          ///< The external ADC used to monitor the requested antenna
)
{
#if !defined (SIERRA_MDM9X28)
    swi_m2m_antenna_set_select_adc_req_msg_v01 selectAdcReq = {0};
    swi_m2m_antenna_set_select_adc_resp_msg_v01 selectAdcRsp = {{0}};
    pa_antenna_Id_t antennaId = 0;
    uint8_t antennaAdcQmiId = 0;

    /* Convert the LE antenna type into the QMI type */
    if ( ConvertAntennaTypeToId(antennaType, &antennaId) != LE_OK )
    {
        LE_ERROR("Unknown antenna type %d", antennaType);
        return LE_FAULT;
    }

    LE_DEBUG("Set ADC %d for antenna %d", adcId, antennaId);

    /* Convert the LE antenna ADC into the QMI type */
    if(adcId == 0)
    {
        // ADC index 0 not supported for AR7/AR8 platform
        // QMI adc_type "0" means INTERNAL_ADC/ No External ADC
        LE_ERROR("Unsuported configuration %d for that platform", adcId);
        return LE_UNSUPPORTED;
    }
    else if(adcId == -1)
    {
        antennaAdcQmiId = 0;
    }
    else
    {
        antennaAdcQmiId = adcId;
    }

    /* Set the request message */
    selectAdcReq.antenna_adc_selection_len = 1;
    selectAdcReq.antenna_adc_selection[0].antenna_selection = antennaId;
    selectAdcReq.antenna_adc_selection[0].adc_type =antennaAdcQmiId;

    // Set the antenna ADC
    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(RealtimeClient,
                                            QMI_SWI_M2M_ANTENNA_SET_SELECT_ADC_REQ_V01,
                                            &selectAdcReq, sizeof(selectAdcReq),
                                            &selectAdcRsp, sizeof(selectAdcRsp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    // Check the response
    if (swiQmi_OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_M2M_ANTENNA_SET_SELECT_ADC_RESP_V01),
                                        clientMsgErr,
                                        selectAdcRsp.resp) != LE_OK)
    {
        LE_ERROR("Could not register message indication");
        return LE_FAULT;
    }

    return LE_OK;
#else

    LE_WARN("Set External Adc is not supported on this platform");
    return LE_UNSUPPORTED;
#endif
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the external ADC used to monitor the requested antenna.
 *
 * @return
 *      - LE_OK on success
 *      - LE_UNSUPPORTED request not supported
 *      - LE_FAULT on other failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_GetExternalAdc
(
    le_antenna_Type_t    antennaType,  ///< Antenna type
    int8_t*              adcIdPtr      ///< The external ADC used to monitor the requested antenna
)
{
#if !defined (SIERRA_MDM9X28)
    pa_antenna_Id_t antennaId;
    swi_m2m_antenna_get_select_adc_resp_msg_v01 selectAdcRsp = {{0}};
    int i;

    /* Convert the LE antenna type into the QMI type */
    if ( ConvertAntennaTypeToId(antennaType, &antennaId) != LE_OK )
    {
        LE_ERROR("Unknown antenna type %d", antennaType);
        return LE_FAULT;
    }

    LE_DEBUG("Get ADC for antenna %d", antennaId);

    /* Get the limits from the QMI service */
    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(RealtimeClient,
                                            QMI_SWI_M2M_ANTENNA_GET_SELECT_ADC_REQ_V01,
                                            NULL, 0, // Empty field
                                            &selectAdcRsp, sizeof(selectAdcRsp),
                                            COMM_DEFAULT_PLATFORM_TIMEOUT);

    /* Check the response */
    if ( swiQmi_OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_M2M_ANTENNA_GET_SELECT_ADC_RESP_V01),
                                     clientMsgErr,
                                     selectAdcRsp.resp) != LE_OK )
    {
        LE_ERROR("Cannot get selected ADC for antenna %d", antennaId);
        return LE_FAULT;
    }

    if(selectAdcRsp.antenna_adc_selection_valid == false)
    {
        LE_ERROR("ADC selection not valid");
        return LE_FAULT;
    }

    /* Get the selected value */
    for( i = 0; i < selectAdcRsp.antenna_adc_selection_len; i++ )
    {
        if(selectAdcRsp.antenna_adc_selection[i].antenna_selection == antennaId)
        {
            LE_DEBUG("antennaId %d - ADC %d"
                    , selectAdcRsp.antenna_adc_selection[i].antenna_selection
                    , selectAdcRsp.antenna_adc_selection[i].adc_type);

            /* Convert the QMI type to the LE antenna ADC */
            switch (selectAdcRsp.antenna_adc_selection[i].adc_type)
            {
                case 0:
                {
                    *adcIdPtr = -1;
                    return LE_OK;
                }
                break;

                case 1:
                case 2:
                {
                    *adcIdPtr = selectAdcRsp.antenna_adc_selection[i].adc_type;
                    return LE_OK;
                }
                break;

                default:
                {
                    LE_ERROR("Unknown ADC type %d for antenna %d"
                            , selectAdcRsp.antenna_adc_selection[i].adc_type
                            , antennaId);
                    return LE_FAULT;
                }
            }
        }
    }

    LE_WARN("Get ADC for antenna %d not found", antennaId);
    return LE_UNSUPPORTED;
#else

    LE_WARN("get External Adc is not supported on this platform");
    return LE_UNSUPPORTED;
#endif
}

//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to set the status indication on a specific antenna
 *
 * * @return
 * - LE_OK on success
 * - LE_BUSY if the status indication is already set for the requested antenna
 * - LE_FAULT on other failure
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_SetStatusIndication
(
    le_antenna_Type_t antennaType
)
{
    pa_antenna_Id_t antennaId;

    /* convert the LE antenna type into the QMI type */
    if ( ConvertAntennaTypeToId(antennaType, &antennaId) != LE_OK )
    {
        LE_ERROR("Unknown antenna type %d", antennaType);
        return LE_FAULT;
    }

    /*  Check if antenna status is not already selected */
    if ( !( AntennaSelectionMask & antennaId ) )
    {
        // initialize the current status
        if ( pa_antenna_GetStatus(antennaType, &PaAntennaCtx[antennaType].currentStatus) != LE_OK )
        {
            LE_ERROR("Unable to get the status");
            return LE_FAULT;
        }

        /* Remember the request for the next time */
        AntennaSelectionMask |= antennaId;

        LE_DEBUG("AntennaSelectionMask %d", AntennaSelectionMask);

        return SetAntennaStatusIndication();
    }
    else
    {
        return LE_BUSY;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to remove the status indication on a specific antenna
 *
 * @return
 * - LE_OK on success
 * - LE_FAULT on failure
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_RemoveStatusIndication
(
    le_antenna_Type_t antennaType
)
{
    pa_antenna_Id_t antennaId;

    /* convert the LE antenna type into the QMI type */
    if ( ConvertAntennaTypeToId(antennaType, &antennaId) != LE_OK )
    {
        LE_ERROR("Unknown antenna type %d", antennaType);

        return LE_FAULT;
    }

    if ( AntennaSelectionMask & antennaId )
    {
        AntennaSelectionMask &= ~ antennaId;

        LE_DEBUG("AntennaSelectionMask %d", AntennaSelectionMask);

        return SetAntennaStatusIndication();
    }
    else
    {
        // Antenna not selected
        LE_ERROR("No subscribed to the status indication");

        return LE_FAULT;
    }

}

//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to add a status notification handler
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t* pa_antenna_AddStatusHandler
(
    pa_antenna_StatusIndHandlerFunc_t   msgHandler
)
{
    le_event_HandlerRef_t  handlerRef = NULL;

    if ( msgHandler != NULL )
    {
        handlerRef = le_event_AddHandler( "PaAntennaStatusHandler",
                              AntennaStatusEvent,
                            (le_event_HandlerFunc_t) msgHandler);
    }
    else
    {
        LE_ERROR("Null handler given in parameter");
    }

    return (le_event_HandlerRef_t*) handlerRef;
}

//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to intialize the PA antenna
 * @return
 * - LE_OK on success
 * - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_Init
(
    void
)
{
    // Create the event for signaling user handlers.
    AntennaStatusEvent = le_event_CreateId("AntennaStatusEvent",sizeof(pa_antenna_StatusInd_t));

    if (swiQmi_InitServices(QMI_SERVICE_REALTIME) != LE_OK)
    {
        LE_CRIT("QMI_SERVICE_REALTIME cannot be initialized.");
        return LE_FAULT;
    }

    // Get the qmi client service handle for later use.
    if ((RealtimeClient = swiQmi_GetClientHandle(QMI_SERVICE_REALTIME)) == NULL)
    {
        LE_ERROR("Cannot get QMI_SERVICE_REALTIME client!");
        return LE_FAULT;
    }

    // Register our own handler with the QMI DMS service.
    swiQmi_AddIndicationHandler(NewMsgHandler, QMI_SERVICE_REALTIME,
                                QMI_SWI_M2M_ANTENNA_STATUS_IND_V01, NULL);

    return LE_OK;
}
