/**
 * @file info_qmi.h
 *
 * The INFO QMI implementation's inter-module include file. This file should not be used outside the
 * QMI modem services implementation.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#ifndef SRC_INFO_QMI_INCLUDE_GUARD
#define SRC_INFO_QMI_INCLUDE_GUARD


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the QMI INFO module.
 *
 * @return
 *      LE_OK if successful.
 *      LE_FAULT if unsuccessful.
 */
//--------------------------------------------------------------------------------------------------
le_result_t info_qmi_Init
(
    void
);


#endif // SRC_INFO_QMI_INCLUDE_GUARD
