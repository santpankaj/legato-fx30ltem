/**
 * @file pa_mdmCfg_qmi.c
 *
 * QMI implementation of @ref c_pa_mdmCfg API.
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include "legato.h"
#include "interfaces.h"
#include "swiQmi.h"
#include "pa_mdmCfg.h"


//--------------------------------------------------------------------------------------------------
// Static declarations
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * The MGS (M2M General Service) client.
 * Must be obtained by calling swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type MgsClient = NULL;


//--------------------------------------------------------------------------------------------------
// Public functions
//--------------------------------------------------------------------------------------------------

#if defined(QMI_SWI_M2M_NV_BACKUP_REQ_V01)
//--------------------------------------------------------------------------------------------------
/**
 * Store the modem current configuration.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_FAULT          The function failed.
 *  - LE_UNSUPPORTED    The function does not supported on this platform.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdmCfg_StoreCurrentConfiguration
(
    void
)
{
    le_result_t result;
    qmi_client_error_type rc;

    SWIQMI_DECLARE_MESSAGE(swi_m2m_nv_backup_resp_msg_v01, mgsResp);

    // Send request to FW
    rc = qmi_client_send_msg_sync(MgsClient,
                                  QMI_SWI_M2M_NV_BACKUP_REQ_V01,
                                  NULL, 0,
                                  &mgsResp, sizeof(mgsResp),
                                  COMM_LONG_PLATFORM_TIMEOUT);

    result = swiQmi_CheckResponse(STRINGIZE_EXPAND (QMI_SWI_M2M_NV_BACKUP_RESP_V01),
                                  rc,
                                  mgsResp.resp.result,
                                  mgsResp.resp.error);

    if (LE_OK != result)
    {
        LE_ERROR("Error on NV Backup: rc %d, resp error %d", rc, mgsResp.resp.error);
    }
    else
    {
        LE_INFO("NV Backup: %hu stored, %hu restored, %hu deleted, %hu defaulted,"
                " %hu skipped, %hu failed, %hu fail info",
                mgsResp.stored, mgsResp.restored, mgsResp.deleted, mgsResp.defaulted,
                mgsResp.skipped, mgsResp.failed, mgsResp.fail_info);
    }

    LE_DEBUG("return %d", result);
    return result;
}
#endif

#if defined(QMI_SWI_M2M_NV_RESTORE_REQ_V01)
//--------------------------------------------------------------------------------------------------
/**
 * Restore previously saved the modem configuration.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_FAULT          The function failed.
 *  - LE_UNSUPPORTED    The function does not supported on this platform.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdmCfg_RestoreSavedConfiguration
(
    void
)
{
    le_result_t result;
    qmi_client_error_type rc;

    SWIQMI_DECLARE_MESSAGE(swi_m2m_nv_restore_resp_msg_v01, mgsResp);

    // Send request to FW
    rc = qmi_client_send_msg_sync(MgsClient,
                                  QMI_SWI_M2M_NV_RESTORE_REQ_V01,
                                  NULL, 0,
                                  &mgsResp, sizeof(mgsResp),
                                  COMM_LONG_PLATFORM_TIMEOUT);

    result = swiQmi_CheckResponse(STRINGIZE_EXPAND (QMI_SWI_M2M_NV_RESTORE_RESP_V01),
                                  rc,
                                  mgsResp.resp.result,
                                  mgsResp.resp.error);

    if (LE_OK != result)
    {
        LE_ERROR("Error on NV Restore: rc %d, resp error %d", rc, mgsResp.resp.error);
    }
    else
    {
        LE_INFO("NV Restore: %hu stored, %hu restored, %hu deleted, %hu defaulted,"
                " %hu skipped, %hu failed, %hu fail info",
                mgsResp.stored, mgsResp.restored, mgsResp.deleted, mgsResp.defaulted,
                mgsResp.skipped, mgsResp.failed, mgsResp.fail_info);
    }

    LE_DEBUG("return %d", result);
    return result;
}
#endif

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the PA modem configuration module.
 *
 * @return
 *   - LE_FAULT         The function failed to initialize the PA modem configuration module.
 *   - LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdmCfg_Init
(
    void
)
{
    /* Initialize the required service */
    if (LE_OK != swiQmi_InitServices(QMI_SERVICE_MGS))
    {
        LE_ERROR("Could not initialize QMI M2M General Service.\n");
        return LE_FAULT;
    }

    MgsClient = swiQmi_GetClientHandle(QMI_SERVICE_MGS);
    if (NULL == MgsClient)
    {
        LE_ERROR("Error opening QMI M2M General Service.\n");
        return LE_FAULT;
    }

    return LE_OK;
}
