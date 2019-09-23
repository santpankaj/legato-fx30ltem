/** @file pa_qmi_local.h
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#ifndef LEGATO_PAQMILOCAL_INCLUDE_GUARD
#define LEGATO_PAQMILOCAL_INCLUDE_GUARD

#include "interfaces.h"

//--------------------------------------------------------------------------------------------------
/**
 * Convert {mobileCountryCode,mobileNetworkCode} into string.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on any other failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t ConvertMccMncIntoStringFormat
(
    uint16_t    mcc,                        ///< [IN] the mcc to encode
    uint16_t    mnc,                        ///< [IN] the mnc to encode
    bool        isMncIncPcsDigit,           ///< [IN] true if MNC is a three-digit value,
                                            ///       false if MNC is a two-digit value
    char        mccPtr[LE_MRC_MCC_BYTES],   ///< [OUT] the mcc encoded
    char        mncPtr[LE_MRC_MNC_BYTES]    ///< [OUT] the mnc encoded
);

#endif // LEGATO_PAQMILOCAL_INCLUDE_GUARD
