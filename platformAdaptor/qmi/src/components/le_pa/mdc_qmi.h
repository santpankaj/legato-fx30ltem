/**
 * @file mdc_qmi.h
 *
 * The MDC QMI implementation's inter-module include file.  This file should not be used outside the
 * QMI modem services implementation.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#ifndef SRC_MDC_QMI_INCLUDE_GUARD
#define SRC_MDC_QMI_INCLUDE_GUARD


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the QMI MDC module.
 *
 * @return
 *      LE_OK if successful.
 *      LE_FAULT if unsuccessful.
 */
//--------------------------------------------------------------------------------------------------
le_result_t mdc_qmi_Init
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the default QMI WDS client handle
 *
 * @return
 *      The QMI WDS client handle
 */
//--------------------------------------------------------------------------------------------------
qmi_client_type mdc_qmi_GetDefaultWdsClient
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Release a wdsClient
 *
 */
//--------------------------------------------------------------------------------------------------
void mdc_qmi_ReleaseWdsClient
(
    qmi_client_type wdsClient
);

#endif // SRC_MDC_QMI_INCLUDE_GUARD
