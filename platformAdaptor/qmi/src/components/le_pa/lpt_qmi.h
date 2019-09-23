/**
 * @file lpt_qmi.h
 *
 * The LPT QMI implementation's inter-module include file.
 * This file should not be used outside the QMI modem services implementation.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#ifndef SRC_LPT_QMI_INCLUDE_GUARD
#define SRC_LPT_QMI_INCLUDE_GUARD


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the QMI LPT module.
 *
 * @return
 *      LE_OK if successful.
 *      LE_FAULT if unsuccessful.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_lpt_Init
(
    void
);

#endif // SRC_LPT_QMI_INCLUDE_GUARD
