/**
 * @file mrc_qmi.h
 *
 * The MRC QMI implementation's inter-module include file.  This file should not be used outside the
 * QMI modem services implementation.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#ifndef SRC_MRC_QMI_INCLUDE_GUARD
#define SRC_MRC_QMI_INCLUDE_GUARD


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the QMI MRC module.
 *
 * @return
 *      LE_OK if successful.
 *      LE_FAULT if unsuccessful.
 */
//--------------------------------------------------------------------------------------------------
le_result_t mrc_qmi_Init
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the Home Network Name information.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if the Home Network Name can't fit in nameStr
 *      - LE_FAULT on any other failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t mrc_qmi_GetHomeNetworkName
(
    char       *nameStr,               ///< [OUT] the home network Name
    size_t      nameStrSize            ///< [IN]  the nameStr size
);

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the Home Network MCC and MNC.
 *
 * @return
 *      - LE_OK on success
 *      - LE_NOT_FOUND if Home Network has not been provisioned
 *      - LE_FAULT for unexpected error
 */
//--------------------------------------------------------------------------------------------------
le_result_t mrc_qmi_GetHomeNetworkMccMnc
(
    uint16_t* mccPtr,             ///< the home network MCC
    uint16_t* mncPtr,             ///< the home network MNC
    bool*     isMncIncPcsDigitPtr ///< true if MNC is a three-digit value,
                                  ///  false if MNC is a two-digit value
);

#endif // SRC_MRC_QMI_INCLUDE_GUARD
