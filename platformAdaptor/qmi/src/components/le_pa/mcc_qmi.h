/**
 * @file mcc_qmi.h
 *
 * The MCC QMI implementation's inter-module include file.  This file should not be used outside the
 * QMI modem services implementation.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#ifndef SRC_MCC_QMI_INCLUDE_GUARD
#define SRC_MCC_QMI_INCLUDE_GUARD


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the QMI MCC module.
 *
 * @return
 *      LE_OK if successful.
 *      LE_FAULT if unsuccessful.
 */
//--------------------------------------------------------------------------------------------------
le_result_t mcc_qmi_Init
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * This function is called to inform pa_mcc that pa_ecall is requesting the end of the call.
 *
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t mcc_qmi_EcallStop
(
    void
);


#endif // SRC_MCC_QMI_INCLUDE_GUARD
