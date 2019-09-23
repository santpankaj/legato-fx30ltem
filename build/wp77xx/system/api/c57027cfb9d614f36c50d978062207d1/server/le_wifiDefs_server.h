

/*
 * ====================== WARNING ======================
 *
 * THE CONTENTS OF THIS FILE HAVE BEEN AUTO-GENERATED.
 * DO NOT MODIFY IN ANY WAY.
 *
 * ====================== WARNING ======================
 */


#ifndef LE_WIFIDEFS_INTERFACE_H_INCLUDE_GUARD
#define LE_WIFIDEFS_INTERFACE_H_INCLUDE_GUARD


#include "legato.h"





//--------------------------------------------------------------------------------------------------
/**
 * The maximum length of the textual representation of an IP address.
 */
//--------------------------------------------------------------------------------------------------
#define LE_WIFIDEFS_MAX_IP_LENGTH 15

//--------------------------------------------------------------------------------------------------
/**
 * The first WiFi channel.
 * @note This is only valid for 2,4 GHz WiFi frequencies.
 */
//--------------------------------------------------------------------------------------------------
#define LE_WIFIDEFS_MIN_CHANNEL_VALUE 1

//--------------------------------------------------------------------------------------------------
/**
 * The last WiFi channel.
 * @note This is only valid for 2,4 GHz WiFi frequencies.
 */
//--------------------------------------------------------------------------------------------------
#define LE_WIFIDEFS_MAX_CHANNEL_VALUE 14

//--------------------------------------------------------------------------------------------------
/**
 * The maximum length of the wep key.
 */
//--------------------------------------------------------------------------------------------------
#define LE_WIFIDEFS_MAX_WEPKEY_LENGTH 63

//--------------------------------------------------------------------------------------------------
/**
 * The maximum number of bytes of the wep key
 * One extra byte is added for the null character.
 */
//--------------------------------------------------------------------------------------------------
#define LE_WIFIDEFS_MAX_WEPKEY_BYTES 64

//--------------------------------------------------------------------------------------------------
/**
 * The minimum length of the pass-phrase used to generate PSK is 8 bytes. See 802.11-2007: H4.2.
 */
//--------------------------------------------------------------------------------------------------
#define LE_WIFIDEFS_MIN_PASSPHRASE_LENGTH 8

//--------------------------------------------------------------------------------------------------
/**
 * The maximum length of the pass-phrase used to generate PSK is 63 bytes. See 802.11-2007: H4.2.
 */
//--------------------------------------------------------------------------------------------------
#define LE_WIFIDEFS_MAX_PASSPHRASE_LENGTH 63

//--------------------------------------------------------------------------------------------------
/**
 * Maximum length of PSK, Pre Shared Key.
 * One extra byte is added for the null character.
 */
//--------------------------------------------------------------------------------------------------
#define LE_WIFIDEFS_MAX_PASSPHRASE_BYTES 64

//--------------------------------------------------------------------------------------------------
/**
 * The length of the PSK, Pre Shared Key, is 64 bytes. See 802.11-2007: H4.2.
 */
//--------------------------------------------------------------------------------------------------
#define LE_WIFIDEFS_MAX_PSK_LENGTH 64

//--------------------------------------------------------------------------------------------------
/**
 * Maximum length of PSK, Pre Shared Key.
 * One extra byte is added for the null character.
 */
//--------------------------------------------------------------------------------------------------
#define LE_WIFIDEFS_MAX_PSK_BYTES 65

//--------------------------------------------------------------------------------------------------
/**
 * Maximum length of the User-Name. RFC2865 recommends the ability to handle at least 63 octets.
 */
//--------------------------------------------------------------------------------------------------
#define LE_WIFIDEFS_MAX_USERNAME_LENGTH 63

//--------------------------------------------------------------------------------------------------
/**
 * Maximum length of the User-Name.
 * One extra byte is added for the null character.
 */
//--------------------------------------------------------------------------------------------------
#define LE_WIFIDEFS_MAX_USERNAME_BYTES 64

//--------------------------------------------------------------------------------------------------
/**
 * Maximum length of the User-Name. RFC2865 recommends at least 130.
 */
//--------------------------------------------------------------------------------------------------
#define LE_WIFIDEFS_MAX_PASSWORD_LENGTH 130

//--------------------------------------------------------------------------------------------------
/**
 * Maximum length of the User-Name.
 * One extra byte is added for the null character.
 */
//--------------------------------------------------------------------------------------------------
#define LE_WIFIDEFS_MAX_PASSWORD_BYTES 131

//--------------------------------------------------------------------------------------------------
/**
 * The minimum length of octets of the Service set identification (SSID).
 * @note While the values are probably human readable, this is not a string.
 */
//--------------------------------------------------------------------------------------------------
#define LE_WIFIDEFS_MIN_SSID_LENGTH 1

//--------------------------------------------------------------------------------------------------
/**
 * The maximum length of octets of the Service set identification (SSID).
 * @note While the values are probably human readable, this is not a string.
 */
//--------------------------------------------------------------------------------------------------
#define LE_WIFIDEFS_MAX_SSID_LENGTH 32

//--------------------------------------------------------------------------------------------------
/**
 * The maximum length of octets of the Service set identification (SSID).
 * @note While the values are probably human readable, this is not a string.
 */
//--------------------------------------------------------------------------------------------------
#define LE_WIFIDEFS_MAX_SSID_BYTES 33

//--------------------------------------------------------------------------------------------------
/**
 * The maximum length of octets of the Basic Service set identifier (BSSID). The length is derived
 * from 6 bytes represented as hexadecimal character string with bytes separated by colons.
 */
//--------------------------------------------------------------------------------------------------
#define LE_WIFIDEFS_MAX_BSSID_LENGTH 17

//--------------------------------------------------------------------------------------------------
/**
 * Maximum length of the Basic Service set identifier (BSSID).
 * One extra byte is added for the null character.
 */
//--------------------------------------------------------------------------------------------------
#define LE_WIFIDEFS_MAX_BSSID_BYTES 18

#endif // LE_WIFIDEFS_INTERFACE_H_INCLUDE_GUARD