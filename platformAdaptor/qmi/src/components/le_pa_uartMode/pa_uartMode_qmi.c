/**
 * @file pa_uartMode_qmi.c
 *
 * QMI implementation of @ref c_pa_uartMode API.
 *
 * This API allows the caller to set the mode for UART ports.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "pa_uartMode.h"
#include "swiQmi.h"


//--------------------------------------------------------------------------------------------------
/**
 * The device management client handle.  Must be obtained by calling swiQmi_GetClientHandle() before
 * it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type DmsClientHandle = NULL;


//--------------------------------------------------------------------------------------------------
/**
 * QMI request timeout in milliseconds.
 */
//--------------------------------------------------------------------------------------------------
#define QMI_REQ_TIMEOUT             10000


//--------------------------------------------------------------------------------------------------
/**
 * Translates a UART mode value to a QMI specific mode value.
 */
//--------------------------------------------------------------------------------------------------
static uint8_t UartModeToQmiMode
(
    pa_uartMode_Mode_t mode                     ///< [IN] Port mode.
)
{
    switch (mode)
    {
        case UART_DISABLED:
            return 0;

        case UART_AT_CMD:
            return 1;

        case UART_DIAG_MSG:
            return 2;

        case UART_NMEA:
            return 4;

        case UART_LINUX_CONSOLE:
            return 16;

        case UART_LINUX_APP:
            return 17;

        default:
            LE_FATAL("Unrecognized mode value %d.", mode);
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * Translates a QMI specific mode value to the PA interfaces UART mode.
 */
//--------------------------------------------------------------------------------------------------
static pa_uartMode_Mode_t QmiModeToUartMode
(
    uint8_t qmiUartMode                         ///< [IN] Port mode.
)
{
    switch (qmiUartMode)
    {
        case 0:
            return UART_DISABLED;

        case 1:
            return UART_AT_CMD;

        case 2:
            return UART_DIAG_MSG;

        case 4:
            return UART_NMEA;

        case 16:
            return UART_LINUX_CONSOLE;

        case 17:
            return UART_LINUX_APP;

        default:
            LE_FATAL("Unrecognized qmi mode value %d.", qmiUartMode);
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * Gets the current mode.
 *
 * @return
 *      LE_OK if successful.
 *      LE_FAULT if there was an error.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_uartMode_Get
(
    uint uartNum,                               ///< [IN] 1 for UART1, 2 for UART2.
    pa_uartMode_Mode_t* modePtr                 ///< [OUT] Port's mode.
)
{
    SWIQMI_DECLARE_MESSAGE(dms_swi_get_map_uart_resp_msg_v01, mapUartResp);

    // Send the message and handle the response
    qmi_client_error_type rc = qmi_client_send_msg_sync(DmsClientHandle,
                                                        QMI_DMS_SWI_GET_MAP_UART_REQ_V01,
                                                        NULL, 0,
                                                        &mapUartResp, sizeof(mapUartResp),
                                                        QMI_REQ_TIMEOUT);

    le_result_t result = swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_DMS_SWI_GET_MAP_UART_REQ_V01),
                                              rc,
                                              mapUartResp.resp.result,
                                              mapUartResp.resp.error);

    if (result != LE_OK)
    {
        LE_CRIT("Unexpected QMI response.");
        return LE_FAULT;
    }

    if (uartNum == 1)
    {
        *modePtr = QmiModeToUartMode(mapUartResp.transfer_byte_mode.UART1_service);
    }
    else if (uartNum == 2)
    {
        *modePtr = QmiModeToUartMode(mapUartResp.transfer_byte_mode.UART2_service);
    }
    else
    {
        LE_ERROR("Unsupported UART port number %d.", uartNum);
        return LE_FAULT;
    }

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Sets the current mode.
 *
 * @return
 *      LE_OK if successful.
 *      LE_FAULT if there was an error.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_uartMode_Set
(
    uint uartNum,                               ///< [IN] 1 for UART1, 2 for UART2.
    pa_uartMode_Mode_t mode                     ///< [IN] Port's mode.
)
{
    if ( (uartNum != 1) && (uartNum != 2) )
    {
        LE_ERROR("Unsupported UART port number %d.", uartNum);
        return LE_FAULT;
    }

    uint8_t qmiUartMode = UartModeToQmiMode(mode);

    SWIQMI_DECLARE_MESSAGE(dms_swi_set_map_uart_req_msg_v01, mapUartReq);
    SWIQMI_DECLARE_MESSAGE(dms_swi_set_map_uart_resp_msg_v01, mapUartResp);

    // Fill in the request.
    // WARNING: UART1_service refers to the mode while UART2_service refers to the uart number here!
    //          The dms_swi_set_map_uart_req_msg_v01.transfer_byte_mode structure members are named
    //          horribly because it is reused from the dms_swi_get_map_uart_resp_msg_v01 structure!
    mapUartReq.transfer_byte_mode.UART1_service = qmiUartMode;
    mapUartReq.transfer_byte_mode.UART2_service = uartNum;

    // Send the message and handle the response
    qmi_client_error_type rc = qmi_client_send_msg_sync(DmsClientHandle,
                                                        QMI_DMS_SWI_SET_MAP_UART_REQ_V01,
                                                        &mapUartReq, sizeof(mapUartReq),
                                                        &mapUartResp, sizeof(mapUartResp),
                                                        QMI_REQ_TIMEOUT);

    le_result_t result = swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_DMS_SWI_SET_MAP_UART_REQ_V01),
                                              rc,
                                              mapUartResp.resp.result,
                                              mapUartResp.resp.error);

    if (result != LE_OK)
    {
        LE_CRIT("Unexpected QMI response.");
        return LE_FAULT;
    }

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Init this component
 */
//--------------------------------------------------------------------------------------------------
COMPONENT_INIT
{
    // Initialize the required service
    LE_FATAL_IF(swiQmi_InitServices(QMI_SERVICE_DMS) != LE_OK,
                "Could not initialize QMI device management service.");

    // Get the qmi client service handle for later use.
    DmsClientHandle = swiQmi_GetClientHandle(QMI_SERVICE_DMS);

    LE_FATAL_IF(DmsClientHandle == NULL,
                "Could not get service handle for QMI device management service.");
}
