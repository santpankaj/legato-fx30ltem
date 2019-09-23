/** @file pa_gnss.c
 *
 * QMI implementation of pa_gnss API.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "pa_gnss.h"
#include "swiQmi.h"
#include <math.h>

//--------------------------------------------------------------------------------------------------
/**
 * The acquisition rate in milliseconds.
 */
//--------------------------------------------------------------------------------------------------
static uint32_t AcqRate = 1000; // in milliseconds.

//--------------------------------------------------------------------------------------------------
/**
 * The DMS (Device Management Service) client.  Must be obtained by calling
 * swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type DmsClient;

//--------------------------------------------------------------------------------------------------
/**
 * The Location client.  Must be obtained by calling swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type LocClient;

//--------------------------------------------------------------------------------------------------
/**
 * The M2M General client.  Must be obtained by calling swiQmi_GetClientHandle() before it can be
 * used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type GeneralClient;

//--------------------------------------------------------------------------------------------------
/**
 * The SWI Location client.  Must be obtained by calling swiQmi_GetClientHandle() before it can be
 * used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type SwiLocClient;

//--------------------------------------------------------------------------------------------------
/**
 * Position event ID used to report position events to the registered event handlers.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t PositionEvent;

//--------------------------------------------------------------------------------------------------
/**
 * NMEA event ID used to report NMEA frames to the registered event handlers.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t NmeaEvent;

//--------------------------------------------------------------------------------------------------
/**
 * Memory pool for position event data.
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t PositionEventDataPool;

//--------------------------------------------------------------------------------------------------
/**
 * Memory pool for NMEA event data.
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t NmeaEventDataPool;

//--------------------------------------------------------------------------------------------------
/**
 * The computed position data.
 */
//--------------------------------------------------------------------------------------------------
static pa_Gnss_Position_t PositionData;

//--------------------------------------------------------------------------------------------------
/**
 * The configured SUPL assisted mode.
 */
//--------------------------------------------------------------------------------------------------
static le_gnss_AssistedMode_t SuplAssitedMode = LE_GNSS_STANDALONE_MODE;

//--------------------------------------------------------------------------------------------------
/**
 * NMEA handler counter
 */
//--------------------------------------------------------------------------------------------------
static uint8_t NmeaHandlerCounter;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for TTFF computation
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    le_clk_Time_t       acqStartTime;       ///< Acquisition start time
    le_clk_Time_t       ttff;               ///< TTFF value
    bool                firstFix;           ///< First Position fix status
} pa_gnss_Context_t;

//--------------------------------------------------------------------------------------------------
/**
 * GNSS context
 */
//--------------------------------------------------------------------------------------------------
static pa_gnss_Context_t GnssCtx;

#define SESSION_ID             0
#define START_REQUEST_PATTERN  0xFE
#define STOP_REQUEST_PATTERN   0xFF

//--------------------------------------------------------------------------------------------------
/**
 * Specific timeout for pa_gnss_Stop()
 *
 * The QMI platform might take up to a bit more than one minute to answer to the stop request:
 * a timeout of 70 seconds covers this case.
 */
//--------------------------------------------------------------------------------------------------
#define STOP_TIMEOUT_SEC     70
#define STOP_TIMEOUT_USEC    0

//--------------------------------------------------------------------------------------------------
/**
 * Generic timeout for restart of GNSS.
 */
//--------------------------------------------------------------------------------------------------
#define TIMEOUT_SEC         10
#define TIMEOUT_USEC        0

//--------------------------------------------------------------------------------------------------
/**
 * Waiting time before starting the GNSS in case of warm restart.
 */
//--------------------------------------------------------------------------------------------------
#define WAIT_BEFORE_WARM_START_SEC   10

//--------------------------------------------------------------------------------------------------
/**
 * Enumeration for time conversion
 */
//--------------------------------------------------------------------------------------------------
#define HOURS_TO_SEC           (3600)
#define SEC_TO_MSEC            (1000)


//--------------------------------------------------------------------------------------------------
/**
 * PRN to SV ID offset definitions for QMI interface
 *
 */
//--------------------------------------------------------------------------------------------------
#define SAT_ID_OFFSET_GPS             (0)
#define SAT_ID_OFFSET_GLONASS         (64)
#define SAT_ID_OFFSET_SBAS            (-87)
#define SAT_ID_OFFSET_GALILEO         (0)
#define SAT_ID_OFFSET_BEIDOU          (0)
#define SAT_ID_OFFSET_QZSS            (0)

//--------------------------------------------------------------------------------------------------
/**
 * Multiplying factor accuracy
 */
//--------------------------------------------------------------------------------------------------
#define ONE_DECIMAL_PLACE_ACCURACY   (10)
#define TWO_DECIMAL_PLACE_ACCURACY   (100)
#define THREE_DECIMAL_PLACE_ACCURACY (1000)
#define SIX_DECIMAL_PLACE_ACCURACY   (1000000)

//--------------------------------------------------------------------------------------------------
/**
 * Define the maximum length of the Satellites Vehicle information list providing DGNSS correction.
 */
//--------------------------------------------------------------------------------------------------
#define SV_DGNSS_MAX_LEN   3

//--------------------------------------------------------------------------------------------------
/**
 * Define the indexes for velocity uncertainty.
 */
//--------------------------------------------------------------------------------------------------
#define EAST_VELOCITY      0
#define NORTH_VELOCITY     1
#define UP_VELOCITY        2

//--------------------------------------------------------------------------------------------------
/**
 * Enumeration to synchronize QMI LOC API
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    PA_GNSS_APISYNC_QMI_LOC_INJECT_PREDICTED_ORBITS_DATA_REQ_V02 = 0,
    ///< QMI_LOC_INJECT_PREDICTED_ORBITS_DATA_REQ_V02
    PA_GNSS_APISYNC_QMI_LOC_GET_PREDICTED_ORBITS_DATA_VALIDITY_REQ_V02,
    ///< QMI_LOC_GET_PREDICTED_ORBITS_DATA_VALIDITY_REQ_V02
    PA_GNSS_APISYNC_QMI_LOC_SET_OPERATION_MODE_REQ_V02,
    ///< QMI_LOC_SET_OPERATION_MODE_REQ_V02
    PA_GNSS_APISYNC_QMI_LOC_SET_SERVER_REQ_V02,
    ///< QMI_LOC_SET_SERVER_REQ_V02
    PA_GNSS_APISYNC_QMI_LOC_INJECT_SUPL_CERTIFICATE_REQ_V02,
    ///< QMI_LOC_INJECT_SUPL_CERTIFICATE_REQ_V02
    PA_GNSS_APISYNC_QMI_LOC_DELETE_SUPL_CERTIFICATE_REQ_V02,
    ///< QMI_LOC_DELETE_SUPL_CERTIFICATE_REQ_V02
    PA_GNSS_APISYNC_QMI_LOC_SET_GNSS_CONSTELL_REPORT_CONFIG_V02,
    ///< QMI_LOC_SET_GNSS_CONSTELL_REPORT_CONFIG_V02
    PA_GNSS_APISYNC_QMI_LOC_GET_REGISTERED_EVENTS_REQ_V02,
    ///< QMI_LOC_GET_REGISTERED_EVENTS_REQ_V02
    PA_GNSS_APISYNC_QMI_LOC_EVENT_ENGINE_STATE_IND_V02,
    ///< QMI_LOC_EVENT_ENGINE_STATE_IND_V02
    PA_GNSS_APISYNC_QMI_LOC_SET_NMEA_TYPES_REQ_V02,
    ///< QMI_LOC_SET_NMEA_TYPES_REQ_V02
    PA_GNSS_APISYNC_QMI_LOC_GET_NMEA_TYPES_REQ_V02,
    ///< QMI_LOC_GET_NMEA_TYPES_REQ_V02
    PA_GNSS_APISYNC_QMI_LOC_INJECT_UTC_TIME_REQ_V02,
    ///< QMI_LOC_INJECT_UTC_TIME_REQ_V02
    PA_GNSS_APISYNC_MAX ///< Do not use
}
pa_gnss_ApiSync_t;


//--------------------------------------------------------------------------------------------------
/**
 * Structure to synchronize Qualcomm Location API
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    bool                      commandInProgress;   ///< a command is in progress
    pthread_mutex_t            mutexPosix;         ///< Mutex to prevent race condition with
                                                   ///commandInProgress
    // Cannot use le_mutex_Ref_t because of non managing le_mutex non-legato threads.
    // TODO: use le_mutex_Ref_t when non-legato threads will be manage
    le_sem_Ref_t               semaphoreRef;       ///< Semaphore to synchronize commandInProgress
    le_sem_Ref_t               semaphoreXtra;      ///< Semaphore to synchronize Xtra upload and
                                                   ///injection
    void                       *dataPtr;           ///< any data
    le_result_t                result;             ///< Result of the async handler
    qmiLocEngineStateEnumT_v02 gnssEngineState;    ///< GNSS engine state for engine status handler
    qmiLocEngineStateEnumT_v02 engineState;        ///< Engine state for gnss start/stop/restart
}
pa_gnss_LocSync_t;

//--------------------------------------------------------------------------------------------------
/**
 * Semaphore to synchronize XTRA data injection QMI call
 */
//--------------------------------------------------------------------------------------------------
static pa_gnss_LocSync_t FunctionSync[PA_GNSS_APISYNC_MAX];

//--------------------------------------------------------------------------------------------------
/**
 * Inline function to convert and round to the nearest
 *
 * The function firstly converts the double value to int according to the requested place after the
 * decimal given by place parameter. Secondly, a round to the nearest is done the int value.
 */
//--------------------------------------------------------------------------------------------------
static inline int32_t ConvertAndRoundToNearest
(
    double value,    ///< [IN] value to round to the nearest
    int32_t place    ///< [IN] the place after the decimal in power of 10
)
{
    return (int32_t)((int64_t)((place*value*10) + ((value > 0) ? 5 : -5))/10);
}

//--------------------------------------------------------------------------------------------------
/**
 * Start a synchronize command
 */
//--------------------------------------------------------------------------------------------------
static void StartCommand
(
    pa_gnss_LocSync_t* locSyncPtr
)
{
    LE_FATAL_IF((pthread_mutex_lock(&locSyncPtr->mutexPosix)!=0),"Could not lock the mutex");
    locSyncPtr->commandInProgress = true;
    LE_FATAL_IF((pthread_mutex_unlock(&locSyncPtr->mutexPosix)!=0),"Could not unlock the mutex");
}

//--------------------------------------------------------------------------------------------------
/**
 * Stop a synchronize command
 */
//--------------------------------------------------------------------------------------------------
static void StopCommand
(
    pa_gnss_LocSync_t* locSyncPtr
)
{
    LE_FATAL_IF((pthread_mutex_lock(&locSyncPtr->mutexPosix)!=0),"Could not lock the mutex");
    locSyncPtr->commandInProgress = false;
    LE_FATAL_IF((pthread_mutex_unlock(&locSyncPtr->mutexPosix)!=0),"Could not unlock the mutex");
}

//--------------------------------------------------------------------------------------------------
/**
 * Finish the synchronize command
 */
//--------------------------------------------------------------------------------------------------
static void SendCommandResult
(
    pa_gnss_LocSync_t* locSyncPtr,
    le_result_t        result
)
{
    LE_FATAL_IF((pthread_mutex_lock(&locSyncPtr->mutexPosix)!=0),"Could not lock the mutex");
    if (locSyncPtr->commandInProgress)
    {
        locSyncPtr->result = result;
        locSyncPtr->commandInProgress = false;
        if (locSyncPtr->semaphoreRef != NULL)
        {
            le_sem_Post(locSyncPtr->semaphoreRef);
        }
    }
    LE_FATAL_IF((pthread_mutex_unlock(&locSyncPtr->mutexPosix)!=0),"Could not unlock the mutex");
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize position valid data flags to false and satellites measurement structure
 */
//--------------------------------------------------------------------------------------------------
static void InitializeValidGnssPosition
(
    pa_Gnss_Position_t* posDataPtr  // [IN/OUT] Pointer to the position data.
)
{
    int i;
    posDataPtr->fixState = LE_GNSS_STATE_FIX_NO_POS;
    posDataPtr->altitudeValid = false;
    posDataPtr->altitudeAssumedValid = false;
    posDataPtr->altitudeOnWgs84Valid = false;
    posDataPtr->dateValid = false;
    posDataPtr->hdopValid = false;
    posDataPtr->hSpeedUncertaintyValid = false;
    posDataPtr->hSpeedValid = false;
    posDataPtr->hUncertaintyValid = false;
    posDataPtr->latitudeValid = false;
    posDataPtr->longitudeValid = false;
    posDataPtr->timeValid = false;
    posDataPtr->gpsTimeValid = false;
    posDataPtr->timeAccuracyValid = false;
    posDataPtr->positionLatencyValid = false;
    posDataPtr->directionUncertaintyValid = false;
    posDataPtr->directionValid = false;
    posDataPtr->vdopValid = false;
    posDataPtr->vSpeedUncertaintyValid = false;
    posDataPtr->vSpeedValid = false;
    posDataPtr->vUncertaintyValid = false;
    posDataPtr->pdopValid = false;
    posDataPtr->satMeasValid = false;
    posDataPtr->leapSecondsValid = false;
    posDataPtr->gdopValid = false;
    posDataPtr->tdopValid = false;

    for (i=0; i<LE_GNSS_SV_INFO_MAX_LEN; i++)
    {
        posDataPtr->satMeas[i].satId = 0;
        posDataPtr->satMeas[i].satLatency = 0;
    }
 }


//--------------------------------------------------------------------------------------------------
/**
 * Initialize satellites info that are updated in SV information report indication.
 */
//--------------------------------------------------------------------------------------------------
static void InitializeSatInfo
(
    pa_Gnss_Position_t* posDataPtr  // [IN/OUT] Pointer to the position data.
)
{
    int i;
    posDataPtr->satsInViewCountValid = false;
    posDataPtr->satsTrackingCountValid = false;
    posDataPtr->satInfoValid = false;
    posDataPtr->magneticDeviationValid = false;

    for (i=0; i<LE_GNSS_SV_INFO_MAX_LEN; i++)
    {
        posDataPtr->satInfo[i].satId = 0;
        posDataPtr->satInfo[i].satConst = LE_GNSS_SV_CONSTELLATION_UNDEFINED;
        posDataPtr->satInfo[i].satTracked = 0;
        posDataPtr->satInfo[i].satSnr = 0;
        posDataPtr->satInfo[i].satAzim = 0;
        posDataPtr->satInfo[i].satElev = 0;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize satellites info that are updated as "used" in SV information report indication.
 */
//--------------------------------------------------------------------------------------------------
static void InitializeSatUsedInfo
(
    pa_Gnss_Position_t* posDataPtr  // [IN/OUT] Pointer to the position data.
)
{
    int i;
    // Reset the SV marked as used in position data list.
    posDataPtr->satsUsedCountValid = false;
    posDataPtr->satsUsedCount = 0;
    for (i=0; i<LE_GNSS_SV_INFO_MAX_LEN; i++)
    {
        posDataPtr->satInfo[i].satUsed = false;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize position valid data flags to false and satellites measurement structure
 */
//--------------------------------------------------------------------------------------------------
static void InitializeDataPosition
(
    pa_Gnss_Position_t* posDataPtr  // [IN/OUT] Pointer to the position data.
)
{
    InitializeValidGnssPosition(posDataPtr);
    InitializeSatInfo(posDataPtr);
    InitializeSatUsedInfo(posDataPtr);
}

//--------------------------------------------------------------------------------------------------
/**
 * Fill in the position data by parsing a position report.
 */
//--------------------------------------------------------------------------------------------------
static void GetPositionDataFromReport
(
    const qmiLocEventPositionReportIndMsgT_v02* posReportPtr, // [IN] The position report.
    pa_Gnss_Position_t* posDataPtr                            // [OUT] Pointer to the position data.
)
{
    if (posReportPtr->timestampUtc_valid)
    {
        struct tm brokenTime;
        time_t utcTimestamp = posReportPtr->timestampUtc / 1000; // timestampUtc is in milliseconds
        posDataPtr->epochTime = posReportPtr->timestampUtc;

        if (gmtime_r(&utcTimestamp, &brokenTime) != 0)
        {
            posDataPtr->time.hours = brokenTime.tm_hour;
            posDataPtr->time.minutes = brokenTime.tm_min;
            posDataPtr->time.seconds = brokenTime.tm_sec;
            posDataPtr->time.milliseconds = posReportPtr->timestampUtc % 1000;
            posDataPtr->timeValid = true;

            posDataPtr->date.year = brokenTime.tm_year + 1900;  // Convert to current year.
            posDataPtr->date.month = brokenTime.tm_mon + 1;  // Convert to months with Jan == 1.
            posDataPtr->date.day = brokenTime.tm_mday;
            posDataPtr->dateValid = true;
        }
    }

    if (posReportPtr->gpsTime_valid)
    {
        posDataPtr->gpsWeek = posReportPtr->gpsTime.gpsWeek;
        posDataPtr->gpsTimeOfWeek = posReportPtr->gpsTime.gpsTimeOfWeekMs;
        posDataPtr->gpsTimeValid = true;
    }

    if (posReportPtr->timeUnc_valid)
    {
        // If timeUnc is beyond the uint32_t limit, then convert it to UINT32_MAX
        if ((uint64_t)(SIX_DECIMAL_PLACE_ACCURACY*(posReportPtr->timeUnc)) >= UINT32_MAX)
        {
            LE_DEBUG("Time accuracy is beyond the limit");
            posDataPtr->timeAccuracy = UINT32_MAX;
        }
        else
        {
            posDataPtr->timeAccuracy = ConvertAndRoundToNearest(posReportPtr->timeUnc,
                                                                SIX_DECIMAL_PLACE_ACCURACY);

            LE_DEBUG("posReportPtr->timeUnc %f, posDataPtr->timeAccuracy %d",
                     posReportPtr->timeUnc, posDataPtr->timeAccuracy);
        }
        posDataPtr->timeAccuracyValid = true;
    }

    if (posReportPtr->altitudeWrtMeanSeaLevel_valid)
    {
        posDataPtr->altitude =
            ConvertAndRoundToNearest(posReportPtr->altitudeWrtMeanSeaLevel,
                                     THREE_DECIMAL_PLACE_ACCURACY);
        posDataPtr->altitudeValid = true;

        LE_DEBUG("posReportPtr->altitude %f, posDataPtr->altitude %d",
                  posReportPtr->altitudeWrtMeanSeaLevel,
                  posDataPtr->altitude);
    }

    if (posReportPtr->altitudeWrtEllipsoid_valid)
    {
        posDataPtr->altitudeOnWgs84 =
            ConvertAndRoundToNearest(posReportPtr->altitudeWrtEllipsoid,
                                     THREE_DECIMAL_PLACE_ACCURACY);
        posDataPtr->altitudeOnWgs84Valid = true;

        LE_DEBUG("posReportPtr->altitudeWrt %f, posDataPtr->altitudeWrt %d",
                  posReportPtr->altitudeWrtEllipsoid,
                  posDataPtr->altitudeOnWgs84);
    }

    LE_DEBUG("altitudeAssumedValid %d, altitudeAssumed %d",
              posReportPtr->altitudeAssumed_valid, posReportPtr->altitudeAssumed);

    if (posReportPtr->altitudeAssumed_valid)
    {
        posDataPtr->altitudeAssumed = posReportPtr->altitudeAssumed;
        posDataPtr->altitudeAssumedValid = true;
    }

    // TODO: This DOP block may deprecated in the future. Use extDOP block instead.
    if ((eQMI_LOC_SESS_STATUS_SUCCESS_V02 == posReportPtr->sessionStatus) &&
        posReportPtr->DOP_valid)
    {
        posDataPtr->hdop = ConvertAndRoundToNearest(posReportPtr->DOP.HDOP,
                                                    THREE_DECIMAL_PLACE_ACCURACY);
        posDataPtr->vdop = ConvertAndRoundToNearest(posReportPtr->DOP.VDOP,
                                                    THREE_DECIMAL_PLACE_ACCURACY);
        posDataPtr->pdop = ConvertAndRoundToNearest(posReportPtr->DOP.PDOP,
                                                    THREE_DECIMAL_PLACE_ACCURACY);

        posDataPtr->hdopValid = posDataPtr->vdopValid = posDataPtr->pdopValid = true;

        LE_DEBUG("posReportPtr->HDOP %f, posReportPtr->VDOP %f, posReportPtr->PDOP %f",
                  posReportPtr->DOP.HDOP,
                  posReportPtr->DOP.VDOP,
                  posReportPtr->DOP.PDOP);

        LE_DEBUG("posDataPtr->HDOP %d, posReportPtr->VDOP %d, posReportPtr->PDOP %d",
                        posDataPtr->hdop, posDataPtr->vdop, posDataPtr->pdop);

    }

#if (LOC_V02_IDL_MINOR_VERS >= 0x45)
    if ((eQMI_LOC_SESS_STATUS_SUCCESS_V02 == posReportPtr->sessionStatus) &&
        posReportPtr->extDOP_valid)
    {
        posDataPtr->tdop = ConvertAndRoundToNearest(posReportPtr->extDOP.TDOP,
                                                    THREE_DECIMAL_PLACE_ACCURACY);
        posDataPtr->gdop = ConvertAndRoundToNearest(posReportPtr->extDOP.GDOP,
                                                    THREE_DECIMAL_PLACE_ACCURACY);

        posDataPtr->gdopValid = posDataPtr->tdopValid = true;

        LE_DEBUG("posReportPtr->TDOP %f, posReportPtr->GDOP %f",
                  posReportPtr->extDOP.TDOP,
                  posReportPtr->extDOP.GDOP);

        LE_DEBUG("posDataPtr->TDOP %d, posReportPtr->GDOP %d",
                        posDataPtr->tdop, posDataPtr->gdop);

    }
#endif

    if (posReportPtr->latitude_valid)
    {
        posDataPtr->latitude = ConvertAndRoundToNearest(posReportPtr->latitude,
                                                        SIX_DECIMAL_PLACE_ACCURACY);
        posDataPtr->latitudeValid = true;

        LE_DEBUG("posReportPtr->latitude %f, posDataPtr->latitude %d",
                  posReportPtr->latitude, posDataPtr->latitude);
    }

    if (posReportPtr->longitude_valid)
    {
        posDataPtr->longitude = ConvertAndRoundToNearest(posReportPtr->longitude,
                                                         SIX_DECIMAL_PLACE_ACCURACY);
        posDataPtr->longitudeValid = true;

        LE_DEBUG("posReportPtr->longitude %f, posDataPtr->longitude %d",
                     posReportPtr->longitude, posDataPtr->longitude);
    }

    if (posReportPtr->speedHorizontal_valid)
    {
        posDataPtr->hSpeed = ConvertAndRoundToNearest(posReportPtr->speedHorizontal,
                                                      TWO_DECIMAL_PLACE_ACCURACY);
        posDataPtr->hSpeedValid = true;
    }

    if (posReportPtr->speedVertical_valid)
    {
        posDataPtr->vSpeed = ConvertAndRoundToNearest(posReportPtr->speedVertical,
                                                      TWO_DECIMAL_PLACE_ACCURACY);
        posDataPtr->vSpeedValid = true;
    }

#if (LOC_V02_IDL_MINOR_VERS >= 0x40)
    // Horizontal and vertical speed uncertainty can be calculated with the velocity uncertainty
    // information for ENU (East, North and Up) direction (Units: Meters/second).
    // Horizontal speed uncertainty = sqrt(East^2 + North^2)
    // Vertical speed uncertainty = Up
    if (posReportPtr->velUncEnu_valid)
    {
        float eastVelocity =  posReportPtr->velUncEnu[EAST_VELOCITY];
        float northVelocity = posReportPtr->velUncEnu[NORTH_VELOCITY];

        float hSpeedUncertainty = sqrtf(eastVelocity*eastVelocity + northVelocity*northVelocity);

        posDataPtr->hSpeedUncertainty = ConvertAndRoundToNearest(hSpeedUncertainty,
                                                                 THREE_DECIMAL_PLACE_ACCURACY);
        posDataPtr->vSpeedUncertainty = ConvertAndRoundToNearest(
                                                               posReportPtr->velUncEnu[UP_VELOCITY],
                                                               THREE_DECIMAL_PLACE_ACCURACY);

        posDataPtr->hSpeedUncertaintyValid = true;
        posDataPtr->vSpeedUncertaintyValid = true;

        LE_DEBUG("Report: eastVelocity %f, northVelocity %f, upVelocity %f",
                 posReportPtr->velUncEnu[EAST_VELOCITY],
                 posReportPtr->velUncEnu[NORTH_VELOCITY],
                 posReportPtr->velUncEnu[UP_VELOCITY]);

        LE_DEBUG("float hSpeedUncertainty %f, hSpeedUncertainty %d, vSpeedUncertainty %d",
                 hSpeedUncertainty,
                 posDataPtr->hSpeedUncertainty,
                 posDataPtr->vSpeedUncertainty);
    }
#else
    if (posReportPtr->speedUnc_valid)
    {
        posDataPtr->hSpeedUncertainty = ConvertAndRoundToNearest(posReportPtr->speedUnc,
                                                                 THREE_DECIMAL_PLACE_ACCURACY);
        posDataPtr->vSpeedUncertainty = ConvertAndRoundToNearest(posReportPtr->speedUnc,
                                                                 THREE_DECIMAL_PLACE_ACCURACY);
        posDataPtr->hSpeedUncertaintyValid = true;
        posDataPtr->vSpeedUncertaintyValid = true;
    }
#endif

    if (posReportPtr->heading_valid)
    {
        posDataPtr->direction = ConvertAndRoundToNearest(posReportPtr->heading,
                                                         ONE_DECIMAL_PLACE_ACCURACY);
        posDataPtr->directionValid = true;
    }

    if (posReportPtr->headingUnc_valid)
    {
        posDataPtr->directionUncertainty = ConvertAndRoundToNearest(posReportPtr->headingUnc,
                                                                    ONE_DECIMAL_PLACE_ACCURACY);
        posDataPtr->directionUncertaintyValid = true;
    }

    if (posReportPtr->horUncCircular_valid)
    {
        posDataPtr->hUncertainty = ConvertAndRoundToNearest(posReportPtr->horUncCircular,
                                                            TWO_DECIMAL_PLACE_ACCURACY);
        posDataPtr->hUncertaintyValid = true;

        LE_DEBUG("posReportPtr->horUncCircular %f, posDataPtr->hUncertainty %d",
                     posReportPtr->horUncCircular, posDataPtr->hUncertainty);
    }

    if (posReportPtr->vertUnc_valid)
    {
        posDataPtr->vUncertainty = ConvertAndRoundToNearest(posReportPtr->vertUnc,
                                                            THREE_DECIMAL_PLACE_ACCURACY);
        posDataPtr->vUncertaintyValid = true;

        LE_DEBUG("posReportPtr->vertUnc %f, posDataPtr->hUncertainty %d",
                     posReportPtr->vertUnc, posDataPtr->vUncertainty);
    }

    if (posReportPtr->magneticDeviation_valid)
    {
        posDataPtr->magneticDeviation = ConvertAndRoundToNearest(posReportPtr->magneticDeviation,
                                                                 ONE_DECIMAL_PLACE_ACCURACY);
        posDataPtr->magneticDeviationValid = true;
    }

    if (posReportPtr->leapSeconds_valid)
    {
        posDataPtr->leapSeconds = posReportPtr->leapSeconds;
        posDataPtr->leapSecondsValid = true;
    }

    if ((posReportPtr->timeSrc_valid) &&
        ((eQMI_LOC_TIME_SRC_UNKNOWN_V02 == posReportPtr->timeSrc) ||
        (eQMI_LOC_TIME_SRC_INVALID_V02 == posReportPtr->timeSrc)))
    {
        LE_DEBUG("timeSrc unknown or invalid");
        posDataPtr->timeValid = false;
        posDataPtr->dateValid = false;
        posDataPtr->gpsTimeValid = false;
        posDataPtr->timeAccuracyValid = false;
    }

    // Initialise the SV used table while the fix calculation is not available in GNSS engine
    if (eQMI_LOC_SESS_STATUS_IN_PROGRESS_V02 == posReportPtr->sessionStatus)
    {
        InitializeSatUsedInfo(posDataPtr);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Find the SV id in the computed position data list.
 */
//--------------------------------------------------------------------------------------------------
static bool UsedSvIdFound
(
    int usedSvId,                   // [IN] Satellite Id used to establish a fix.
    pa_Gnss_Position_t* posDataPtr  // [IN/OUT] Pointer to the position data.
)
{
    int j;
    LE_DEBUG("Sv used: %d",usedSvId);

    // Find this SV in the computed position data list.
    // The SVs in the position data list correspond to SVs given in the last SV information
    // report indication. They are the SV viewed and tracked for the navigation.
    for (j=0; j<LE_GNSS_SV_INFO_MAX_LEN; j++)
    {
        if (usedSvId == posDataPtr->satInfo[j].satId)
        {
            LE_DEBUG("Sv used: %d found at satInfo list index: %d",usedSvId,j);

            if (false == posDataPtr->satInfo[j].satTracked)
            {
                LE_DEBUG("Satellite %d is used without being tracked", usedSvId);
            }
            // Set this SV as "used" even if it was not saved as "tracked" in the last
            // position data list.
            posDataPtr->satInfo[j].satUsed = true;
            return true;
        }
    }
    // Do not set this SV as "used".
    if (LE_GNSS_SV_INFO_MAX_LEN == j)
    {
        LE_DEBUG("Satellite %d is not viewed, don't use it for a Fix", usedSvId);
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/**
 * Fill in the position data by parsing a position report and updates the satellites used to
 * establish a fix.
 */
//--------------------------------------------------------------------------------------------------
static void GetUsedSvFromPositionData
(
    const qmiLocEventPositionReportIndMsgT_v02* posReportPtr, // [IN] The position report.
    pa_Gnss_Position_t* posDataPtr                            // [OUT] Pointer to the position data.
)
{
    int i;
    uint8_t satsUsedCount = 0;
    uint32_t svListlen;

    // Reset the SV marked as used in position data list.
    InitializeSatUsedInfo(posDataPtr);

    if (false == posReportPtr->gnssSvUsedList_valid)
    {
        LE_DEBUG("Invalid gnssSvUsedList");
        return;
    }

    // Find the satellite ID used for the fix.
    if (posReportPtr->gnssSvUsedList_len > LE_GNSS_SV_INFO_MAX_LEN)
    {
        LE_WARN("SV list size %d not expected",posReportPtr->gnssSvUsedList_len);
        svListlen = LE_GNSS_SV_INFO_MAX_LEN;
    }
    else
    {
        svListlen = posReportPtr->gnssSvUsedList_len;
    }

    for (i=0; i<svListlen; i++)
    {
        // Get the SV used to calculate the fix from the position report.
        if (UsedSvIdFound(posReportPtr->gnssSvUsedList[i], posDataPtr))
        {
            satsUsedCount++;
        }
    }

    posDataPtr->satsUsedCountValid = true;
    posDataPtr->satsUsedCount = satsUsedCount;
    LE_DEBUG("Sv list size: %d, Sv used: %d, Total Used %d",svListlen,
             satsUsedCount,
             posDataPtr->satsUsedCount);
}

//--------------------------------------------------------------------------------------------------
/**
 * Fill in the position data by parsing a position report and updates the satellites providing DGNSS
 * correction used to establish a fix.
 */
//--------------------------------------------------------------------------------------------------
static void GetDgnssUsedSvFromPositionData
(
    const qmiLocEventPositionReportIndMsgT_v02* posReportPtr, // [IN] The position report.
    pa_Gnss_Position_t* posDataPtr                            // [OUT] Pointer to the position data.
)
{
#if defined(QMI_LOC_DGNSS_STATION_ID_ARRAY_LENGTH_V02)
    int i;
    uint8_t satsUsedCount = 0;
    uint32_t svListlen;

    if (false == posReportPtr->dgnssStationId_valid)
    {
        LE_DEBUG("No Sv providing DGNSS correction");
        return;
    }

    // Find the satellite ID providing DGNSS correction used for the fix.
    if (posReportPtr->dgnssStationId_len > SV_DGNSS_MAX_LEN)
    {
        LE_WARN("SV DGNSS list size %d not expected",posReportPtr->dgnssStationId_len);
        svListlen = SV_DGNSS_MAX_LEN;
    }
    else
    {
        svListlen = posReportPtr->dgnssStationId_len;
    }

    LE_DEBUG("Find Sv used from DGNSS list");

    for (i=0; i<svListlen; i++)
    {
        // Get the SV used to calculate the fix from the position report.
        if (UsedSvIdFound(posReportPtr->dgnssStationId[i]+ SAT_ID_OFFSET_SBAS, posDataPtr))
        {
            satsUsedCount++;
        }
    }

    posDataPtr->satsUsedCountValid = true;
    posDataPtr->satsUsedCount += satsUsedCount;
    LE_DEBUG("Sv DGNSS list size: %d, Sv used: %d, Total Used %d",svListlen,
             satsUsedCount,
             posDataPtr->satsUsedCount);
#endif
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler function called for satellites measurement report settings.
 */
//--------------------------------------------------------------------------------------------------
static void SetSatMeasurementReportHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    if (indBufPtr == NULL)
    {
        LE_ERROR("Indication message empty.");
        return;
    }

    pa_gnss_LocSync_t *locSyncPtr = &FunctionSync[
                                    PA_GNSS_APISYNC_QMI_LOC_SET_GNSS_CONSTELL_REPORT_CONFIG_V02];
    qmiLocSetGNSSConstRepConfigIndMsgT_v02 *constRepConfigIndPtr = locSyncPtr->dataPtr;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        LocClient, QMI_IDL_INDICATION, QMI_LOC_SET_GNSS_CONSTELL_REPORT_CONFIG_IND_V02,
        indBufPtr, indBufLen,
        constRepConfigIndPtr, sizeof(*constRepConfigIndPtr));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_LOC_SET_GNSS_CONSTELL_REPORT_CONFIG_IND_V02, error = %i",
                 clientMsgErr);
        SendCommandResult(locSyncPtr,LE_FAULT);
        return;
    }

    LE_DEBUG("QMI_LOC_SET_GNSS_CONSTELL_REPORT_CONFIG_IND_V02 status %d"
            ,constRepConfigIndPtr->status);

    SendCommandResult(locSyncPtr,LE_OK);
}


//--------------------------------------------------------------------------------------------------
/**
 * Activate Satellites measurement report.
 *
 * @return
 *      LE_OK if successful.
 *      LE_FAULT if there was an error.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SetSatMeasurementReport
(
    void
)
{
    le_result_t result = LE_OK;
    SWIQMI_DECLARE_MESSAGE(qmiLocSetGNSSConstRepConfigReqMsgT_v02, constRepConfigReq);
    SWIQMI_DECLARE_MESSAGE(qmiLocGenRespMsgT_v02, genResp);
    SWIQMI_DECLARE_MESSAGE(qmiLocSetGNSSConstRepConfigIndMsgT_v02, constRepConfigReqInd);
    pa_gnss_LocSync_t *locSyncPtr = &FunctionSync
                                    [PA_GNSS_APISYNC_QMI_LOC_SET_GNSS_CONSTELL_REPORT_CONFIG_V02];

    // Set message contents
    constRepConfigReq.measReportConfig_valid = true;
    constRepConfigReq.measReportConfig =  eQMI_SYSTEM_GPS_V02
                                        | eQMI_SYSTEM_GLO_V02
                                        | eQMI_SYSTEM_BDS_V02
                                        | eQMI_SYSTEM_GAL_V02;

    // Set buffer to fill by the handler
    locSyncPtr->dataPtr = &constRepConfigReqInd;
    // Start the command
    StartCommand(locSyncPtr);

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(LocClient,
                                         QMI_LOC_SET_GNSS_CONSTELL_REPORT_CONFIG_V02,
                                         &constRepConfigReq, sizeof(constRepConfigReq),
                                         &genResp, sizeof(genResp),
                                         COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_LOC_SET_GNSS_CONSTELL_REPORT_CONFIG_RESP_V02),
                                 clientMsgErr,
                                 genResp.resp) == LE_OK)
    {
        // Wait QMI notification
        le_clk_Time_t timer = { .sec= TIMEOUT_SEC, .usec= TIMEOUT_USEC };
        le_result_t semResult = le_sem_WaitWithTimeOut(locSyncPtr->semaphoreRef,timer);

        // Command is finished
        StopCommand(locSyncPtr);

        // Check le_sem_WaitUntil timeOut
        if (semResult != LE_OK)
        {
            LE_WARN("Timeout happen [%d]", semResult);
            result = LE_TIMEOUT;
        }
        else if (locSyncPtr->result != LE_OK)
        {
            LE_ERROR("Error for QMI_LOC_SET_GNSS_CONSTELL_REPORT_CONFIG_IND_V02, error = %i"
                    , locSyncPtr->result);
            result = LE_FAULT;
        }
        else if (constRepConfigReqInd.status == eQMI_LOC_SUCCESS_V02)
        {
            LE_DEBUG("Measurement report activated");
            result = LE_OK;
        }
    }
    else
    {
        StopCommand(locSyncPtr);
        LE_ERROR("Error for QMI_LOC_SET_GNSS_CONSTELL_REPORT_CONFIG_RESP_V02");
        result = LE_FAULT;
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Fill in the satellites information by parsing a report.
 */
//--------------------------------------------------------------------------------------------------
static void GetSatInfoFromReport
(
    const qmiLocEventGnssSvInfoIndMsgT_v02* svInfoPtr,  // [IN] The position report.
    pa_Gnss_Position_t* posDataPtr                      // [OUT] Pointer to the position data.
)
{
    uint32_t svListlen;
    LE_DEBUG("SV info valid %d",svInfoPtr->svList_valid);

    // Initialize the satellites vehicles information
    InitializeSatInfo(posDataPtr);

    if (false == svInfoPtr->svList_valid)
    {
        LE_DEBUG("Empty satellites information list !");
        return;
    }

    if (svInfoPtr->svList_len > LE_GNSS_SV_INFO_MAX_LEN)
    {
        LE_WARN("SV list size %d not expected",svInfoPtr->svList_len);
        svListlen = LE_GNSS_SV_INFO_MAX_LEN;
    }
    else
    {
        svListlen = svInfoPtr->svList_len;
    }
    LE_DEBUG("SV info Len %d", svListlen);

    int i = 0;
    uint8_t satsInViewCount = 0;
    uint8_t satsTrackingCount = 0;

    // Update the satellites vehicles information
    posDataPtr->satInfoValid = true;
    for (i=0; i<svListlen; i++)
    {
        if (0 != svInfoPtr->svList[i].gnssSvId)
        {
            LE_DEBUG("[%02d] SV %03d C%02d - status %d- SNR%02.2f - Azim%03.2f - Elev%02.2f",
                     i,
                     svInfoPtr->svList[i].gnssSvId,
                     svInfoPtr->svList[i].system,
                     svInfoPtr->svList[i].svStatus,
                     svInfoPtr->svList[i].snr,
                     svInfoPtr->svList[i].azimuth,
                     svInfoPtr->svList[i].elevation);

            // Satellites in View count
            satsInViewCount++;

            // Satellites in View Signal To Noise Ratio
            posDataPtr->satInfo[i].satSnr= svInfoPtr->svList[i].snr;
            // Satellites in View Azimuth
            posDataPtr->satInfo[i].satAzim= svInfoPtr->svList[i].azimuth;
            // Satellites in View Elevation
            posDataPtr->satInfo[i].satElev= svInfoPtr->svList[i].elevation;

            // Set GNSS Constellation and compute Satellite ID if required
            switch(svInfoPtr->svList[i].system)
            {
                case eQMI_LOC_SV_SYSTEM_GPS_V02:
                    posDataPtr-> satInfo[i].satConst = LE_GNSS_SV_CONSTELLATION_GPS;
                    // Satellites in View ID number
                    posDataPtr->satInfo[i].satId=
                        svInfoPtr->svList[i].gnssSvId + SAT_ID_OFFSET_GPS;
                    break;
                case eQMI_LOC_SV_SYSTEM_GLONASS_V02:
                    posDataPtr-> satInfo[i].satConst = LE_GNSS_SV_CONSTELLATION_GLONASS;
                    // Satellites in View ID number
                    posDataPtr->satInfo[i].satId=
                        svInfoPtr->svList[i].gnssSvId + SAT_ID_OFFSET_GLONASS;
                    break;
                case eQMI_LOC_SV_SYSTEM_SBAS_V02:
                    posDataPtr-> satInfo[i].satConst = LE_GNSS_SV_CONSTELLATION_SBAS;
                    // Satellites in View ID number
                    posDataPtr->satInfo[i].satId=
                        svInfoPtr->svList[i].gnssSvId + SAT_ID_OFFSET_SBAS;
                    break;
                case eQMI_LOC_SV_SYSTEM_GALILEO_V02:
                    posDataPtr-> satInfo[i].satConst = LE_GNSS_SV_CONSTELLATION_GALILEO;
                    // Satellites in View ID number
                    posDataPtr->satInfo[i].satId=
                        svInfoPtr->svList[i].gnssSvId + SAT_ID_OFFSET_GALILEO;
                    break;
                case eQMI_LOC_SV_SYSTEM_BDS_V02:
                    posDataPtr-> satInfo[i].satConst = LE_GNSS_SV_CONSTELLATION_BEIDOU;
                    // Satellites in View ID number
                    posDataPtr->satInfo[i].satId=
                        svInfoPtr->svList[i].gnssSvId + SAT_ID_OFFSET_BEIDOU;
                    break;
                // MDM9x40 or MDM9x28
#if defined(SIERRA_MDM9X40) || defined(SIERRA_MDM9X28)
                case eQMI_LOC_SV_SYSTEM_QZSS_V02:
                    posDataPtr-> satInfo[i].satConst = LE_GNSS_SV_CONSTELLATION_QZSS;
                    // Satellites in View ID number
                    posDataPtr->satInfo[i].satId=
                        svInfoPtr->svList[i].gnssSvId + SAT_ID_OFFSET_QZSS;
                    break;
#endif
                // COMPASS satellite (Deprecated)
                default:
                    posDataPtr-> satInfo[i].satConst = LE_GNSS_SV_CONSTELLATION_UNDEFINED;
                    LE_WARN("Undefined constellation %d for SV %d"
                            , svInfoPtr->svList[i].system
                            , svInfoPtr->svList[i].gnssSvId);
                    // Satellites in View ID number
                    posDataPtr->satInfo[i].satId = svInfoPtr->svList[i].gnssSvId;
                    break;
            }
            // Satellite in View tracked for Navigation
            if (svInfoPtr->svList[i].svStatus == eQMI_LOC_SV_STATUS_TRACK_V02)
            {
                posDataPtr->satInfo[i].satTracked = true;
                satsTrackingCount++;
            }
            else
            {
                posDataPtr->satInfo[i].satTracked = false;
            }
            LE_DEBUG("svInfo: sat viewed: %d tracked: %d",
                      posDataPtr->satInfo[i].satId,
                      posDataPtr->satInfo[i].satTracked);
        }
    }
    // Satellites in View count
    posDataPtr->satsInViewCountValid = true;
    posDataPtr->satsInViewCount = satsInViewCount;
    // Tracking satellites in View count
    posDataPtr->satsTrackingCountValid = true;
    posDataPtr->satsTrackingCount = satsTrackingCount;
}

//--------------------------------------------------------------------------------------------------
/**
 * Fill in the satellites measurement by parsing a measurement report.
 *
 * @return
 *      LE_OK if successful.
 *      LE_FAULT if there was an error.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t GetSatMeasurementFromReport
(
    const qmiLocEventGnssSvMeasInfoIndMsgT_v02* svMeasPtr,  // [IN] The measurement report.
    pa_Gnss_Position_t* posDataPtr                          // [OUT] Pointer to the position data.
)
{

    int i = 0;
    int j = 0;

    LE_DEBUG("Measurement report %d - len (%d)" , svMeasPtr->svMeasurement_valid
                                                , svMeasPtr->svMeasurement_len);

    if (svMeasPtr->svMeasurement_valid)
    {
        posDataPtr->satMeasValid = true;

        // Find empty field to append latency values.
        j = 0;
        while (j < LE_GNSS_SV_INFO_MAX_LEN)
        {
            if (0 == posDataPtr->satMeas[j].satId)
            {
                break;
            }

            j++;
        }

        LE_DEBUG("Index %d", j);

        for (i=0; (i < svMeasPtr->svMeasurement_len)&&((j+i)< LE_GNSS_SV_INFO_MAX_LEN); i++)
        {
            if (QMI_LOC_SV_MEAS_LIST_MAX_SIZE_V02 > i)
            {
                // Satellite latency measurement
                posDataPtr->satMeas[j+i].satId = svMeasPtr->svMeasurement[i].gnssSvId;
                posDataPtr->satMeas[j+i].satLatency =  svMeasPtr->svMeasurement[i].measLatency;
            }
        }

        for (i=0; i< LE_GNSS_SV_INFO_MAX_LEN; i++)
        {
            if (posDataPtr->satMeas[i].satId != 0)
            {
                LE_DEBUG("[%d] SvId (%d) measLatency (%d)"  , i
                                                            , posDataPtr->satMeas[i].satId
                                                            , posDataPtr->satMeas[i].satLatency);
            }
        }
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to enable/disable Xtra feature
 *
 * @return LE_FAULT         The function failed to enable/disable
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t EnableXtra
(
    bool enable
)
{
    le_result_t result;
    swi_m2m_gps_set_xtra_data_enable_req_msg_v01 setXtraDataEnableReq = {0};
    swi_m2m_gps_set_xtra_data_enable_resp_msg_v01 setXtraDataEnableRsp = { {0} };

    LE_DEBUG("Xtra to %s",enable?"Enable":"Disable");

    // Set XTRA injection state
    if (enable == true)
    {
        // 1: Enable, XTRA data can be injected from external such as QMI API
        setXtraDataEnableReq.gps_xtra_data_enable_flag = 1;
    }
    else
    {
        // Disable
        setXtraDataEnableReq.gps_xtra_data_enable_flag = 0;
    }

    // Send QMI message
    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(GeneralClient,
                                         QMI_SWI_M2M_GPS_SET_XTRA_DATA_ENABLE_REQ_V01,
                                         &setXtraDataEnableReq, sizeof(setXtraDataEnableReq),
                                         &setXtraDataEnableRsp, sizeof(setXtraDataEnableRsp),
                                         COMM_DEFAULT_PLATFORM_TIMEOUT);

    /* Check the response */
    if(swiQmi_OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_M2M_GPS_SET_XTRA_DATA_ENABLE_RESP_V01),
                                    clientMsgErr,
                                    setXtraDataEnableRsp.resp) != LE_OK)
    {
        LE_ERROR("Set XTRA injection state error");
        result = LE_FAULT;
    }
    else
    {
        LE_DEBUG("Set XTRA injection state done");
        result = LE_OK;
    }

    return result;

}

//--------------------------------------------------------------------------------------------------
/**
 * This function enables the use of the 'Extended Ephemeris' file into the GNSS device.
 *
 * @return LE_FAULT         The function failed to enable the 'Extended Ephemeris' file.
 * @return LE_OK            The function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_EnableExtendedEphemerisFile
(
    void
)
{
    return (EnableXtra(true));
}


//--------------------------------------------------------------------------------------------------
/**
 * This function disables the use of the 'Extended Ephemeris' file into the GNSS device.
 *
 * @return LE_FAULT         The function failed to disable the 'Extended Ephemeris' file.
 * @return LE_OK            The function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_DisableExtendedEphemerisFile
(
    void
)
{
    return (EnableXtra(false));
}

//--------------------------------------------------------------------------------------------------
/**
 * Reports the position event informing the application with position's data
 */
//--------------------------------------------------------------------------------------------------
static void ReportPositionEvent
(
    void
)
{
    // Build the data for the user's event handler.
    pa_Gnss_Position_t* posDataPtr = le_mem_ForceAlloc(PositionEventDataPool);
    memcpy(posDataPtr, &PositionData, sizeof(pa_Gnss_Position_t));

    InitializeValidGnssPosition(&PositionData);

    LE_DEBUG("Report Position Event");
    le_event_ReportWithRefCounting(PositionEvent, posDataPtr);
}

//--------------------------------------------------------------------------------------------------
/**
 * Reports the NMEA event informing the application with NMEA frames
 */
//--------------------------------------------------------------------------------------------------
static void ReportNmeaEvent
(
    char* nmeaPtr
)
{
    // Build the data for the user's event handler.
    char* nmeaDataPoolPtr = le_mem_ForceAlloc(NmeaEventDataPool);
    // Copy NMEA
    strncpy(nmeaDataPoolPtr, nmeaPtr, QMI_LOC_NMEA_STRING_MAX_LENGTH_V02 + 1);

    LE_DEBUG("Report NMEA Event");
    le_event_ReportWithRefCounting(NmeaEvent, nmeaDataPoolPtr);
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler function called when there is an Position Report indication. This handler reports a
 * position event to the user when there is a valid position event.
 */
//--------------------------------------------------------------------------------------------------
static void PositionHandler
(
    void*           indBufPtr,   // [IN] The indication message data.
    unsigned int    indBufLen,   // [IN] The indication message data length in bytes.
    void*           contextPtr   // [IN] Context value that was passed to
                                 //      swiQmi_AddIndicationHandler().
)
{
    char  timeString[50];
    size_t* numBytesPtr=0;

    if (indBufPtr == NULL)
    {
        LE_ERROR("Indication message empty.");
        return;
    }

    qmiLocEventPositionReportIndMsgT_v02 positionInd = {0};

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        LocClient, QMI_IDL_INDICATION, QMI_LOC_EVENT_POSITION_REPORT_IND_V02,
        indBufPtr, indBufLen,
        &positionInd, sizeof(positionInd));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_LOC_EVENT_POSITION_REPORT_IND_V02, error = %i",
                 clientMsgErr);
        return;
    }

    LE_DEBUG("Session Id (%d) status (%d)",positionInd.sessionId,positionInd.sessionStatus );

    if ((SESSION_ID != positionInd.sessionId) ||
       ((eQMI_LOC_SESS_STATUS_SUCCESS_V02 != positionInd.sessionStatus) &&
       (eQMI_LOC_SESS_STATUS_IN_PROGRESS_V02 != positionInd.sessionStatus)))
    {
        LE_WARN("Bad position indication");
        return;
    }

    GetPositionDataFromReport(&positionInd, &PositionData);

    if (!(PositionData.dateValid)      ||
        (!PositionData.timeValid)      ||
        (!PositionData.latitudeValid)  ||
        (!PositionData.longitudeValid) ||
        (eQMI_LOC_SESS_STATUS_SUCCESS_V02 != positionInd.sessionStatus))
    {
        // Date, time, latitude, longitude and sucess session are required to set a Fix.
        PositionData.fixState = LE_GNSS_STATE_FIX_NO_POS;
        LE_DEBUG("STATE_FIX_NO_POS: sessionStatus %d", positionInd.sessionStatus);
        ReportPositionEvent();
        return;
    }

    // TTFF update
    if (!GnssCtx.firstFix)
    {
        // Update First fix status
        GnssCtx.firstFix = true;
        // TTFF computation
        GnssCtx.ttff = le_clk_Sub(le_clk_GetRelativeTime(), GnssCtx.acqStartTime);
        // Debug traces
        le_clk_ConvertToLocalTimeString(GnssCtx.ttff, LE_CLK_STRING_FORMAT_TIME,
                                        timeString, sizeof(timeString), numBytesPtr);
        LE_DEBUG("TTFF %s", timeString);
    }

    // Get SVs that participated to calculate the fix. (The SVs providing DGNSS correction
    // are not taken into account for determining FIX_2D or FIX_3D)
    GetUsedSvFromPositionData(&positionInd,&PositionData);

    // Update fix state according to overall fix position information.
    if (PositionData.satsUsedCount < 3)
    {
        // Set the Fix state to "estimated" if less than 3 SV (0,1 or 2) are used to establish
        // the Fix, regardless if the altitude is estimated or not.
        PositionData.fixState = LE_GNSS_STATE_FIX_ESTIMATED;
        LE_DEBUG("STATE_FIX_ESTIMATED: satsUsedCount %d", PositionData.satsUsedCount);
        goto out;
    }

    if (
        (PositionData.satsUsedCount == 3) ||
        ((PositionData.satsUsedCount > 3) && (false == PositionData.altitudeValid)) ||
        ((PositionData.satsUsedCount > 3) && (true == PositionData.altitudeValid) &&
        (true == PositionData.altitudeAssumedValid) && (true == PositionData.altitudeAssumed))
       )
    {
        // Set the fix state to "2D Fix" :
        // - If exactly 3 SV are used to establish the Fix, regardless if the altitude is estimated
        //   or not.
        // - If more than 3 SV are used to establish the Fix but the altitude info is not valid.
        // - If more than 3 SV are used to establish the Fix, the altitude info is valid but the
        //   altitude is assumed.
        PositionData.fixState = LE_GNSS_STATE_FIX_2D;
        LE_DEBUG("STATE_FIX_2D: satsUsed %d, altValid %d",
                 PositionData.satsUsedCount, PositionData.altitudeValid);
        LE_DEBUG("STATE_FIX_2D: altAssumedValid %d, altAssumed %d",
                 PositionData.altitudeAssumedValid, PositionData.altitudeAssumed);
        goto out;
    }

    // For all other cases, the state is "3D Fix".
    PositionData.fixState = LE_GNSS_STATE_FIX_3D;
    LE_DEBUG("STATE_FIX_3D");

out:
    // Get SVs providing DGNSS correction
    GetDgnssUsedSvFromPositionData(&positionInd,&PositionData);
    // Report the position event
    ReportPositionEvent();

    return;
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler function called when there is an Measurement Report indication.
 */
//--------------------------------------------------------------------------------------------------
static void MeasurementReportHandler
(
    void*           indBufPtr,   // [IN] The indication message data.
    unsigned int    indBufLen,   // [IN] The indication message data length in bytes.
    void*           contextPtr   // [IN] Context value that was passed to
                                 //      swiQmi_AddIndicationHandler().
)
{
    if (indBufPtr == NULL)
    {
        LE_ERROR("Indication message empty.");
        return;
    }

    qmiLocEventGnssSvMeasInfoIndMsgT_v02 measurementInd = {0};

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        LocClient, QMI_IDL_INDICATION, QMI_LOC_EVENT_GNSS_MEASUREMENT_REPORT_IND_V02,
        indBufPtr, indBufLen,
        &measurementInd, sizeof(measurementInd));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_LOC_EVENT_GNSS_MEASUREMENT_REPORT_IND, error = %i"
                , clientMsgErr);
        return;
    }

    // Get satellites measurement
    if (GetSatMeasurementFromReport(&measurementInd,&PositionData) != LE_OK)
    {
        LE_ERROR("Get Satellites measurement failed");
    }

}

//--------------------------------------------------------------------------------------------------
/**
 * Handler function called when there is a NMEA Report indication.
 */
//--------------------------------------------------------------------------------------------------
static void NmeaHandler
(
    void*           indBufPtr,   // [IN] The indication message data.
    unsigned int    indBufLen,   // [IN] The indication message data length in bytes.
    void*           contextPtr   // [IN] Context value that was passed to
                                 //      swiQmi_AddIndicationHandler().
)
{
    if (indBufPtr == NULL)
    {
        LE_ERROR("Indication message empty.");
        return;
    }

    qmiLocEventNmeaIndMsgT_v02 nmeaInd = {{0}};

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        LocClient, QMI_IDL_INDICATION, QMI_LOC_EVENT_NMEA_IND_V02,
        indBufPtr, indBufLen,
        &nmeaInd, sizeof(nmeaInd));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_LOC_EVENT_NMEA_IND_V02, error = %i", clientMsgErr);
        return;
    }

    // Report the NMEA sentence to the application
    ReportNmeaEvent(nmeaInd.nmea);
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler function called when there is an SV information Report indication.
 */
//--------------------------------------------------------------------------------------------------
static void SvInfoHandler
(
    void*           indBufPtr,   // [IN] The indication message data.
    unsigned int    indBufLen,   // [IN] The indication message data length in bytes.
    void*           contextPtr   // [IN] Context value that was passed to
                                 //      swiQmi_AddIndicationHandler().
)
{
    if (indBufPtr == NULL)
    {
        LE_ERROR("Indication message empty.");
        return;
    }

    qmiLocEventGnssSvInfoIndMsgT_v02 svInfoInd = {0};

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        LocClient, QMI_IDL_INDICATION, QMI_LOC_EVENT_GNSS_SV_INFO_IND_V02,
        indBufPtr, indBufLen,
        &svInfoInd, sizeof(svInfoInd));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_LOC_EVENT_GNSS_SV_INFO_IND_V02, error = %i", clientMsgErr);
        return;
    }

    // Get satellites information
    GetSatInfoFromReport(&svInfoInd,&PositionData);
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler function called when there is an Xtra injection indication.
 */
//--------------------------------------------------------------------------------------------------
static void XTRAInjectionHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                // swiQmi_AddIndicationHandler().
)
{
    if (indBufPtr == NULL)
    {
        LE_ERROR("Indication message empty.");
        return;
    }

    pa_gnss_LocSync_t *locSyncPtr =
              &FunctionSync[PA_GNSS_APISYNC_QMI_LOC_INJECT_PREDICTED_ORBITS_DATA_REQ_V02];
    uint32_t *numberPartLeftPtr = (uint32_t*)locSyncPtr->dataPtr;

    qmiLocInjectPredictedOrbitsDataIndMsgT_v02 xtraInjectionInd = {0};

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        LocClient, QMI_IDL_INDICATION, QMI_LOC_INJECT_PREDICTED_ORBITS_DATA_IND_V02,
        indBufPtr, indBufLen,
        &xtraInjectionInd, sizeof(xtraInjectionInd));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_LOC_INJECT_PREDICTED_ORBITS_DATA_IND_V02, error = %i",
                 clientMsgErr);
        // don't post semaphoreXtra
        SendCommandResult(locSyncPtr,LE_FAULT);
        return;
    }

    LE_DEBUG("XTRA injection part (%d-%d)->%d",
              xtraInjectionInd.partNum_valid,
              xtraInjectionInd.partNum,
              xtraInjectionInd.status);

    (*numberPartLeftPtr)--;

    if ( xtraInjectionInd.status == eQMI_LOC_SUCCESS_V02 )
    {
        // Final event success
        if ((*numberPartLeftPtr)==0)
        {
            SendCommandResult(locSyncPtr,LE_OK);
        }
        else
        {
            LE_DEBUG("Still waiting %d part",(*numberPartLeftPtr));
        }
    }
    else
    {
        LE_WARN("There was an error (%x)",xtraInjectionInd.status);
        if ( xtraInjectionInd.status == eQMI_LOC_GENERAL_FAILURE_V02)
        {
            SendCommandResult(locSyncPtr,LE_FORMAT_ERROR);
        }
        else
        {
            SendCommandResult(locSyncPtr,LE_FAULT);
        }
    }

    // post the synchronisation XTRA file block transfer semaphore
    le_sem_Post(locSyncPtr->semaphoreXtra);
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler function called when there is an UTC time injection indication.
 */
//--------------------------------------------------------------------------------------------------
static void UtcTimeInjectionHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                // swiQmi_AddIndicationHandler().
)
{
    if (NULL == indBufPtr)
    {
        LE_ERROR("Indication message empty.");
        return;
    }

    pa_gnss_LocSync_t *locSyncPtr =
              &FunctionSync[PA_GNSS_APISYNC_QMI_LOC_INJECT_UTC_TIME_REQ_V02];

    SWIQMI_DECLARE_MESSAGE(qmiLocInjectUtcTimeIndMsgT_v02, InjectUtcTimeInd);

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        LocClient, QMI_IDL_INDICATION, QMI_LOC_INJECT_UTC_TIME_IND_V02,
        indBufPtr, indBufLen,
        &InjectUtcTimeInd, sizeof(InjectUtcTimeInd));

    if (QMI_NO_ERR != clientMsgErr)
    {
        LE_ERROR("Error in decoding QMI_LOC_INJECT_UTC_TIME_IND_V02, error = %i",
                 clientMsgErr);
        SendCommandResult(locSyncPtr,LE_FAULT);
        return;
    }

    if (eQMI_LOC_SUCCESS_V02 == InjectUtcTimeInd.status)
    {
        SendCommandResult(locSyncPtr,LE_OK);
    }
    else
    {
        LE_WARN("There was an error (%x)",InjectUtcTimeInd.status);
        SendCommandResult(locSyncPtr,LE_FAULT);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler function called for xtra validity indication.
 */
//--------------------------------------------------------------------------------------------------
static void XTRAValidityHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    if (indBufPtr == NULL)
    {
        LE_ERROR("Indication message empty.");
        return;
    }

    pa_gnss_LocSync_t *locSyncPtr =
                 &FunctionSync[PA_GNSS_APISYNC_QMI_LOC_GET_PREDICTED_ORBITS_DATA_VALIDITY_REQ_V02];
    qmiLocGetPredictedOrbitsDataValidityIndMsgT_v02 *xtraValidityIndPtr = locSyncPtr->dataPtr;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        LocClient, QMI_IDL_INDICATION, QMI_LOC_GET_PREDICTED_ORBITS_DATA_VALIDITY_IND_V02,
        indBufPtr, indBufLen,
        xtraValidityIndPtr, sizeof(*xtraValidityIndPtr));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_LOC_GET_PREDICTED_ORBITS_DATA_VALIDITY_REQ_V02, error = %i",
                 clientMsgErr);
        SendCommandResult(locSyncPtr,LE_FAULT);
        return;
    }

    LE_DEBUG("Status %d",xtraValidityIndPtr->status);

    LE_DEBUG("XTRA validity %d start %"PRIu64" duration %"PRIu16,
        xtraValidityIndPtr->validityInfo_valid,
        xtraValidityIndPtr->validityInfo.startTimeInUTC,
        xtraValidityIndPtr->validityInfo.durationHours);

    SendCommandResult(locSyncPtr,LE_OK);
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler function called for GNSS service Data Deletion indication.
 */
//--------------------------------------------------------------------------------------------------
static void GnssDataDeleteHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    if (indBufPtr == NULL)
    {
        LE_ERROR("Indication message empty.");
        return;
    }

#if defined(QMI_LOC_DELETE_ASSIST_DATA_REQ_V02)

    qmiLocDeleteAssistDataIndMsgT_v02 delInd;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        LocClient, QMI_IDL_INDICATION, QMI_LOC_DELETE_ASSIST_DATA_IND_V02,
        indBufPtr, indBufLen,
        &delInd, sizeof(delInd));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_LOC_DELETE_ASSIST_DATA_IND_V02, error = %i", clientMsgErr);
        return;
    }

#elif defined(QMI_LOC_DELETE_GNSS_SERVICE_DATA_REQ_V02)

    qmiLocDeleteGNSSServiceDataIndMsgT_v02 delInd;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        LocClient, QMI_IDL_INDICATION, QMI_LOC_DELETE_GNSS_SERVICE_DATA_IND_V02,
        indBufPtr, indBufLen,
        &delInd, sizeof(delInd));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_LOC_DELETE_GNSS_SERVICE_DATA_IND_V02, error = %i",
                 clientMsgErr);
        return;
    }
#else
    LE_ERROR("Clear Data service mode not supported");
#endif

    if (delInd.status != 0x00)
    {
        LE_ERROR("Delete GNSS data indication status failed (%d)", delInd.status);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler function called for SUPL operation mode indication.
 */
//--------------------------------------------------------------------------------------------------
static void SetSuplAssitedModeHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    if (indBufPtr == NULL)
    {
        LE_ERROR("Indication message empty.");
        return;
    }

    pa_gnss_LocSync_t *locSyncPtr = &FunctionSync[
                                    PA_GNSS_APISYNC_QMI_LOC_SET_OPERATION_MODE_REQ_V02];
    qmiLocSetOperationModeIndMsgT_v02 *operationModeIndPtr = locSyncPtr->dataPtr;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        LocClient, QMI_IDL_INDICATION, QMI_LOC_SET_OPERATION_MODE_IND_V02,
        indBufPtr, indBufLen,
        operationModeIndPtr, sizeof(*operationModeIndPtr));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_LOC_SET_OPERATION_MODE_IND_V02, error = %i",
                 clientMsgErr);
        SendCommandResult(locSyncPtr,LE_FAULT);
        return;
    }

    LE_DEBUG("QMI_LOC_SET_OPERATION_MODE_IND_V02 status %d",operationModeIndPtr->status);

    SendCommandResult(locSyncPtr,LE_OK);
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler function called for SUPL server configuration.
 */
//--------------------------------------------------------------------------------------------------
static void SetSuplServerConfigHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    if (indBufPtr == NULL)
    {
        LE_ERROR("Indication message empty.");
        return;
    }

    pa_gnss_LocSync_t *locSyncPtr = &FunctionSync[PA_GNSS_APISYNC_QMI_LOC_SET_SERVER_REQ_V02];
    qmiLocSetServerIndMsgT_v02 *suplSetServerIndPtr = locSyncPtr->dataPtr;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        LocClient, QMI_IDL_INDICATION, QMI_LOC_SET_SERVER_IND_V02,
        indBufPtr, indBufLen,
        suplSetServerIndPtr, sizeof(*suplSetServerIndPtr));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_LOC_SET_SERVER_IND_V02, error = %i",
                 clientMsgErr);
        SendCommandResult(locSyncPtr,LE_FAULT);
        return;
    }

    LE_DEBUG("QMI_LOC_SET_SERVER_IND_V02 status %d",suplSetServerIndPtr->status);

    SendCommandResult(locSyncPtr,LE_OK);
}


//--------------------------------------------------------------------------------------------------
/**
 * Handler function called for Engines On/Off
 */
//--------------------------------------------------------------------------------------------------
static void EngineStatusHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    pa_gnss_LocSync_t *locSyncPtr = &FunctionSync
                                [PA_GNSS_APISYNC_QMI_LOC_EVENT_ENGINE_STATE_IND_V02];

    if (indBufPtr == NULL)
    {
        LE_ERROR("Indication message empty.");
        return;
    }

    qmiLocEventEngineStateIndMsgT_v02 engineStateInd = {0};

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        LocClient, QMI_IDL_INDICATION, QMI_LOC_EVENT_ENGINE_STATE_IND_V02,
        indBufPtr, indBufLen,
        &engineStateInd, sizeof(engineStateInd));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_LOC_EVENT_ENGINE_STATE_IND_V02, error = %i",
                 clientMsgErr);
        SendCommandResult(locSyncPtr,LE_FAULT);
        return;
    }

    LE_DEBUG("EngineStatusHandler engineState = %d", engineStateInd.engineState);

    // update gnssEngineState
    locSyncPtr->gnssEngineState = engineStateInd.engineState;

    if ( ((START_REQUEST_PATTERN == locSyncPtr->engineState) &&
          (eQMI_LOC_ENGINE_STATE_ON_V02 == engineStateInd.engineState))
       ||
         ((STOP_REQUEST_PATTERN == locSyncPtr->engineState) &&
         (eQMI_LOC_ENGINE_STATE_OFF_V02 == engineStateInd.engineState))
       )
    {
        locSyncPtr->engineState = engineStateInd.engineState;

        LE_DEBUG("EngineStatusHandler: EngineState is now:%d (on=%d ;off=%d",
                  engineStateInd.engineState,
                  eQMI_LOC_ENGINE_STATE_ON_V02,
                  eQMI_LOC_ENGINE_STATE_OFF_V02);
        SendCommandResult(locSyncPtr,LE_OK);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler function called for SUPL certificate injection.
 */
//--------------------------------------------------------------------------------------------------
static void InjectSuplCertificateHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    if (indBufPtr == NULL)
    {
        LE_ERROR("Indication message empty.");
        return;
    }

    pa_gnss_LocSync_t *locSyncPtr = &FunctionSync[
                                    PA_GNSS_APISYNC_QMI_LOC_INJECT_SUPL_CERTIFICATE_REQ_V02];
    qmiLocInjectSuplCertificateIndMsgT_v02 *suplInjectCertificateIndPtr = locSyncPtr->dataPtr;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        LocClient, QMI_IDL_INDICATION, QMI_LOC_INJECT_SUPL_CERTIFICATE_IND_V02,
        indBufPtr, indBufLen,
        suplInjectCertificateIndPtr, sizeof(*suplInjectCertificateIndPtr));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_LOC_INJECT_SUPL_CERTIFICATE_IND_V02, error = %i",
                 clientMsgErr);
        SendCommandResult(locSyncPtr,LE_FAULT);
        return;
    }

    SendCommandResult(locSyncPtr,LE_OK);
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler function called for SUPL certificate deletion.
 */
//--------------------------------------------------------------------------------------------------
static void DeleteSuplCertificateHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                // swiQmi_AddIndicationHandler().
)
{
    if (indBufPtr == NULL)
    {
        LE_ERROR("Indication message empty.");
        return;
    }

    pa_gnss_LocSync_t *locSyncPtr = &FunctionSync[
                                     PA_GNSS_APISYNC_QMI_LOC_DELETE_SUPL_CERTIFICATE_REQ_V02];
    qmiLocDeleteSuplCertificateIndMsgT_v02 *suplDeleteCertificateIndPtr = locSyncPtr->dataPtr;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        LocClient, QMI_IDL_INDICATION, QMI_LOC_DELETE_SUPL_CERTIFICATE_IND_V02,
        indBufPtr, indBufLen,
        suplDeleteCertificateIndPtr, sizeof(*suplDeleteCertificateIndPtr));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_LOC_DELETE_SUPL_CERTIFICATE_IND_V02, error = %i",
                 clientMsgErr);
        SendCommandResult(locSyncPtr,LE_FAULT);
        return;
    }

    SendCommandResult(locSyncPtr,LE_OK);
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler function called for registered events indication.
 */
//--------------------------------------------------------------------------------------------------
static void RegEventsHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    if (indBufPtr == NULL)
    {
        LE_ERROR("Indication message empty.");
        return;
    }

    pa_gnss_LocSync_t *locSyncPtr =
                   &FunctionSync[PA_GNSS_APISYNC_QMI_LOC_GET_REGISTERED_EVENTS_REQ_V02];
    qmiLocGetRegisteredEventsIndMsgT_v02 *registeredEventsIndPtr = locSyncPtr->dataPtr;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        LocClient, QMI_IDL_INDICATION, QMI_LOC_GET_REGISTERED_EVENTS_REQ_V02,
        indBufPtr, indBufLen,
        registeredEventsIndPtr, sizeof(*registeredEventsIndPtr));

    if (clientMsgErr != QMI_NO_ERR)
    {
        LE_ERROR("Error in decoding QMI_LOC_GET_REGISTERED_EVENTS_REQ_V02, error = %i",
                 clientMsgErr);
        SendCommandResult(locSyncPtr,LE_FAULT);
        return;
    }

    LE_DEBUG("Reg events %d, 0X%X"  , registeredEventsIndPtr->eventRegMask_valid
                                    ,(uint) registeredEventsIndPtr->eventRegMask);

    SendCommandResult(locSyncPtr,LE_OK);
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler function called for setting enabled NMEA sentences.
 */
//--------------------------------------------------------------------------------------------------
static void SetNmeaSentencesHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    if (NULL == indBufPtr)
    {
        LE_ERROR("Indication message empty.");
        return;
    }

    pa_gnss_LocSync_t *locSyncPtr = &FunctionSync[PA_GNSS_APISYNC_QMI_LOC_SET_NMEA_TYPES_REQ_V02];
    qmiLocSetNmeaTypesIndMsgT_v02 *nmeaTypesPtr = locSyncPtr->dataPtr;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        LocClient, QMI_IDL_INDICATION, QMI_LOC_SET_NMEA_TYPES_IND_V02,
        indBufPtr, indBufLen,
        nmeaTypesPtr, sizeof(*nmeaTypesPtr));

    if (QMI_NO_ERR != clientMsgErr)
    {
        LE_ERROR("Error in decoding QMI_LOC_SET_NMEA_TYPES_IND_V02, error = %i",
                  clientMsgErr);
        SendCommandResult(locSyncPtr, LE_FAULT);
        return;
    }

    LE_DEBUG("QMI_LOC_SET_NMEA_TYPES_IND_V02 status %d", nmeaTypesPtr->status);

    SendCommandResult(locSyncPtr, LE_OK);
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler function called for getting enabled NMEA sentences.
 */
//--------------------------------------------------------------------------------------------------
static void GetNmeaSentencesHandler
(
    void*           indBufPtr,  // [IN] The indication message data.
    unsigned int    indBufLen,  // [IN] The indication message data length in bytes.
    void*           contextPtr  // [IN] Context value that was passed to
                                //      swiQmi_AddIndicationHandler().
)
{
    if (NULL == indBufPtr)
    {
        LE_ERROR("Indication message empty.");
        return;
    }

    pa_gnss_LocSync_t *locSyncPtr = &FunctionSync[PA_GNSS_APISYNC_QMI_LOC_GET_NMEA_TYPES_REQ_V02];
    qmiLocGetNmeaTypesIndMsgT_v02 *nmeaTypesPtr = locSyncPtr->dataPtr;

    qmi_client_error_type clientMsgErr = qmi_client_message_decode(
        LocClient, QMI_IDL_INDICATION, QMI_LOC_GET_NMEA_TYPES_IND_V02,
        indBufPtr, indBufLen,
        nmeaTypesPtr, sizeof(*nmeaTypesPtr));

    if (QMI_NO_ERR != clientMsgErr)
    {
        LE_ERROR("Error in decoding QMI_LOC_GET_NMEA_TYPES_IND_V02, error = %i",
                  clientMsgErr);
        SendCommandResult(locSyncPtr, LE_FAULT);
        return;
    }

    LE_DEBUG("QMI_LOC_GET_NMEA_TYPES_IND_V02 status %d", nmeaTypesPtr->status);

    SendCommandResult(locSyncPtr, LE_OK);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function sets the GNSS registered events
 *
 * @return LE_FAULT           The function failed.
 * @return LE_OK              The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SetRegEvents
(
    bool enableMask,                        ///< [IN] The field(s) of the specified mask
                                            ///< is/are unmasked (1) or masked (0)
    qmiLocEventRegMaskT_v02 eventRegMask    ///< [IN] Event registration mask
)
{
    le_result_t result;
    pa_gnss_LocSync_t *locSyncPtr =
                        &FunctionSync[PA_GNSS_APISYNC_QMI_LOC_GET_REGISTERED_EVENTS_REQ_V02];
    qmiLocGetRegisteredEventsIndMsgT_v02 regEventsInd = {0};
    qmiLocGenRespMsgT_v02 genResp = { {0} };
    qmiLocRegEventsReqMsgT_v02 regEventReq = {0};
    qmi_client_error_type clientMsgErr;

    LE_DEBUG("Set events %d, 0x%X", enableMask, (uint)eventRegMask);

    // Get registered events from QMI

    // Set buffer to fill by the handler
    locSyncPtr->dataPtr = &regEventsInd;

    // Start the command
    StartCommand(locSyncPtr);

    clientMsgErr = qmi_client_send_msg_sync(LocClient,
                                                QMI_LOC_GET_REGISTERED_EVENTS_REQ_V02,
                                                NULL, 0,
                                                &genResp, sizeof(genResp),
                                                COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_LOC_GET_REGISTERED_EVENTS_RESP_V02),
                                    clientMsgErr,
                                    genResp.resp) != LE_OK)
    {
        StopCommand(locSyncPtr);
        return LE_FAULT;
    }

    le_clk_Time_t timer = { .sec= TIMEOUT_SEC, .usec= TIMEOUT_USEC };

    result = le_sem_WaitWithTimeOut(locSyncPtr->semaphoreRef,timer);
    // Command is finished
    StopCommand(locSyncPtr);

    // Check le_sem_WaitUntil timeOut
    if (result != LE_OK)
    {
        LE_WARN("Timeout happen");
        return LE_FAULT;
    }

    // Check synchronization command success
    if ( locSyncPtr->result != LE_OK)
    {
        return locSyncPtr->result;
    }

    if (regEventsInd.eventRegMask_valid)
    {
        LE_DEBUG("Get events 0x%X", (uint)regEventsInd.eventRegMask);
    }
    else
    {
        LE_WARN("Invalid response");
        return LE_FAULT;
    }

    // Apply the mask
    if (enableMask)
    {
        regEventsInd.eventRegMask |= eventRegMask;
    }
    else
    {
        regEventsInd.eventRegMask &= ~eventRegMask;
    }

    // Set the position event.
    regEventReq.eventRegMask = regEventsInd.eventRegMask;

    clientMsgErr = qmi_client_send_msg_sync(LocClient,
                                                        QMI_LOC_REG_EVENTS_REQ_V02,
                                                        &regEventReq, sizeof(regEventReq),
                                                        &genResp, sizeof(genResp),
                                                        COMM_DEFAULT_PLATFORM_TIMEOUT);

    result = swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_LOC_REG_EVENTS_REQ_V02),
                                                  clientMsgErr,
                                                  genResp.resp);

    LE_DEBUG("Events registered (0x%X,%d)", (uint)regEventReq.eventRegMask, result);

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to stop the GNSS acquisition in the case of restarting the GNSS
 * device by pa_gnss_ForceRestart().
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeed.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ForceGnssStop
(
    void
)
{
    le_result_t result;
    char semName[64];
    qmiLocStopReqMsgT_v02 stopReq = {0};
    qmiLocGenRespMsgT_v02 genResp = { {0} };
    pa_gnss_LocSync_t *locSyncPtr =
        &FunctionSync[PA_GNSS_APISYNC_QMI_LOC_EVENT_ENGINE_STATE_IND_V02];
    le_clk_Time_t timeToWait = { .sec= STOP_TIMEOUT_SEC, .usec= STOP_TIMEOUT_USEC };
    locSyncPtr->engineState = STOP_REQUEST_PATTERN;

    LE_DEBUG("Force Stop GNSS");

    snprintf(semName,sizeof(semName),"GnssFuncSem-%d",
                 PA_GNSS_APISYNC_QMI_LOC_EVENT_ENGINE_STATE_IND_V02);
    locSyncPtr->semaphoreRef = le_sem_Create(semName,0);

    // Start the command
    StartCommand(locSyncPtr);

    // Use the same session ID that was used to start the acquisition.
    stopReq.sessionId = SESSION_ID;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(LocClient,
                                                        QMI_LOC_STOP_REQ_V02,
                                                        &stopReq, sizeof(stopReq),
                                                        &genResp, sizeof(genResp),
                                                        COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_LOC_STOP_REQ_V02),
                                    clientMsgErr,
                                    genResp.resp) == LE_OK)
    {
        le_result_t semResult = le_sem_WaitWithTimeOut(locSyncPtr->semaphoreRef,timeToWait);
        if (semResult != LE_OK)
        {
            LE_ERROR("ERROR le_sem_WaitWithTimeOut timed out %d", semResult);
            LE_ERROR("ERROR EngineState is not OFF.  %d", locSyncPtr->engineState);
            result = LE_FAULT;
            goto out;
        }

        if (locSyncPtr->result != LE_OK)
        {
            LE_ERROR("Error for PA_GNSS_APISYNC_QMI_LOC_EVENT_ENGINE_STATE_IND_V02, error = %i"
                         , locSyncPtr->result);
            result = LE_FAULT;
            goto out;
        }

        // check that GNSS is really OFF
        if(eQMI_LOC_ENGINE_STATE_OFF_V02 != locSyncPtr->engineState)
        {
            LE_ERROR("ERROR EngineState is not OFF.  %d", locSyncPtr->engineState);
            result = LE_FAULT;
            goto out;
        }

        LE_INFO("EngineState is OFF");
        result = LE_OK;
    }
    else
    {
        LE_ERROR("Error for PA_GNSS_APISYNC_QMI_LOC_EVENT_ENGINE_STATE_IND_V02");
        result = LE_FAULT;
    }

out:
    StopCommand(locSyncPtr);
    le_sem_Delete(locSyncPtr->semaphoreRef);
    locSyncPtr->semaphoreRef = NULL;
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function clears GNSS data services such as ephemeris, position, time, Almanac
 * and calibration data.
 *
 * @return LE_FAULT           The function failed.
 * @return LE_OK              The function succeeded.
 * @return LE_UNSUPPORTED     Restart type not supported.
 * @return LE_BAD_PARAMETER   Bad input parameter.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ClearDataServices
(
    bool deleteAll,                  ///< [IN] Clear all GNSS services
    pa_gnss_Restart_t  restartType   ///< [IN] GNSS restart type
)
{
    le_result_t result;

    if (PA_GNSS_HOT_RESTART == restartType)
    {
        // Nothing to clear for HOT start
        return LE_OK;
    }
    else if (PA_GNSS_UNKNOWN_RESTART == restartType)
    {
        LE_ERROR("Unknown restart type!");
        return LE_BAD_PARAMETER;
    }
    else
    {
        SWIQMI_DECLARE_MESSAGE(qmiLocGenRespMsgT_v02, genResp);

#if defined(QMI_LOC_DELETE_GNSS_SERVICE_DATA_REQ_V02)
        SWIQMI_DECLARE_MESSAGE(qmiLocDeleteGNSSServiceDataReqMsgT_v02, dataReq);
#elif defined(QMI_LOC_DELETE_ASSIST_DATA_REQ_V02)
        SWIQMI_DECLARE_MESSAGE(qmiLocDeleteAssistDataReqMsgT_v02, dataReq);
        int i;
#else
        LE_ERROR("Clear Data service mode not supported");
        return LE_UNSUPPORTED;
#endif
        LE_DEBUG("ClearDataServices (%d,%d)", deleteAll, restartType);

        // Clear all GNSS services
        if ((deleteAll) || (PA_GNSS_FACTORY_RESTART == restartType))
        {
            // 0x01 (TRUE)  -- All constellation's service data is to be reset;
            // 0x00 (FALSE) -- The optional fields in the message are to be used to determine which
            //                 data is to be deleted
            dataReq.deleteAllFlag = 0x01;
        }

#if defined(QMI_LOC_DELETE_GNSS_SERVICE_DATA_REQ_V02)
        // Warm or Cold restart
        dataReq.deleteSatelliteData_valid = true;

        // GPS system data to be deleted
        dataReq.deleteSatelliteData.system = QMI_LOC_SYSTEM_GPS_V02 | QMI_LOC_SYSTEM_GLO_V02 |
                                             QMI_LOC_SYSTEM_BDS_V02 |  QMI_LOC_SYSTEM_GAL_V02 |
                                             QMI_LOC_SYSTEM_QZSS_V02;

        if (PA_GNSS_WARM_RESTART == restartType)
        {
            // Requested bitmask of data to be deleted for the specified satellite system.
            dataReq.deleteSatelliteData.deleteSatelliteDataMask =
                   QMI_LOC_DELETE_DATA_MASK_EPHEMERIS_V02;
        }
        else
        {
            //  Cold restart
            // Requested bitmask of data to be deleted for the specified satellite system.
            dataReq.deleteSatelliteData.deleteSatelliteDataMask =
                    QMI_LOC_DELETE_DATA_MASK_EPHEMERIS_V02 | QMI_LOC_DELETE_DATA_MASK_TIME_V02;

            // Requested bitmask of common data to be deleted
            dataReq.deleteCommonDataMask_valid = true;
            // reset Position estimate common for all GNSS type, all CLOCK_INFO mask
            // and UTC estimate
            dataReq.deleteCommonDataMask = QMI_LOC_DELETE_COMMON_MASK_POS_V02 |
                       QMI_LOC_DELETE_COMMON_MASK_TIME_V02 | QMI_LOC_DELETE_COMMON_MASK_UTC_V02;
        }

        qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(LocClient,
                                             QMI_LOC_DELETE_GNSS_SERVICE_DATA_REQ_V02,
                                             &dataReq, sizeof(dataReq),
                                             &genResp, sizeof(genResp),
                                             COMM_DEFAULT_PLATFORM_TIMEOUT);

        result = swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_LOC_DELETE_GNSS_SERVICE_DATA_REQ_V02),
                                      clientMsgErr,
                                      genResp.resp.result,
                                      genResp.resp.error);

#elif defined(QMI_LOC_DELETE_ASSIST_DATA_REQ_V02)
        // Warm or cold restart, ephemerises cleared
        dataReq.deleteSvInfoList_valid  = true;
        dataReq.deleteSvInfoList_len = 96;
        // GPS ephemerises
        for(i = 0; i < 32; i++)
        {
            dataReq.deleteSvInfoList[i].gnssSvId = (i + 1);
            dataReq.deleteSvInfoList[i].system = eQMI_LOC_SV_SYSTEM_GPS_V02;
            dataReq.deleteSvInfoList[i].deleteSvInfoMask = QMI_LOC_MASK_DELETE_EPHEMERIS_V02;
        }
        // SBAS ephemerises
        for(i = 32; i < 64; i++)
        {
            dataReq.deleteSvInfoList[i].gnssSvId = (i + 1);
            dataReq.deleteSvInfoList[i].system = eQMI_LOC_SV_SYSTEM_SBAS_V02;
            dataReq.deleteSvInfoList[i].deleteSvInfoMask = QMI_LOC_MASK_DELETE_EPHEMERIS_V02;
        }
        // GLONASS ephemerises
        for(i = 64; i < 96; i++)
        {
            dataReq.deleteSvInfoList[i].gnssSvId = (i + 1);
            dataReq.deleteSvInfoList[i].system = eQMI_LOC_SV_SYSTEM_GLONASS_V02;
            dataReq.deleteSvInfoList[i].deleteSvInfoMask = QMI_LOC_MASK_DELETE_EPHEMERIS_V02;
        }

        if (PA_GNSS_COLD_RESTART == restartType)
        {
            /* Clear position & time */
            dataReq.deleteGnssDataMask_valid = true;
            dataReq.deleteGnssDataMask = ( 0x00000004 |  // DELETE_GPS_TIME
                                           0x00000040 |  // DELETE_GLO_TIME
                                           0x00000400 |  // DELETE_POSITION
                                           0x00000800 |  // DELETE_TIME
                                           0x00002000 ); // DELETE_UTC
        }

        qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(LocClient,
                                             QMI_LOC_DELETE_ASSIST_DATA_REQ_V02,
                                             &dataReq, sizeof(dataReq),
                                             &genResp, sizeof(genResp),
                                             COMM_DEFAULT_PLATFORM_TIMEOUT);

        result = swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_LOC_DELETE_ASSIST_DATA_REQ_V02),
                                      clientMsgErr,
                                      genResp.resp.result,
                                      genResp.resp.error);
#endif
        if (LE_OK != result)
        {
            LE_ERROR("Failed to clear DataServices");
        }
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the PA GNSS Module.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_Init
(
    void
)
{
    le_result_t result;

    // Create the event and memory pool for signaling user handlers.
    PositionEvent = le_event_CreateIdWithRefCounting("PositionEvent");
    PositionEventDataPool = le_mem_CreatePool("PositionEventDataPool", sizeof(pa_Gnss_Position_t));

    NmeaEvent = le_event_CreateIdWithRefCounting("NmeaEvent");
    NmeaEventDataPool = le_mem_CreatePool(  "NmeaEventDataPool"
                                            , QMI_LOC_NMEA_STRING_MAX_LENGTH_V02 + 1);

    int i;
    for (i=0;i<PA_GNSS_APISYNC_MAX;i++)
    {
        char semName[64];
        char mutexName[64];
        snprintf(semName,sizeof(semName),"GnssFuncSem-%d",i);
        snprintf(mutexName,sizeof(mutexName),"GnssFuncMutext-%d",i);

        FunctionSync[i].commandInProgress = false;
        pthread_mutex_init(&FunctionSync[i].mutexPosix, NULL);
        FunctionSync[i].semaphoreRef = le_sem_Create(semName,0);
        FunctionSync[i].semaphoreXtra = le_sem_Create(semName,0);
        FunctionSync[i].dataPtr = NULL;
        FunctionSync[i].gnssEngineState = eQMI_LOC_ENGINE_STATE_OFF_V02;
    }

    // Initialize data position
    InitializeDataPosition(&PositionData);

    // Start the qmi services.
    if (swiQmi_InitServices(QMI_SERVICE_LOC) != LE_OK)
    {
        LE_ERROR("Could not initialize the QMI Positioning Services.");
        return LE_FAULT;
    }
    if (swiQmi_InitServices(QMI_SERVICE_DMS) != LE_OK)
    {
        LE_ERROR("Could not initialize the QMI DMS Services.");
        return LE_FAULT;
    }
    if (swiQmi_InitServices(QMI_SERVICE_MGS) != LE_OK)
    {
        LE_ERROR("Could not initialize the QMI MGS Services.");
        return LE_FAULT;
    }

    if (LE_OK != swiQmi_InitServices(QMI_SERVICE_SWI_LOC))
    {
        LE_WARN("Could not initialize the SWI QMI Location Services.");
    }

    // Get the qmi client service handle for later use.
    LocClient = swiQmi_GetClientHandle(QMI_SERVICE_LOC);
    if (NULL == LocClient)
    {
        return LE_FAULT;
    }

    DmsClient = swiQmi_GetClientHandle(QMI_SERVICE_DMS);
    if (NULL == DmsClient)
    {
        return LE_FAULT;
    }

    GeneralClient = swiQmi_GetClientHandle(QMI_SERVICE_MGS);
    if (NULL == GeneralClient)
    {
        return LE_FAULT;
    }

    SwiLocClient = swiQmi_GetClientHandle(QMI_SERVICE_SWI_LOC);
    if (NULL == SwiLocClient)
    {
        LE_WARN("Could not initialize the SWI QMI Location client handle.");
    }

    // Register our own handlers with the QMI LOC service.
    swiQmi_AddIndicationHandler(PositionHandler, QMI_SERVICE_LOC,
                                QMI_LOC_EVENT_POSITION_REPORT_IND_V02,NULL);

    // Register our own handlers with the QMI LOC service.
    swiQmi_AddIndicationHandler(SvInfoHandler, QMI_SERVICE_LOC,
                                QMI_LOC_EVENT_GNSS_SV_INFO_IND_V02,NULL);

    // Register our own handler with the QMI LOC service.
    swiQmi_AddIndicationHandler(XTRAInjectionHandler, QMI_SERVICE_LOC,
                                QMI_LOC_INJECT_PREDICTED_ORBITS_DATA_IND_V02,NULL);

    // Register our own handler with the QMI LOC service.
    swiQmi_AddIndicationHandler(UtcTimeInjectionHandler, QMI_SERVICE_LOC,
                                QMI_LOC_INJECT_UTC_TIME_IND_V02,NULL);

    // Register our own handler with the QMI LOC service.
    swiQmi_AddIndicationHandler(XTRAValidityHandler, QMI_SERVICE_LOC,
                                QMI_LOC_GET_PREDICTED_ORBITS_DATA_VALIDITY_IND_V02,NULL);

    // Register our own handler with the QMI LOC service.
#if defined(QMI_LOC_DELETE_ASSIST_DATA_REQ_V02)
    swiQmi_AddIndicationHandler(GnssDataDeleteHandler, QMI_SERVICE_LOC,
                                QMI_LOC_DELETE_ASSIST_DATA_IND_V02, NULL);
#elif defined(QMI_LOC_DELETE_GNSS_SERVICE_DATA_REQ_V02)
    swiQmi_AddIndicationHandler(GnssDataDeleteHandler, QMI_SERVICE_LOC,
                                QMI_LOC_DELETE_GNSS_SERVICE_DATA_IND_V02, NULL);
#else
    LE_ERROR("Clear Data service mode not supported");
#endif

    // Register our own handler with the QMI LOC service.
    swiQmi_AddIndicationHandler(SetSuplAssitedModeHandler, QMI_SERVICE_LOC,
                                QMI_LOC_SET_OPERATION_MODE_REQ_V02, NULL);

    // Register our own handler with the QMI LOC service.
    swiQmi_AddIndicationHandler(SetSuplServerConfigHandler, QMI_SERVICE_LOC,
                                QMI_LOC_SET_SERVER_REQ_V02, NULL);

    // Register our own handler with the QMI LOC service.
    swiQmi_AddIndicationHandler(InjectSuplCertificateHandler, QMI_SERVICE_LOC,
                                QMI_LOC_INJECT_SUPL_CERTIFICATE_REQ_V02, NULL);

    // Register our own handler with the QMI LOC service.
    swiQmi_AddIndicationHandler(DeleteSuplCertificateHandler, QMI_SERVICE_LOC,
                                QMI_LOC_DELETE_SUPL_CERTIFICATE_REQ_V02, NULL);

    // Register our own handlers with the QMI LOC service.
    swiQmi_AddIndicationHandler(MeasurementReportHandler, QMI_SERVICE_LOC,
                                QMI_LOC_EVENT_GNSS_MEASUREMENT_REPORT_IND_V02,NULL);

    // Register our own handler with the QMI LOC service.
    swiQmi_AddIndicationHandler(SetSatMeasurementReportHandler, QMI_SERVICE_LOC,
                                QMI_LOC_SET_GNSS_CONSTELL_REPORT_CONFIG_IND_V02, NULL);

    // Register our own handler with the QMI LOC service.
    swiQmi_AddIndicationHandler(NmeaHandler, QMI_SERVICE_LOC,
                                QMI_LOC_EVENT_NMEA_IND_V02, NULL);

    // Register our own handlers with the QMI LOC service.
    swiQmi_AddIndicationHandler(EngineStatusHandler, QMI_SERVICE_LOC,
                                QMI_LOC_EVENT_ENGINE_STATE_IND_V02,NULL);

    // Register our own handler with the QMI LOC service.
    swiQmi_AddIndicationHandler(SetNmeaSentencesHandler, QMI_SERVICE_LOC,
                                QMI_LOC_SET_NMEA_TYPES_REQ_V02, NULL);

    // Register our own handler with the QMI LOC service.
    swiQmi_AddIndicationHandler(GetNmeaSentencesHandler, QMI_SERVICE_LOC,
                                QMI_LOC_GET_NMEA_TYPES_REQ_V02, NULL);

    // Register our own handler with the QMI LOC service.
    swiQmi_AddIndicationHandler(RegEventsHandler, QMI_SERVICE_LOC,
                                QMI_LOC_GET_REGISTERED_EVENTS_IND_V02,NULL);

    // Set the indications to receive for the LOC service.
    result = SetRegEvents(true,
                          QMI_LOC_EVENT_MASK_POSITION_REPORT_V02
                        | QMI_LOC_EVENT_MASK_GNSS_SV_INFO_V02
                        | QMI_LOC_EVENT_MASK_GNSS_MEASUREMENT_REPORT_V02
                        | QMI_LOC_EVENT_MASK_ENGINE_STATE_V02
                        | QMI_LOC_EVENT_MASK_INJECT_TIME_REQ_V02);

    /* GNSS context initialization */
    // Update position status
    GnssCtx.firstFix = false;
    // Clear TTFF value
    GnssCtx.ttff.sec = 0x00;
    GnssCtx.ttff.usec = 0x00;

    //  NMEA handler counter initialization
    NmeaHandlerCounter = 0;

    // Activate Satellites measurement report
    SetSatMeasurementReport();

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to release the PA GNSS Module.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_Release
(
    void
)
{
    //@todo Check if we need to do anything here.
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the GNSS constellation bit mask
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED request not supported
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_SetConstellation
(
    le_gnss_ConstellationBitMask_t constellationMask  ///< [IN] GNSS constellation used in solution.
)
{
    le_result_t result = LE_FAULT;

#if defined(QMI_SWI_M2M_GNSS_SET_GNSSCONFIG_REQ_V01)

    SWIQMI_DECLARE_MESSAGE(swi_m2m_gnss_set_gnssconfig_req_msg_v01, setGnssConfigReq);
    SWIQMI_DECLARE_MESSAGE(swi_m2m_gnss_set_gnssconfig_resp_msg_v01, setGnssConfigRsp);

    LE_DEBUG("Set GNSS constellation 0x%X", constellationMask);

    // enable the satellites constellations according to the constellation mask
    setGnssConfigReq.gps_valid = true;
    if (constellationMask & LE_GNSS_CONSTELLATION_GPS)
    {
        setGnssConfigReq.gps = LE_GNSS_WORLDWIDE_AREA;
    }
    else
    {
        LE_ERROR("GPS Constellation is not activated");
        return LE_UNSUPPORTED;
    }

    if (constellationMask & LE_GNSS_CONSTELLATION_SBAS)
    {
        LE_WARN("SBAS Constellation setting is not supported");
        return LE_UNSUPPORTED;
    }

    setGnssConfigReq.glonass_valid = true;
    if (constellationMask & LE_GNSS_CONSTELLATION_GLONASS)
    {
        setGnssConfigReq.glonass = LE_GNSS_WORLDWIDE_AREA;
    }

    setGnssConfigReq.bds_valid = true;
    if (constellationMask & LE_GNSS_CONSTELLATION_BEIDOU)
    {
        setGnssConfigReq.bds = LE_GNSS_WORLDWIDE_AREA;
    }

    setGnssConfigReq.gal_valid = true;
    if (constellationMask & LE_GNSS_CONSTELLATION_GALILEO)
    {
        setGnssConfigReq.gal = LE_GNSS_WORLDWIDE_AREA;
    }

#if defined(C_RCVR_GNSS_CONFIG_QZSS_ENABLED)
    setGnssConfigReq.qzss_valid = true;
    if (constellationMask & LE_GNSS_CONSTELLATION_QZSS)
    {
        setGnssConfigReq.qzss = LE_GNSS_WORLDWIDE_AREA;
    }
#endif
    // Send QMI message
    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(GeneralClient,
                                         QMI_SWI_M2M_GNSS_SET_GNSSCONFIG_REQ_V01,
                                         &setGnssConfigReq, sizeof(setGnssConfigReq),
                                         &setGnssConfigRsp, sizeof(setGnssConfigRsp),
                                         COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK == swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_GNSS_SET_GNSSCONFIG_REQ_V01),
                                      clientMsgErr,
                                      setGnssConfigRsp.resp.result,
                                      setGnssConfigRsp.resp.error))
    {
        LE_DEBUG("Set GNSS constellation done");
        result = LE_OK;
    }
    else
    {
        LE_ERROR("Set GNSS constellation error");
    }

#elif defined(QMI_SWI_M2M_GPS_SET_GPSONLY_REQ_V01)

    swi_m2m_gps_set_gpsonly_req_msg_v01 setGpsOnlyReq = {0};
    swi_m2m_gps_set_gpsonly_resp_msg_v01 setGpsOnlyRsp = { {0} };

    LE_DEBUG("Set GNSS constellation 0x%X", constellationMask);

    // Set the constellation mode
    switch((int)constellationMask) // TODO: Low priority, remove (int) cast operator.
    {
        case LE_GNSS_CONSTELLATION_GPS:
        {
            setGpsOnlyReq.GPSOnly = 1; // GPS Only;
        }
        break;

        case (LE_GNSS_CONSTELLATION_GPS | LE_GNSS_CONSTELLATION_GLONASS):
        {
            setGpsOnlyReq.GPSOnly = 0; // GPS+GLONASS
        }
        break;
        default:
        {
            LE_ERROR("Constellation mode not supported");
            return LE_UNSUPPORTED;
        }
    }

    // Send QMI message
    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(GeneralClient,
                                         QMI_SWI_M2M_GPS_SET_GPSONLY_REQ_V01,
                                         &setGpsOnlyReq, sizeof(setGpsOnlyReq),
                                         &setGpsOnlyRsp, sizeof(setGpsOnlyRsp),
                                         COMM_DEFAULT_PLATFORM_TIMEOUT);

    // Check the response
    if (LE_OK != swiQmi_OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_M2M_GPS_SET_GPSONLY_REQ_V01),
                                             clientMsgErr,
                                             setGpsOnlyRsp.resp))
    {
        LE_ERROR("Set GNSS constellation error");
    }
    else
    {
        LE_DEBUG("Set GNSS constellation done");
        result = LE_OK;
    }

#else
    LE_ERROR("Constellation mode not supported");
#endif

    return result;

}

//--------------------------------------------------------------------------------------------------
/**
 * Get the GNSS constellation bit mask
 *
* @return
*  - LE_OK on success
*  - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_GetConstellation
(
    le_gnss_ConstellationBitMask_t* constellationMaskPtr ///< [OUT] GNSS constellation used in
                                                         ///< solution
)
{
    le_result_t result = LE_OK;

#if defined(QMI_SWI_M2M_GNSS_GET_GNSSCONFIG_REQ_V01)
    // swi_m2m_gnss_get_gnssconfig_req_msg_v01 is empty
    SWIQMI_DECLARE_MESSAGE(swi_m2m_gnss_get_gnssconfig_resp_msg_v01, getGnssConfigResp);

    // Send QMI message
    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(GeneralClient,
                                         QMI_SWI_M2M_GNSS_GET_GNSSCONFIG_REQ_V01,
                                         NULL, 0,
                                         &getGnssConfigResp, sizeof(getGnssConfigResp),
                                         COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_GNSS_GET_GNSSCONFIG_REQ_V01),
                                      clientMsgErr,
                                      getGnssConfigResp.resp.result,
                                      getGnssConfigResp.resp.error))
    {
        LE_ERROR("Get GNSS constellation error");
        return LE_FAULT;
    }

    *constellationMaskPtr = 0;

    // Get the GPS constellation mode
    if (getGnssConfigResp.gps_valid && (LE_GNSS_WORLDWIDE_AREA == getGnssConfigResp.gps))
    {
        *constellationMaskPtr |= (uint32_t)LE_GNSS_CONSTELLATION_GPS;
    }

    // Get the GLONASS constellation mode
    if (getGnssConfigResp.glonass_valid && (LE_GNSS_WORLDWIDE_AREA == getGnssConfigResp.glonass))
    {
        *constellationMaskPtr |= (uint32_t)LE_GNSS_CONSTELLATION_GLONASS;
    }

    // Get the BEIDOU constellation mode
    if (getGnssConfigResp.bds_valid &&
        ((LE_GNSS_WORLDWIDE_AREA == getGnssConfigResp.bds) ||
        (LE_GNSS_OUTSIDE_US_AREA == getGnssConfigResp.bds))
       )
    {
        *constellationMaskPtr |= (uint32_t)LE_GNSS_CONSTELLATION_BEIDOU;
    }

    // Get the GALILEO constellation mode
    if (getGnssConfigResp.gal_valid &&
        ((LE_GNSS_WORLDWIDE_AREA == getGnssConfigResp.gal) ||
        (LE_GNSS_OUTSIDE_US_AREA == getGnssConfigResp.gal))
       )
    {
        *constellationMaskPtr |= (uint32_t)LE_GNSS_CONSTELLATION_GALILEO;
    }

#if defined(C_RCVR_GNSS_CONFIG_QZSS_ENABLED)
    // Get the QZSS constellation mode
    if (getGnssConfigResp.qzss_valid &&
        ((LE_GNSS_WORLDWIDE_AREA == getGnssConfigResp.qzss) ||
        (LE_GNSS_OUTSIDE_US_AREA == getGnssConfigResp.qzss))
       )
    {
        *constellationMaskPtr |= (uint32_t)LE_GNSS_CONSTELLATION_QZSS;
    }
#endif
    LE_DEBUG("Get GNSS constellation 0x%X", *constellationMaskPtr);

#elif defined(QMI_SWI_M2M_GPS_GET_GPSONLY_REQ_V01)

    // swi_m2m_gps_get_gpsonly_req_msg is empty
    SWIQMI_DECLARE_MESSAGE(swi_m2m_gps_get_gpsonly_resp_msg_v01, getGpsOnlyResp);

    // Send QMI message
    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(GeneralClient,
                                         QMI_SWI_M2M_GPS_GET_GPSONLY_REQ_V01,
                                         NULL, 0,
                                         &getGpsOnlyResp, sizeof(getGpsOnlyResp),
                                         COMM_DEFAULT_PLATFORM_TIMEOUT);

    // Check the response
    if ( swiQmi_OEMCheckResponseCode(STRINGIZE_EXPAND(QMI_SWI_M2M_GPS_GET_GPSONLY_REQ_V01),
                                     clientMsgErr,
                                     getGpsOnlyResp.resp) != LE_OK )
    {
        LE_ERROR("Get GNSS constellation error");
        return LE_FAULT;
    }

    // Get the constellation mode
    switch(getGpsOnlyResp.GPSOnly)
    {
        case 1: // GPS Only
        {
            *constellationMaskPtr = LE_GNSS_CONSTELLATION_GPS;
        }break;

        case 0: // GPS+GLONASS
        {
            *constellationMaskPtr = (LE_GNSS_CONSTELLATION_GPS | LE_GNSS_CONSTELLATION_GLONASS);
        }break;

        default:
        {
            LE_ERROR("Constellation mode not supported");
            result = LE_FAULT;
        }
        LE_DEBUG("Get GNSS constellation 0x%X", *constellationMaskPtr);
    }

#else
     LE_ERROR("Constellation mode not supported");
     result = LE_FAULT;
#endif

     return result;
}


#if defined(QMI_SWI_M2M_GNSS_SET_GNSSCONFIG_REQ_V01)
//--------------------------------------------------------------------------------------------------
/**
 * Set the area for the GNSS constellation
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED request not supported
 *  - LE_BAD_PARAMETER on invalid constellation area
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_SetConstellationArea
(
    le_gnss_Constellation_t satConstellation,       ///< [IN] GNSS constellation used in solution.
    le_gnss_ConstellationArea_t constellationArea   ///< [IN] GNSS constellation area.
)
{
    if ((LE_GNSS_WORLDWIDE_AREA != constellationArea) &&
        (LE_GNSS_OUTSIDE_US_AREA != constellationArea))
    {
        LE_ERROR("Invalid constellation area %d", constellationArea);
        return LE_BAD_PARAMETER;
    }

    // Get GNSS constellation
    SWIQMI_DECLARE_MESSAGE(swi_m2m_gnss_get_gnssconfig_resp_msg_v01, getGnssConfigResp);

    // Send QMI message
    qmi_client_error_type clientMsgGetErr = qmi_client_send_msg_sync(GeneralClient,
                                         QMI_SWI_M2M_GNSS_GET_GNSSCONFIG_REQ_V01,
                                         NULL, 0,
                                         &getGnssConfigResp, sizeof(getGnssConfigResp),
                                         COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_GNSS_GET_GNSSCONFIG_REQ_V01),
                                      clientMsgGetErr,
                                      getGnssConfigResp.resp.result,
                                      getGnssConfigResp.resp.error))
    {
        LE_ERROR("Get GNSS constellation error");
        return LE_FAULT;
    }

    uint8_t glonassConst=0;
    uint8_t beidouConst=0;
    uint8_t galileoConst=0;
#if defined(C_RCVR_GNSS_CONFIG_QZSS_ENABLED)
    uint8_t qzssConst=0;
#endif
    // Get the GLONASS constellation mode
    if (getGnssConfigResp.glonass_valid)
    {
        glonassConst = getGnssConfigResp.glonass;
    }

    // Get the BEIDOU constellation mode
    if (getGnssConfigResp.bds_valid)
    {
        beidouConst = getGnssConfigResp.bds;
    }

    // Get the GALILEO constellation mode
    if (getGnssConfigResp.gal_valid)
    {
       galileoConst = getGnssConfigResp.gal;
    }

    // Get the QZSS constellation mode
#if defined(C_RCVR_GNSS_CONFIG_QZSS_ENABLED)
    if (getGnssConfigResp.qzss_valid)
    {
        qzssConst = getGnssConfigResp.qzss;
    }
#endif

    // set the constellation area
    SWIQMI_DECLARE_MESSAGE(swi_m2m_gnss_set_gnssconfig_req_msg_v01, setGnssConfigReq);
    SWIQMI_DECLARE_MESSAGE(swi_m2m_gnss_set_gnssconfig_resp_msg_v01, setGnssConfigRsp);

    setGnssConfigReq.gps_valid = true;
    setGnssConfigReq.gps  = LE_GNSS_WORLDWIDE_AREA;

    setGnssConfigReq.glonass_valid = true;
    setGnssConfigReq.glonass  = glonassConst;

    setGnssConfigReq.bds_valid = true;
    setGnssConfigReq.bds  = beidouConst;

    setGnssConfigReq.gal_valid = true;
    setGnssConfigReq.gal  = galileoConst;

#if defined(C_RCVR_GNSS_CONFIG_QZSS_ENABLED)
    setGnssConfigReq.qzss_valid = true;
    setGnssConfigReq.qzss = qzssConst;
#endif

    switch (satConstellation)
    {
        case LE_GNSS_SV_CONSTELLATION_GPS:
        case LE_GNSS_SV_CONSTELLATION_GLONASS:
        {
            if (LE_GNSS_OUTSIDE_US_AREA == (uint8_t)constellationArea)
            {
                LE_ERROR("Area %d is not supported", constellationArea);
                return LE_UNSUPPORTED;
            }
            setGnssConfigReq.gps = LE_GNSS_WORLDWIDE_AREA;
            setGnssConfigReq.glonass = LE_GNSS_WORLDWIDE_AREA;
        }
        break;
        case LE_GNSS_SV_CONSTELLATION_BEIDOU:
        {
            setGnssConfigReq.bds = (uint8_t)constellationArea;
        }
        break;
        case LE_GNSS_SV_CONSTELLATION_GALILEO:
        {
            setGnssConfigReq.gal = (uint8_t)constellationArea;
        }
        break;
#if defined(C_RCVR_GNSS_CONFIG_QZSS_ENABLED)
        case LE_GNSS_SV_CONSTELLATION_QZSS:
        {
            setGnssConfigReq.qzss = (uint8_t)constellationArea;
        }
        break;
#endif
        default:
        {
            LE_ERROR("Constellation area for %d is not supported", satConstellation);
            return LE_UNSUPPORTED;
        }
    };

    // Send QMI message
    qmi_client_error_type clientSetMsgErr = qmi_client_send_msg_sync(GeneralClient,
                                         QMI_SWI_M2M_GNSS_SET_GNSSCONFIG_REQ_V01,
                                         &setGnssConfigReq, sizeof(setGnssConfigReq),
                                         &setGnssConfigRsp, sizeof(setGnssConfigRsp),
                                         COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK == swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_GNSS_SET_GNSSCONFIG_REQ_V01),
                                      clientSetMsgErr,
                                      setGnssConfigRsp.resp.result,
                                      setGnssConfigRsp.resp.error))
    {
        LE_DEBUG("Set GNSS constellation area done");
        return LE_OK;
    }

    LE_ERROR("Set GNSS constellation area error");
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the area for the GNSS constellation
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED request not supported
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_GetConstellationArea
(
    le_gnss_Constellation_t satConstellation,         ///< [IN] GNSS constellation used in solution.
    le_gnss_ConstellationArea_t* constellationAreaPtr ///< [OUT] GNSS constellation area.
)
{
    if (NULL == constellationAreaPtr)
    {
        LE_KILL_CLIENT("Pointer is NULL !");
        return LE_FAULT;
    }

    // swi_m2m_gnss_get_gnssconfig_req_msg_v01 is empty
    SWIQMI_DECLARE_MESSAGE(swi_m2m_gnss_get_gnssconfig_resp_msg_v01, getGnssConfigResp);

    // Send QMI message
    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(GeneralClient,
                                         QMI_SWI_M2M_GNSS_GET_GNSSCONFIG_REQ_V01,
                                         NULL, 0,
                                         &getGnssConfigResp, sizeof(getGnssConfigResp),
                                         COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_GNSS_GET_GNSSCONFIG_REQ_V01),
                                      clientMsgErr,
                                      getGnssConfigResp.resp.result,
                                      getGnssConfigResp.resp.error))
    {
        LE_ERROR("Get GNSS constellation area error");
        return LE_FAULT;
    }
    *constellationAreaPtr = 0;

    switch(satConstellation)
    {
        case LE_GNSS_SV_CONSTELLATION_GPS:
        {
            if (true == getGnssConfigResp.gps_valid)
            {
                *constellationAreaPtr = (le_gnss_ConstellationArea_t)getGnssConfigResp.gps;
            }
            else
            {
                LE_ERROR("Constellation %d not set", satConstellation);
                return LE_FAULT;
            }
        }
        break;

        case LE_GNSS_SV_CONSTELLATION_GLONASS:
        {
            if (true == getGnssConfigResp.glonass_valid)
            {
                *constellationAreaPtr = (le_gnss_ConstellationArea_t)getGnssConfigResp.glonass;
            }
            else
            {
                LE_ERROR("Constellation %d not set", satConstellation);
                return LE_FAULT;
            }
        }
        break;


        case LE_GNSS_SV_CONSTELLATION_BEIDOU:
        {
            if (true == getGnssConfigResp.bds_valid)
            {
                *constellationAreaPtr = (le_gnss_ConstellationArea_t)getGnssConfigResp.bds;
            }
            else
            {
                LE_ERROR("Constellation %d not set", satConstellation);
                return LE_FAULT;
            }
        }
        break;
        case LE_GNSS_SV_CONSTELLATION_GALILEO:
        {
            if (true == getGnssConfigResp.gal_valid)
            {
                *constellationAreaPtr = (le_gnss_ConstellationArea_t)getGnssConfigResp.gal;
            }
            else
            {
                LE_ERROR("Constellation %d not set", satConstellation);
                return LE_FAULT;
            }
        }
        break;
#if defined(C_RCVR_GNSS_CONFIG_QZSS_ENABLED)
        case LE_GNSS_SV_CONSTELLATION_QZSS:
        {
            if (true == getGnssConfigResp.qzss_valid)
            {
                *constellationAreaPtr = (le_gnss_ConstellationArea_t)getGnssConfigResp.qzss;
            }
            else
            {
                LE_ERROR("Constellation %d not set", satConstellation);
                return LE_FAULT;
            }
        }
        break;
#endif
        default:
        {
            LE_ERROR("Constellation area for %d is not supported", satConstellation);
            return LE_UNSUPPORTED;
        }
    };

    return LE_OK;
}
#endif

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to start the GNSS acquisition.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_Start
(
    void
)
{
    le_result_t result;
    char semName[64];
    qmiLocStartReqMsgT_v02 startReq = {0};
    qmiLocGenRespMsgT_v02 genResp = { {0} };
    pa_gnss_LocSync_t *locSyncPtr =
               &FunctionSync[PA_GNSS_APISYNC_QMI_LOC_EVENT_ENGINE_STATE_IND_V02];
    qmiLocEngineStateEnumT_v02 gnssEngineState = 0;

    // Initialize SV info before starting and restarting the GNSS engine.
    InitializeSatUsedInfo(&PositionData);
    InitializeSatInfo(&PositionData);

    // Initialize the GNSS engine state.
    locSyncPtr->engineState = 0;

    LE_DEBUG("Start GNSS");

    // The same session ID must be used to stop the acquisition.
    startReq.sessionId = SESSION_ID;

    // Set the acquisition to periodic rather then a single acquisition.
    startReq.fixRecurrence_valid = true;
    startReq.fixRecurrence = eQMI_LOC_RECURRENCE_PERIODIC_V02;

    // Set the acquisition rate.
    startReq.minInterval_valid = true;
    startReq.minInterval = AcqRate; // in milliseconds.

    snprintf(semName,sizeof(semName),"GnssFuncSem-%d",
                 PA_GNSS_APISYNC_QMI_LOC_EVENT_ENGINE_STATE_IND_V02);
    locSyncPtr->semaphoreRef = le_sem_Create(semName,0);

    gnssEngineState = locSyncPtr->gnssEngineState;

    // Start the command
    StartCommand(locSyncPtr);

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(LocClient,
                                                        QMI_LOC_START_REQ_V02,
                                                        &startReq, sizeof(startReq),
                                                        &genResp, sizeof(genResp),
                                                        COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_LOC_START_REQ_V02),
                                    clientMsgErr,
                                    genResp.resp) == LE_OK)
    {
        // Do not wait on locSyncPtr->semaphoreRef if the GNSS engine has already been started by
        // another QMI client. The notication for the QMI message QMI_LOC_START_REQ
        // (in EngineStatusHandler) won't comes out in  in this case.

        // if gnssEngineState == eQMI_LOC_ENGINE_STATE_START_V02 don't launch le_sem_WaitWithTimeOut
        // if gnssEngineState == eQMI_LOC_ENGINE_STATE_STOP_V02 and the handler notif comes before
        // launching le_sem_WaitWithTimeOut ; the post is already done (with ok or error).
        if (eQMI_LOC_ENGINE_STATE_OFF_V02 == gnssEngineState)
        {
            le_result_t semResult;
            le_clk_Time_t timeToWait = { .sec= TIMEOUT_SEC, .usec= TIMEOUT_USEC };

            locSyncPtr->engineState = START_REQUEST_PATTERN;
            semResult = le_sem_WaitWithTimeOut(locSyncPtr->semaphoreRef,timeToWait);
            if (semResult != LE_OK)
            {
                LE_ERROR("ERROR le_sem_WaitWithTimeOut %d", semResult);
                LE_ERROR("ERROR EngineState is not ON.  %d", locSyncPtr->engineState);
                result = LE_FAULT;
                goto out;
            }

            if (locSyncPtr->result != LE_OK)
            {
                LE_ERROR("Error for PA_GNSS_APISYNC_QMI_LOC_EVENT_ENGINE_STATE_IND_V02, error = %i"
                      , locSyncPtr->result);
                result = LE_FAULT;
                goto out;
            }

            // check that GNSS is really ON
            if(eQMI_LOC_ENGINE_STATE_ON_V02 != locSyncPtr->engineState)
            {
                LE_ERROR("ERROR EngineState is not ON.  %d", locSyncPtr->engineState);
                result = LE_FAULT;
                goto out;
            }
        }

        LE_INFO("EngineState ON");

        // Update position status
        GnssCtx.firstFix = false;
        // Clear TTFF value
        GnssCtx.ttff.sec = 0x00;
        GnssCtx.ttff.usec = 0x00;
        // TTFF Start time
        GnssCtx.acqStartTime = le_clk_GetRelativeTime();
        result = LE_OK;
    }
    else
    {
        LE_ERROR("Error for PA_GNSS_APISYNC_QMI_LOC_EVENT_ENGINE_STATE_IND_V02");
        result = LE_FAULT;
    }

out:
    StopCommand(locSyncPtr);
    le_sem_Delete(locSyncPtr->semaphoreRef);
    locSyncPtr->semaphoreRef = NULL;
    return result;
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to stop the GNSS acquisition.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_Stop
(
    void
)
{
    qmiLocStopReqMsgT_v02 stopReq = {0};
    qmiLocGenRespMsgT_v02 genResp = { {0} };
    pa_gnss_LocSync_t *locSyncPtr =
        &FunctionSync[PA_GNSS_APISYNC_QMI_LOC_EVENT_ENGINE_STATE_IND_V02];

    LE_DEBUG("Stop GNSS");

    // Initialize the GNSS engine state.
    locSyncPtr->engineState = 0;

    // Use the same session ID that was used to start the acquisition.
    stopReq.sessionId = SESSION_ID;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(LocClient,
                                                        QMI_LOC_STOP_REQ_V02,
                                                        &stopReq, sizeof(stopReq),
                                                        &genResp, sizeof(genResp),
                                                        COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_LOC_STOP_REQ_V02),
                                    clientMsgErr,
                                    genResp.resp) == LE_OK)
    {
        LE_DEBUG("EngineState is now OFF");
        return LE_OK;
    }
    else
    {
        LE_ERROR("Error for PA_GNSS_APISYNC_QMI_LOC_EVENT_ENGINE_STATE_IND_V02");
        return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function enables the GNSS device.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_Enable
(
    void
)
{
    LE_DEBUG("Enable GNSS");
    // Nothing to do.
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function disables the GNSS device.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_Disable
(
    void
)
{
    qmiLocStopReqMsgT_v02 stopReq = {0};
    qmiLocGenRespMsgT_v02 genResp = { {0} };

    LE_DEBUG("Stop all GNSS sessions");

    // Use the session ID 255 to deactivate all ongoing GNSS sessions.
    stopReq.sessionId = 255;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(LocClient,
                                                        QMI_LOC_STOP_REQ_V02,
                                                        &stopReq, sizeof(stopReq),
                                                        &genResp, sizeof(genResp),
                                                        COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_LOC_STOP_REQ_V02),
                                    clientMsgErr,
                                    genResp.resp) == LE_OK)
    {
        return LE_OK;
    }
    else
    {
        return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function sets the GNSS device acquisition rate.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED request not supported
 *  - LE_TIMEOUT a time-out occurred
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_SetAcquisitionRate
(
    uint32_t rate     ///< [IN] rate in milliseconds
)
{
    AcqRate = rate;

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the rate of GNSS fix reception
 *
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_GetAcquisitionRate
(
    uint32_t* ratePtr     ///< [IN] rate in milliseconds
)
{
    *ratePtr = AcqRate;

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register an handler for GNSS position data notifications.
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_gnss_AddPositionDataHandler
(
    pa_gnss_PositionDataHandlerFunc_t handler ///< [IN] The handler function.
)
{
    if (handler == NULL)
    {
        LE_FATAL("The new Position Data handler is NULL");
    }

    LE_DEBUG("Set new Position Data handler");

    return le_event_AddHandler("PositionHandler", PositionEvent, (le_event_HandlerFunc_t)handler);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to remove a handler for GNSS position data notifications.
 */
//--------------------------------------------------------------------------------------------------
void pa_gnss_RemovePositionDataHandler
(
    le_event_HandlerRef_t    handlerRef ///< [IN] The handler reference.
)
{
    le_event_RemoveHandler(handlerRef);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register an handler for NMEA frames notifications.
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_gnss_AddNmeaHandler
(
    pa_gnss_NmeaHandlerFunc_t handler ///< [IN] The handler function.
)
{
    if (handler == NULL)
    {
        LE_FATAL("The new NMEA handler is NULL");
    }

    LE_DEBUG("Add NMEA handler %d", NmeaHandlerCounter);

    // Check added NMEA Handler(s)
    if (!NmeaHandlerCounter)
    {
        // Set the indications to receive for MEA reports for position and satellites in view
        SetRegEvents(true, QMI_LOC_EVENT_MASK_NMEA_V02);
    }

    //  Update NMEA handler counter
    NmeaHandlerCounter++;

    return le_event_AddHandler("NmeaHandler", NmeaEvent, (le_event_HandlerFunc_t)handler);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to remove a handler for NMEA frames notifications.
 */
//--------------------------------------------------------------------------------------------------
void pa_gnss_RemoveNmeaHandler
(
    le_event_HandlerRef_t    handlerRef ///< [IN] The handler reference.
)
{

    LE_DEBUG("Remove NMEA handler %d", NmeaHandlerCounter);

    //  Update NMEA handler counter
    NmeaHandlerCounter--;

    // Check added NMEA Handler(s)
    if (!NmeaHandlerCounter)
    {
        // Disable the indications to receive for MEA reports for position and satellites in view
        SetRegEvents(false, QMI_LOC_EVENT_MASK_NMEA_V02);
    }

    le_event_RemoveHandler(handlerRef);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to load an 'Extended Ephemeris' file into the GNSS device.
 *
 * @return LE_FAULT         The function failed to inject the 'Extended Ephemeris' file.
 * @return LE_TIMEOUT       A time-out occurred.
 * @return LE_FORMAT_ERROR  'Extended Ephemeris' file format error.
 * @return LE_OK            The function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_LoadExtendedEphemerisFile
(
    int32_t       fd      ///< [IN] extended ephemeris file descriptor
)
{
    le_result_t result;
    uint32_t totalSize;
    uint32_t totalPart;
    uint32_t totalPartToReceived;
    uint32_t currentPart;
    struct stat buf;
    pa_gnss_LocSync_t *locSyncPtr =
                       &FunctionSync[PA_GNSS_APISYNC_QMI_LOC_INJECT_PREDICTED_ORBITS_DATA_REQ_V02];
    le_clk_Time_t timeToWait = { .sec= TIMEOUT_SEC, .usec= TIMEOUT_USEC };

    // test if no file
    if (fd == -1)
    {
        LE_ERROR("Bad extended ephemeris file descriptor");
        return LE_FAULT;
    }

    // Get file size.
    fstat(fd, &buf);
    totalSize = buf.st_size;

    if (totalSize % QMI_LOC_MAX_PREDICTED_ORBITS_PART_LEN_V02)
    {
        totalPart = (totalSize/QMI_LOC_MAX_PREDICTED_ORBITS_PART_LEN_V02)+1;
    }
    else
    {
        totalPart = totalSize/QMI_LOC_MAX_PREDICTED_ORBITS_PART_LEN_V02;
    }

    // Set the number of data to receive
    totalPartToReceived = totalPart;
    locSyncPtr->dataPtr = &totalPartToReceived;

    // Command is in progress
    StartCommand(locSyncPtr);

    for (currentPart = 1;
         currentPart <= totalPart;
         currentPart++)
    {
        qmiLocInjectPredictedOrbitsDataReqMsgT_v02 qmiLocInjectDataReq = {0};
        qmiLocGenRespMsgT_v02 genResp = { {0} };

        qmiLocInjectDataReq.totalSize = totalSize;
        qmiLocInjectDataReq.totalParts = totalPart;
        qmiLocInjectDataReq.partNum = currentPart;
        qmiLocInjectDataReq.partData_len = read(fd,
                                                qmiLocInjectDataReq.partData,
                                                QMI_LOC_MAX_PREDICTED_ORBITS_PART_LEN_V02);
        qmiLocInjectDataReq.formatType = eQMI_LOC_PREDICTED_ORBITS_XTRA_V02;
        qmiLocInjectDataReq.formatType_valid = true;

        LE_DEBUG("upload part %d/%db %d/%d %db/%db",
                 qmiLocInjectDataReq.partNum,
                 qmiLocInjectDataReq.partData_len,
                 qmiLocInjectDataReq.partNum,
                 qmiLocInjectDataReq.totalParts,
                 QMI_LOC_MAX_PREDICTED_ORBITS_PART_LEN_V02*(qmiLocInjectDataReq.partNum-1)
                             +qmiLocInjectDataReq.partData_len,
                 qmiLocInjectDataReq.totalSize);

        qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(LocClient,
                                                QMI_LOC_INJECT_PREDICTED_ORBITS_DATA_REQ_V02,
                                                &qmiLocInjectDataReq, sizeof(qmiLocInjectDataReq),
                                                &genResp, sizeof(genResp),
                                                COMM_DEFAULT_PLATFORM_TIMEOUT);

        if (swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_LOC_INJECT_PREDICTED_ORBITS_DATA_REQ_V02),
                                        clientMsgErr,
                                        genResp.resp) != LE_OK)
        {
            StopCommand(locSyncPtr);
            close(fd);
            return LE_FAULT;
        }

        // file blocks synchronisation
        result = le_sem_WaitWithTimeOut(locSyncPtr->semaphoreXtra,timeToWait);
        if (result != LE_OK)
        {
            LE_ERROR("EE file block injection notification timeout");
            StopCommand(locSyncPtr);
            close(fd);
            return LE_TIMEOUT;
        }
    }

    result = le_sem_WaitWithTimeOut(locSyncPtr->semaphoreRef,timeToWait);
    // Command is finished
    StopCommand(locSyncPtr);

    close(fd);

    // Check le_sem_WaitUntil timeOut
    if (result != LE_OK)
    {
        LE_WARN("EE file command timeout");
        return LE_TIMEOUT;
    }

    return (locSyncPtr->result);
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the validity of the last injected Extended Ephemeris.
 *
 * @return LE_FAULT         The function failed to get the validity
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_GetExtendedEphemerisValidity
(
    uint64_t *startTimePtr,    ///< [OUT] Start time in seconds (since Jan. 1, 1970)
    uint64_t *stopTimePtr      ///< [OUT] Stop time in seconds (since Jan. 1, 1970)
)
{
    le_result_t result;
    pa_gnss_LocSync_t *locSyncPtr =
                 &FunctionSync[PA_GNSS_APISYNC_QMI_LOC_GET_PREDICTED_ORBITS_DATA_VALIDITY_REQ_V02];
    qmiLocGetPredictedOrbitsDataValidityIndMsgT_v02 dataValidity = {0};
    qmiLocGenRespMsgT_v02 genResp = { {0} };

    // Set buffer to fill by the handler
    locSyncPtr->dataPtr = &dataValidity;

    // Start the command
    StartCommand(locSyncPtr);

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(LocClient,
                                                QMI_LOC_GET_PREDICTED_ORBITS_DATA_VALIDITY_REQ_V02,
                                                NULL, 0,
                                                &genResp, sizeof(genResp),
                                                COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (swiQmi_CheckResponseCode(STRINGIZE_EXPAND(
                                    QMI_LOC_GET_PREDICTED_ORBITS_DATA_VALIDITY_REQ_V02),
                                    clientMsgErr,
                                    genResp.resp) != LE_OK)
    {
        StopCommand(locSyncPtr);
        return LE_FAULT;
    }

    le_clk_Time_t timer = { .sec= TIMEOUT_SEC, .usec= TIMEOUT_USEC };

    result = le_sem_WaitWithTimeOut(locSyncPtr->semaphoreRef,timer);
    // Command is finished
    StopCommand(locSyncPtr);

    // Check le_sem_WaitUntil timeOut
    if (result != LE_OK)
    {
        LE_WARN("Timeout happen");
        return LE_TIMEOUT;
    }

    // Check synchronization command success
    if ( locSyncPtr->result != LE_OK)
    {
        return locSyncPtr->result;
    }

    if ( dataValidity.validityInfo_valid )
    {
        *startTimePtr = dataValidity.validityInfo.startTimeInUTC;
        *stopTimePtr = *startTimePtr
                        + (dataValidity.validityInfo.durationHours*HOURS_TO_SEC);
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to inject UTC time into the GNSS device.
 *
 * @return
 *  - LE_OK            The function succeeded.
 *  - LE_FAULT         The function failed to inject the UTC time.
 *  - LE_TIMEOUT       A time-out occurred.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_InjectUtcTime
(
    uint64_t timeUtc,      ///< [IN] UTC time since Jan. 1, 1970 in milliseconds
    uint32_t timeUnc       ///< [IN] Time uncertainty in milliseconds
)
{
    le_result_t result;
    pa_gnss_LocSync_t *locSyncPtr =
                 &FunctionSync[PA_GNSS_APISYNC_QMI_LOC_INJECT_UTC_TIME_REQ_V02];

    SWIQMI_DECLARE_MESSAGE(qmiLocInjectUtcTimeReqMsgT_v02, InjectUtcTimeReq);
    SWIQMI_DECLARE_MESSAGE(qmiLocGenRespMsgT_v02, genResp);

    // Start the command
    StartCommand(locSyncPtr);

    InjectUtcTimeReq.timeUtc = timeUtc;
    InjectUtcTimeReq.timeUnc = timeUnc;

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(LocClient,
                                                QMI_LOC_INJECT_UTC_TIME_REQ_V02,
                                                &InjectUtcTimeReq, sizeof(InjectUtcTimeReq),
                                                &genResp, sizeof(genResp),
                                                COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_LOC_INJECT_UTC_TIME_REQ_V02),
                                                       clientMsgErr,
                                                       genResp.resp.result,
                                                       genResp.resp.error))
    {
        StopCommand(locSyncPtr);
        LE_ERROR("Failed to inject UTC time to the GNSS device");
        return LE_FAULT;
    }

    le_clk_Time_t timer = { .sec= TIMEOUT_SEC, .usec= TIMEOUT_USEC };

    result = le_sem_WaitWithTimeOut(locSyncPtr->semaphoreRef,timer);
    // Command is finished
    StopCommand(locSyncPtr);

    // Check le_sem_WaitUntil timeOut
    if (LE_OK != result)
    {
        LE_WARN("Timeout happen");
        return LE_TIMEOUT;
    }

    // Check synchronization command success
    if (LE_OK != locSyncPtr->result)
    {
        LE_ERROR("Failed to inject UTC time to the GNSS device %d", locSyncPtr->result);
        return LE_FAULT;
    }
    LE_INFO("UTC time injected to the GNSS device");
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to restart the GNSS device.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_ForceRestart
(
    pa_gnss_Restart_t  restartType ///< [IN] type of restart
)
{
    le_result_t resultCode;

    // Force Stop GNSS
    if (ForceGnssStop() != LE_OK)
    {
        LE_ERROR("Cannot stop GNSS acquisition!");
    }

    LE_DEBUG("GNSS acquisition is stopped");

    // Clear GNSS service data
    resultCode = ClearDataServices(false, restartType);
    if (resultCode != LE_OK)
    {
        LE_ERROR("Cannot delete GNSS service data (%d)", resultCode);
    }

    // Wait 10 seconds before starting the GNSS in case of warm restart.
    // TODO: This sleep is a temporary workaround. A better solution without a sleep is required.
    if (PA_GNSS_WARM_RESTART == restartType)
    {
        sleep(WAIT_BEFORE_WARM_START_SEC);
    }

    // Start GNSS
    resultCode = pa_gnss_Start();
    if (resultCode != LE_OK)
    {
        LE_ERROR("Cannot start GNSS acquisition!");
    }

    return resultCode;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the TTFF in milliseconds.
 *
 * @return LE_BUSY          The position is not fixed and TTFF can't be measured.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_GetTtff
(
    uint32_t* ttffPtr     ///< [OUT] TTFF in milliseconds
)
{
    le_result_t resultCode = LE_OK;

    if (!GnssCtx.firstFix)
    {
        // TTFF not calculated (Position not fixed)
        LE_WARN("TTFF not available");
        *ttffPtr = 0x00;
        resultCode = LE_BUSY;
    }
    else
    {
        *ttffPtr = ((GnssCtx.ttff.sec * SEC_TO_MSEC) +
                    (GnssCtx.ttff.usec/SEC_TO_MSEC));
    }

    return resultCode;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function sets the SUPL Assisted-GNSS mode.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED request not supported
 *  - LE_TIMEOUT a time-out occurred
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_SetSuplAssistedMode
(
    le_gnss_AssistedMode_t  assistedMode      ///< [IN] Assisted-GNSS mode.
)
{
    le_result_t result = LE_OK;
    qmiLocSetOperationModeReqMsgT_v02 operationModeReq = {0};
    qmiLocGenRespMsgT_v02 genResp = { {0} };
    pa_gnss_LocSync_t *locSyncPtr = &FunctionSync
                                    [PA_GNSS_APISYNC_QMI_LOC_SET_OPERATION_MODE_REQ_V02];
    qmiLocSetOperationModeIndMsgT_v02 operationModeInd = {0};

    LE_DEBUG("SUPL assisted mode %d", assistedMode);

    // Set the SUPL Assisted Mode
    switch(assistedMode)
    {
        case LE_GNSS_STANDALONE_MODE:
        {
            operationModeReq.operationMode = eQMI_LOC_OPER_MODE_STANDALONE_V02;
        }break;
        case LE_GNSS_MS_BASED_MODE:
        {
            operationModeReq.operationMode = eQMI_LOC_OPER_MODE_MSB_V02;
        }break;
        case LE_GNSS_MS_ASSISTED_MODE:
        {
            operationModeReq.operationMode = eQMI_LOC_OPER_MODE_MSA_V02;
        }break;
        default:
        {
            operationModeReq.operationMode = eQMI_LOC_OPER_MODE_DEFAULT_V02;
            LE_ERROR("SUPL assisted mode not supported");
            result = LE_UNSUPPORTED;
        }
    }

    if (result == LE_OK)
    {
        // Set buffer to fill by the handler
        locSyncPtr->dataPtr = &operationModeInd;
        // Start the command
        StartCommand(locSyncPtr);

        qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(LocClient,
                                             QMI_LOC_SET_OPERATION_MODE_REQ_V02,
                                             &operationModeReq, sizeof(operationModeReq),
                                             &genResp, sizeof(genResp),
                                             COMM_DEFAULT_PLATFORM_TIMEOUT);

        if (swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_LOC_SET_OPERATION_MODE_REQ_V02),
                                     clientMsgErr,
                                     genResp.resp) == LE_OK)
        {
            // Wait QMI notification
            le_clk_Time_t timer = { .sec= TIMEOUT_SEC, .usec= TIMEOUT_USEC };
            le_result_t semResult = le_sem_WaitWithTimeOut(locSyncPtr->semaphoreRef,timer);

            // Command is finished
            StopCommand(locSyncPtr);

            // Check le_sem_WaitUntil timeOut
            if (semResult != LE_OK)
            {
                LE_WARN("Timeout happen");
                result = LE_TIMEOUT;
            }
            else if (locSyncPtr->result != LE_OK)
            {
                LE_ERROR("Error for QMI_LOC_SET_OPERATION_MODE_IND_V02, error = %i"
                        , locSyncPtr->result);
                result = LE_FAULT;
            }
            else if (operationModeInd.status == eQMI_LOC_SUCCESS_V02)
            {
                // Update SUPL assisted mode status
                LE_DEBUG("SUPL assisted mode done with status %d", operationModeInd.status);
                SuplAssitedMode = assistedMode;
                result = LE_OK;
            }
            else if (operationModeInd.status == eQMI_LOC_ENGINE_BUSY_V02)
            {
                LE_ERROR("Error for QMI_LOC_SET_OPERATION_MODE_IND_V02, status = %i"
                        , operationModeInd.status);
                result = LE_BUSY;
            }
            else
            {
                LE_ERROR("Error for QMI_LOC_SET_OPERATION_MODE_IND_V02, status = %i"
                        , operationModeInd.status);
                result = LE_FAULT;
            }
        }
        else
        {
            // Command is finished
            LE_ERROR("SUPL assisted mode error");
            StopCommand(locSyncPtr);
            result = LE_FAULT;
        }
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function gets the SUPL Assisted-GNSS mode.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_GetSuplAssistedMode
(
    le_gnss_AssistedMode_t *assistedModePtr      ///< [OUT] Assisted-GNSS mode.
)
{
    LE_DEBUG("SUPL get assisted mode");
    if (assistedModePtr == NULL)
    {
        LE_ERROR("NULL pointer");
        return LE_FAULT;
    }

    // Get the SUPL assisted mode
    *assistedModePtr = SuplAssitedMode;

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function sets the SUPL server URL.
 * That server URL is a NULL-terminated string with a maximum string length (including NULL
 * terminator) equal to 256. Optionally the port number is specified after a colon.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_BUSY service is busy
 *  - LE_TIMEOUT a time-out occurred
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_SetSuplServerUrl
(
    const char*  suplServerUrlPtr      ///< [IN] SUPL server URL.
)
{
    le_result_t result;
    qmiLocSetServerReqMsgT_v02 suplSetServerReq = {0};
    qmiLocGenRespMsgT_v02 genResp = { {0} };
    pa_gnss_LocSync_t *locSyncPtr = &FunctionSync
                                    [PA_GNSS_APISYNC_QMI_LOC_SET_SERVER_REQ_V02];
    qmiLocSetServerIndMsgT_v02 suplSetServerInd = {0};

    if (strlen(suplServerUrlPtr) > LE_GNSS_SUPL_SERVER_URL_MAX_LEN)
    {
        LE_ERROR("Invalid length for SUPL server URL");
        return LE_FAULT;
    }

    LE_DEBUG("SUPL set server URL %s", suplServerUrlPtr);

    // Set SUPL server type
    suplSetServerReq.serverType = eQMI_LOC_SERVER_TYPE_UMTS_SLP_V02; // FW's default value
    // Set SUPL server URL Address
    suplSetServerReq.urlAddr_valid = true;
    strncpy(suplSetServerReq.urlAddr, suplServerUrlPtr, QMI_LOC_MAX_SERVER_ADDR_LENGTH_V02);

    // Set buffer to fill by the handler
    locSyncPtr->dataPtr = &suplSetServerInd;
    // Start the command
    StartCommand(locSyncPtr);

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(LocClient,
                                         QMI_LOC_SET_SERVER_REQ_V02,
                                         &suplSetServerReq, sizeof(suplSetServerReq),
                                         &genResp, sizeof(genResp),
                                         COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_LOC_SET_SERVER_REQ_V02),
                                 clientMsgErr,
                                 genResp.resp) == LE_OK)
    {
        // Wait QMI notification
        le_clk_Time_t timer = { .sec= TIMEOUT_SEC, .usec= TIMEOUT_USEC };
        le_result_t semResult = le_sem_WaitWithTimeOut(locSyncPtr->semaphoreRef,timer);

        // Command is finished
        StopCommand(locSyncPtr);

        // Check le_sem_WaitUntil timeOut
        if (semResult != LE_OK)
        {
            LE_WARN("Timeout happen");
            result = LE_TIMEOUT;
        }
        else if (locSyncPtr->result != LE_OK)
        {
            LE_ERROR("Error for QMI_LOC_SET_SERVER_IND_V02, error = %i"
                    , locSyncPtr->result);
            result = LE_FAULT;
        }
        else if (suplSetServerInd.status == eQMI_LOC_SUCCESS_V02)
        {
            LE_DEBUG("SUPL server URL configured with status %d", suplSetServerInd.status);
            result = LE_OK;
        }
        else if (suplSetServerInd.status == eQMI_LOC_ENGINE_BUSY_V02)
        {
            LE_ERROR("Error for QMI_LOC_SET_SERVER_IND_V02, status = %i", suplSetServerInd.status);
            result = LE_BUSY;
        }
        else
        {
            LE_ERROR("Error for QMI_LOC_SET_SERVER_IND_V02, status = %i", suplSetServerInd.status);
            result = LE_FAULT;
        }
    }
    else
    {
        // Command is finished
        LE_ERROR("SUPL server URL error");
        StopCommand(locSyncPtr);
        result = LE_FAULT;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function injects the SUPL certificate to be used in A-GNSS sessions.
 *
 * @return
 *  - LE_OK on success
 *  - LE_BAD_PARAMETER on invalid parameter
 *  - LE_FAULT on failure
 *  - LE_BUSY service is busy
 *  - LE_TIMEOUT a time-out occurred
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_InjectSuplCertificate
(
    uint8_t  suplCertificateId,      ///< [IN] ID of the SUPL certificate.
                                     ///< Certificate ID range is 0 to 9
    uint16_t suplCertificateLen,     ///< [IN] SUPL certificate size in Bytes.
    const char*  suplCertificatePtr  ///< [IN] SUPL certificate contents.
)
{
    le_result_t result;
    qmiLocInjectSuplCertificateReqMsgT_v02 suplInjectCertificateReq = {0};
    qmiLocGenRespMsgT_v02 genResp = { {0} };
    pa_gnss_LocSync_t *locSyncPtr = &FunctionSync
                                    [PA_GNSS_APISYNC_QMI_LOC_INJECT_SUPL_CERTIFICATE_REQ_V02];
    qmiLocInjectSuplCertificateIndMsgT_v02 suplCertificateInd = {0};

    LE_DEBUG("SUPL inject certificate %d",suplCertificateId);

    // Check input parameters
    if (NULL == suplCertificatePtr)
    {
        LE_ERROR("NULL pointer");
        return LE_BAD_PARAMETER;
    }
    if (suplCertificateId > 9)
    {
        LE_ERROR("Invalid certificate ID %d", suplCertificateId);
        return LE_BAD_PARAMETER;
    }
    if ((suplCertificateLen > QMI_LOC_MAX_SUPL_CERT_LENGTH_V02) ||
        (0 == suplCertificateLen))
    {
        LE_ERROR("Certificate length error %d", suplCertificateLen);
        return LE_BAD_PARAMETER;
    }

    // Set SUPL Certificate
    suplInjectCertificateReq.suplCertId = suplCertificateId;
    suplInjectCertificateReq.suplCertData_len = suplCertificateLen;
    memcpy(suplInjectCertificateReq.suplCertData, suplCertificatePtr, suplCertificateLen);

    // Set buffer to fill by the handler
    locSyncPtr->dataPtr = &suplCertificateInd;
    // Start the command
    StartCommand(locSyncPtr);

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(LocClient,
                                        QMI_LOC_INJECT_SUPL_CERTIFICATE_REQ_V02,
                                        &suplInjectCertificateReq, sizeof(suplInjectCertificateReq),
                                        &genResp, sizeof(genResp),
                                        COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_LOC_INJECT_SUPL_CERTIFICATE_REQ_V02),
                                 clientMsgErr,
                                 genResp.resp) == LE_OK)
    {
        // Wait QMI notification
        le_clk_Time_t timer = { .sec= TIMEOUT_SEC, .usec= TIMEOUT_USEC };
        le_result_t semResult = le_sem_WaitWithTimeOut(locSyncPtr->semaphoreRef,timer);

        // Command is finished
        StopCommand(locSyncPtr);

        // Check le_sem_WaitUntil timeOut
        if (semResult != LE_OK)
        {
            LE_WARN("Timeout happen");
            result = LE_TIMEOUT;
        }
        else if (locSyncPtr->result != LE_OK)
        {
            LE_ERROR("Error for QMI_LOC_INJECT_SUPL_CERTIFICATE_IND_V02, error = %i"
                    , locSyncPtr->result);
            result = LE_FAULT;
        }
        else if (suplCertificateInd.status == eQMI_LOC_SUCCESS_V02)
        {
            LE_DEBUG("SUPL certificate %i injected", suplCertificateId);
            result = LE_OK;
        }
        else if (suplCertificateInd.status == eQMI_LOC_ENGINE_BUSY_V02)
        {
            LE_ERROR("Error for QMI_LOC_INJECT_SUPL_CERTIFICATE_IND_V02, status = %i"
                    , suplCertificateInd.status);
            result = LE_BUSY;
        }
        else
        {
            LE_ERROR("Error for QMI_LOC_INJECT_SUPL_CERTIFICATE_IND_V02, status = %i"
                    , suplCertificateInd.status);
            result = LE_FAULT;
        }
    }
    else
    {
        // Command is finished
        LE_ERROR("SUPL certificate error");
        StopCommand(locSyncPtr);
        result = LE_FAULT;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function deletes the SUPL certificate.
 *
 * @return
 *  - LE_OK on success
 *  - LE_BAD_PARAMETER on invalid parameter
 *  - LE_FAULT on failure
 *  - LE_BUSY service is busy
 *  - LE_TIMEOUT a time-out occurred
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_DeleteSuplCertificate
(
    uint8_t  suplCertificateId  ///< [IN]  ID of the SUPL certificate.
                                ///< Certificate ID range is 0 to 9
)
{
    le_result_t result;
    qmiLocDeleteSuplCertificateReqMsgT_v02 suplDeleteCertificateReq = {0};
    qmiLocGenRespMsgT_v02 genResp = { {0} };
    pa_gnss_LocSync_t *locSyncPtr = &FunctionSync
                                    [PA_GNSS_APISYNC_QMI_LOC_DELETE_SUPL_CERTIFICATE_REQ_V02];
    qmiLocDeleteSuplCertificateIndMsgT_v02 suplDeleteCertificateInd = {0};

    LE_DEBUG("SUPL delete certificate %d", suplCertificateId);

    // Check input parameters
    if (suplCertificateId > 9)
    {
        LE_ERROR("Invalid certificate ID %d", suplCertificateId);
        return LE_BAD_PARAMETER;
    }

    // Set SUPL Certificate
    suplDeleteCertificateReq.suplCertId_valid = true;
    suplDeleteCertificateReq.suplCertId = suplCertificateId;

    // Set buffer to fill by the handler
    locSyncPtr->dataPtr = &suplDeleteCertificateInd;
    // Start the command
    StartCommand(locSyncPtr);

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(LocClient,
                                        QMI_LOC_DELETE_SUPL_CERTIFICATE_REQ_V02,
                                        &suplDeleteCertificateReq, sizeof(suplDeleteCertificateReq),
                                        &genResp, sizeof(genResp),
                                        COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (swiQmi_CheckResponseCode(STRINGIZE_EXPAND(QMI_LOC_DELETE_SUPL_CERTIFICATE_REQ_V02),
                                 clientMsgErr,
                                 genResp.resp) == LE_OK)
    {
        // Wait QMI notification
        le_clk_Time_t timer = { .sec= TIMEOUT_SEC, .usec= TIMEOUT_USEC };
        le_result_t semResult = le_sem_WaitWithTimeOut(locSyncPtr->semaphoreRef,timer);

        // Command is finished
        StopCommand(locSyncPtr);

        // Check le_sem_WaitUntil timeOut
        if (semResult != LE_OK)
        {
            LE_WARN("Timeout happen [%d]", semResult);
            result = LE_TIMEOUT;
        }
        else if (locSyncPtr->result != LE_OK)
        {
            LE_ERROR("Error for QMI_LOC_DELETE_SUPL_CERTIFICATE_IND_V02, error = %i"
                    , locSyncPtr->result);
            result = LE_FAULT;
        }
        else if (suplDeleteCertificateInd.status == eQMI_LOC_SUCCESS_V02)
        {
            LE_DEBUG("SUPL certificate %i deleted", suplCertificateId);
            result = LE_OK;
        }
        else if (suplDeleteCertificateInd.status == eQMI_LOC_ENGINE_BUSY_V02)
        {
            LE_ERROR("Error for QMI_LOC_DELETE_SUPL_CERTIFICATE_IND_V02, status = %i"
                    , suplDeleteCertificateInd.status);
            result = LE_BUSY;
        }
        else
        {
            LE_ERROR("Error for QMI_LOC_DELETE_SUPL_CERTIFICATE_IND_V02, status = %i"
                    , suplDeleteCertificateInd.status);
            result = LE_FAULT;
        }
    }
    else
    {
        // Command is finished
        LE_ERROR("SUPL certificate error");
        StopCommand(locSyncPtr);
        result = LE_FAULT;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the enabled NMEA sentences bit mask
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_BUSY service is busy
 *  - LE_TIMEOUT a time-out occurred
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_SetNmeaSentences
(
    le_gnss_NmeaBitMask_t nmeaMask ///< [IN] Bit mask for enabled NMEA sentences.
)
{
    le_result_t result;
    qmiLocNmeaSentenceMaskT_v02   qmiNmeaMask  = 0;
    qmiLocSetNmeaTypesReqMsgT_v02 nmeaTypesReq = {0};
    qmiLocSetNmeaTypesIndMsgT_v02 nmeaTypesInd = {0};
    qmiLocGenRespMsgT_v02         genResp      = { {0} };
    pa_gnss_LocSync_t *locSyncPtr = &FunctionSync[PA_GNSS_APISYNC_QMI_LOC_SET_NMEA_TYPES_REQ_V02];

    // Convert the bit mask to QMI values
    if (nmeaMask & LE_GNSS_NMEA_MASK_GPGGA)
    {
        qmiNmeaMask |= QMI_LOC_NMEA_MASK_GGA_V02;
    }
    if (nmeaMask & LE_GNSS_NMEA_MASK_GPGSA)
    {
        qmiNmeaMask |= QMI_LOC_NMEA_MASK_GSA_V02;
    }
    if (nmeaMask & LE_GNSS_NMEA_MASK_GPGSV)
    {
        qmiNmeaMask |= QMI_LOC_NMEA_MASK_GSV_V02;
    }
    if (nmeaMask & LE_GNSS_NMEA_MASK_GPRMC)
    {
        qmiNmeaMask |= QMI_LOC_NMEA_MASK_RMC_V02;
    }
    if (nmeaMask & LE_GNSS_NMEA_MASK_GPVTG)
    {
        qmiNmeaMask |= QMI_LOC_NMEA_MASK_VTG_V02;
    }
    if (nmeaMask & LE_GNSS_NMEA_MASK_GLGSV)
    {
        qmiNmeaMask |= QMI_LOC_NMEA_MASK_GLGSV_V02;
    }
    if (nmeaMask & LE_GNSS_NMEA_MASK_GNGNS)
    {
        qmiNmeaMask |= QMI_LOC_NMEA_MASK_GNGNS_V02;
    }
    if (nmeaMask & LE_GNSS_NMEA_MASK_GNGSA)
    {
        qmiNmeaMask |= QMI_LOC_NMEA_MASK_GNGSA_V02;
    }
    if (nmeaMask & LE_GNSS_NMEA_MASK_GAGGA)
    {
        qmiNmeaMask |= QMI_LOC_NMEA_MASK_GAGGA_V02;
    }
    if (nmeaMask & LE_GNSS_NMEA_MASK_GAGSA)
    {
        qmiNmeaMask |= QMI_LOC_NMEA_MASK_GAGSA_V02;
    }
    if (nmeaMask & LE_GNSS_NMEA_MASK_GAGSV)
    {
        qmiNmeaMask |= QMI_LOC_NMEA_MASK_GAGSV_V02;
    }
    if (nmeaMask & LE_GNSS_NMEA_MASK_GARMC)
    {
        qmiNmeaMask |= QMI_LOC_NMEA_MASK_GARMC_V02;
    }
    if (nmeaMask & LE_GNSS_NMEA_MASK_GAVTG)
    {
        qmiNmeaMask |= QMI_LOC_NMEA_MASK_GAVTG_V02;
    }
#if defined(QMI_LOC_NMEA_MASK_PSTIS_V02)
    if (nmeaMask & LE_GNSS_NMEA_MASK_PSTIS)
    {
        qmiNmeaMask |= QMI_LOC_NMEA_MASK_PSTIS_V02;
    }
#endif
    if (nmeaMask & LE_GNSS_NMEA_MASK_PQXFI)
    {
        qmiNmeaMask |= QMI_LOC_NMEA_MASK_PQXFI_V02;
    }

    if (nmeaMask & LE_GNSS_NMEA_MASK_PTYPE)
    {
        qmiNmeaMask |= QMI_LOC_NMEA_MASK_PQXFI_V02;
#if defined(QMI_LOC_NMEA_MASK_PQGSA_V02)
        qmiNmeaMask |= QMI_LOC_NMEA_MASK_PQGSA_V02;
#endif
#if defined(QMI_LOC_NMEA_MASK_PQGSV_V02)
        qmiNmeaMask |= QMI_LOC_NMEA_MASK_PQGSV_V02;
#endif
    }

#if defined(QMI_LOC_NMEA_MASK_GPGRS_V02)
    if (nmeaMask & LE_GNSS_NMEA_MASK_GPGRS)
    {
        qmiNmeaMask |= QMI_LOC_NMEA_MASK_GPGRS_V02;
    }
#endif

#if defined(QMI_LOC_NMEA_MASK_GPGLL_V02)
    if (nmeaMask & LE_GNSS_NMEA_MASK_GPGLL)
    {
        qmiNmeaMask |= QMI_LOC_NMEA_MASK_GPGLL_V02;
    }
#endif

#if defined(QMI_LOC_NMEA_MASK_DEBUG_V02)
    if (nmeaMask & LE_GNSS_NMEA_MASK_DEBUG)
    {
        qmiNmeaMask |= QMI_LOC_NMEA_MASK_DEBUG_V02;
    }
#endif

#if defined(QMI_LOC_NMEA_MASK_GPDTM_V02)
    if (nmeaMask & LE_GNSS_NMEA_MASK_GPDTM)
    {
        qmiNmeaMask |= QMI_LOC_NMEA_MASK_GPDTM_V02;
    }
#endif

#if defined(QMI_LOC_NMEA_MASK_GAGNS_V02)
    if (nmeaMask & LE_GNSS_NMEA_MASK_GAGNS)
    {
        qmiNmeaMask |= QMI_LOC_NMEA_MASK_GAGNS_V02;
    }
#endif
    // ToDo: Complete the conversion with new values when FW bit mask is extended

    // Set new bit mask to apply
    nmeaTypesReq.nmeaSentenceType = qmiNmeaMask;

    LE_DEBUG("Set NMEA sentences: Legato bit mask 0x%08X -> QMI bit mask 0x%08X",
              nmeaMask, qmiNmeaMask);

    // Set buffer to fill by the handler
    locSyncPtr->dataPtr = &nmeaTypesInd;
    // Start the command
    StartCommand(locSyncPtr);

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(LocClient,
                                             QMI_LOC_SET_NMEA_TYPES_REQ_V02,
                                             &nmeaTypesReq, sizeof(nmeaTypesReq),
                                             &genResp, sizeof(genResp),
                                             COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK == swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_LOC_SET_NMEA_TYPES_REQ_V02),
                                      clientMsgErr,
                                      genResp.resp.result,
                                      genResp.resp.error))
    {
        // Wait QMI notification
        le_clk_Time_t timer = { .sec= TIMEOUT_SEC, .usec= TIMEOUT_USEC };
        le_result_t semResult = le_sem_WaitWithTimeOut(locSyncPtr->semaphoreRef,timer);
        // Command is finished
        StopCommand(locSyncPtr);
        // Check le_sem_WaitUntil timeOut
        if (LE_OK != semResult)
        {
            LE_WARN("Timeout happen");
            result = LE_TIMEOUT;
        }
        else if (LE_OK != locSyncPtr->result)
        {
            LE_ERROR("Error for QMI_LOC_SET_NMEA_TYPES_REQ_V02, error = %i", locSyncPtr->result);
            result = LE_FAULT;
        }
        else if (eQMI_LOC_SUCCESS_V02 == nmeaTypesInd.status)
        {
            LE_DEBUG("Successfully set NMEA sentences type with status %d", nmeaTypesInd.status);
            result = LE_OK;
        }
        else if (eQMI_LOC_ENGINE_BUSY_V02 == nmeaTypesInd.status)
        {
            LE_ERROR("Error for QMI_LOC_SET_NMEA_TYPES_REQ_V02, status = %i",
                      nmeaTypesInd.status);
            result = LE_BUSY;
        }
        else
        {
            LE_ERROR("Error for QMI_LOC_SET_NMEA_TYPES_REQ_V02, status = %i",
                     nmeaTypesInd.status);
            result = LE_FAULT;
        }
    }
    else
    {
        // Command is finished
        LE_ERROR("Set NMEA sentences type error");
        StopCommand(locSyncPtr);
        result = LE_FAULT;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the enabled NMEA sentences bit mask
 *
* @return
*  - LE_OK on success
*  - LE_FAULT on failure
*  - LE_BUSY service is busy
*  - LE_TIMEOUT a time-out occurred
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_GetNmeaSentences
(
    le_gnss_NmeaBitMask_t* nmeaMaskPtr ///< [OUT] Bit mask for enabled NMEA sentences.
)
{
    LE_DEBUG("Get enabled NMEA sentences");

    if (NULL == nmeaMaskPtr)
    {
        LE_ERROR("NULL pointer");
        return LE_FAULT;
    }

    le_result_t result;
    qmiLocGetNmeaTypesIndMsgT_v02 nmeaTypesInd = {0};
    qmiLocGenRespMsgT_v02         genResp      = { {0} };
    pa_gnss_LocSync_t *locSyncPtr = &FunctionSync[PA_GNSS_APISYNC_QMI_LOC_GET_NMEA_TYPES_REQ_V02];

    // Set buffer to fill by the handler
    locSyncPtr->dataPtr = &nmeaTypesInd;
    // Start the command
    StartCommand(locSyncPtr);

    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(LocClient,
                                             QMI_LOC_GET_NMEA_TYPES_REQ_V02,
                                             NULL, 0,
                                             &genResp, sizeof(genResp),
                                             COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK == swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_LOC_GET_NMEA_TYPES_REQ_V02),
                                      clientMsgErr,
                                      genResp.resp.result,
                                      genResp.resp.error))
    {
        // Wait QMI notification
        le_clk_Time_t timer = { .sec= TIMEOUT_SEC, .usec= TIMEOUT_USEC };
        le_result_t semResult = le_sem_WaitWithTimeOut(locSyncPtr->semaphoreRef,timer);
        // Command is finished
        StopCommand(locSyncPtr);
        // Check le_sem_WaitUntil timeOut
        if (LE_OK != semResult)
        {
            LE_WARN("Timeout happen");
            result = LE_TIMEOUT;
        }
        else if (LE_OK != locSyncPtr->result)
        {
            LE_ERROR("Error for QMI_LOC_GET_NMEA_TYPES_REQ_V02, error = %i", locSyncPtr->result);
            result = LE_FAULT;
        }
        else if (eQMI_LOC_SUCCESS_V02 == nmeaTypesInd.status)
        {
            LE_DEBUG("Successfully got NMEA sentences type with status %d", nmeaTypesInd.status);

            if (nmeaTypesInd.nmeaSentenceType_valid)
            {
                // Initialize bit mask
                *nmeaMaskPtr = 0;

                // Convert the bit mask to Legato values
                if (nmeaTypesInd.nmeaSentenceType & QMI_LOC_NMEA_MASK_GGA_V02)
                {
                    *nmeaMaskPtr |= LE_GNSS_NMEA_MASK_GPGGA;
                }
                if (nmeaTypesInd.nmeaSentenceType & QMI_LOC_NMEA_MASK_GSA_V02)
                {
                    *nmeaMaskPtr |= LE_GNSS_NMEA_MASK_GPGSA;
                }
                if (nmeaTypesInd.nmeaSentenceType & QMI_LOC_NMEA_MASK_GSV_V02)
                {
                    *nmeaMaskPtr |= LE_GNSS_NMEA_MASK_GPGSV;
                }
                if (nmeaTypesInd.nmeaSentenceType & QMI_LOC_NMEA_MASK_RMC_V02)
                {
                    *nmeaMaskPtr |= LE_GNSS_NMEA_MASK_GPRMC;
                }
                if (nmeaTypesInd.nmeaSentenceType & QMI_LOC_NMEA_MASK_VTG_V02)
                {
                    *nmeaMaskPtr |= LE_GNSS_NMEA_MASK_GPVTG;
                }
                if (nmeaTypesInd.nmeaSentenceType & QMI_LOC_NMEA_MASK_GLGSV_V02)
                {
                    *nmeaMaskPtr |= LE_GNSS_NMEA_MASK_GLGSV;
                }
                if (nmeaTypesInd.nmeaSentenceType & QMI_LOC_NMEA_MASK_GNGNS_V02)
                {
                    *nmeaMaskPtr |= LE_GNSS_NMEA_MASK_GNGNS;
                }
                if (nmeaTypesInd.nmeaSentenceType & QMI_LOC_NMEA_MASK_GNGSA_V02)
                {
                    *nmeaMaskPtr |= LE_GNSS_NMEA_MASK_GNGSA;
                }
                if (nmeaTypesInd.nmeaSentenceType & QMI_LOC_NMEA_MASK_GAGGA_V02)
                {
                    *nmeaMaskPtr |= LE_GNSS_NMEA_MASK_GAGGA;
                }
                if (nmeaTypesInd.nmeaSentenceType & QMI_LOC_NMEA_MASK_GAGSA_V02)
                {
                    *nmeaMaskPtr |= LE_GNSS_NMEA_MASK_GAGSA;
                }
                if (nmeaTypesInd.nmeaSentenceType & QMI_LOC_NMEA_MASK_GAGSV_V02)
                {
                    *nmeaMaskPtr |= LE_GNSS_NMEA_MASK_GAGSV;
                }
                if (nmeaTypesInd.nmeaSentenceType & QMI_LOC_NMEA_MASK_GARMC_V02)
                {
                    *nmeaMaskPtr |= LE_GNSS_NMEA_MASK_GARMC;
                }
                if (nmeaTypesInd.nmeaSentenceType & QMI_LOC_NMEA_MASK_GAVTG_V02)
                {
                    *nmeaMaskPtr |= LE_GNSS_NMEA_MASK_GAVTG;
                }
#if defined(QMI_LOC_NMEA_MASK_PSTIS_V02)
                if (nmeaTypesInd.nmeaSentenceType & QMI_LOC_NMEA_MASK_PSTIS_V02)
                {
                    *nmeaMaskPtr |= LE_GNSS_NMEA_MASK_PSTIS;
                }
#endif
                if (nmeaTypesInd.nmeaSentenceType & QMI_LOC_NMEA_MASK_PQXFI_V02)
                {
                    *nmeaMaskPtr |= LE_GNSS_NMEA_MASK_PQXFI;
                }
                if (nmeaTypesInd.nmeaSentenceType & QMI_LOC_NMEA_MASK_PQXFI_V02)
                {
                    *nmeaMaskPtr |= LE_GNSS_NMEA_MASK_PTYPE;
                }
#if defined(QMI_LOC_NMEA_MASK_PQGSA_V02)
                if (nmeaTypesInd.nmeaSentenceType & QMI_LOC_NMEA_MASK_PQGSA_V02)
                {
                    *nmeaMaskPtr |= LE_GNSS_NMEA_MASK_PTYPE;
                }
#endif
#if defined(QMI_LOC_NMEA_MASK_PQGSV_V02)
                if (nmeaTypesInd.nmeaSentenceType & QMI_LOC_NMEA_MASK_PQGSV_V02)
                {
                    *nmeaMaskPtr |= LE_GNSS_NMEA_MASK_PTYPE;
                }
#endif
#if defined(QMI_LOC_NMEA_MASK_GPGRS_V02)
                if (nmeaTypesInd.nmeaSentenceType & QMI_LOC_NMEA_MASK_GPGRS_V02)
                {
                    *nmeaMaskPtr |= LE_GNSS_NMEA_MASK_GPGRS;
                }
#endif
#if defined(QMI_LOC_NMEA_MASK_GPGLL_V02)
                if (nmeaTypesInd.nmeaSentenceType & QMI_LOC_NMEA_MASK_GPGLL_V02)
                {
                    *nmeaMaskPtr |= LE_GNSS_NMEA_MASK_GPGLL;
                }
#endif
#if defined(QMI_LOC_NMEA_MASK_DEBUG_V02)
                if (nmeaTypesInd.nmeaSentenceType & QMI_LOC_NMEA_MASK_DEBUG_V02)
                {
                    *nmeaMaskPtr |= LE_GNSS_NMEA_MASK_DEBUG;
                }
#endif
#if defined(QMI_LOC_NMEA_MASK_GPDTM_V02)
                if (nmeaTypesInd.nmeaSentenceType & QMI_LOC_NMEA_MASK_GPDTM_V02)
                {
                    *nmeaMaskPtr |= LE_GNSS_NMEA_MASK_GPDTM;
                }
#endif
#if defined(QMI_LOC_NMEA_MASK_GAGNS_V02)
                if (nmeaTypesInd.nmeaSentenceType & QMI_LOC_NMEA_MASK_GAGNS_V02)
                {
                    *nmeaMaskPtr |= LE_GNSS_NMEA_MASK_GAGNS;
                }
#endif
                // ToDo: Complete the conversion with new values when FW bit mask is extended

                LE_DEBUG("Get NMEA sentences: QMI bit mask 0x%08X -> Legato bit mask 0x%08X",
                          nmeaTypesInd.nmeaSentenceType, *nmeaMaskPtr);

                result = LE_OK;
            }
            else
            {
                LE_ERROR("Enabled NMEA sentences bit mask not valid!");
                result = LE_FAULT;
            }
        }
        else if (eQMI_LOC_ENGINE_BUSY_V02 == nmeaTypesInd.status)
        {
            LE_ERROR("Error for QMI_LOC_GET_NMEA_TYPES_REQ_V02, status = %i",
                      nmeaTypesInd.status);
            result = LE_BUSY;
        }
        else
        {
            LE_ERROR("Error for QMI_LOC_GET_NMEA_TYPES_REQ_V02, status = %i",
                      nmeaTypesInd.status);
            result = LE_FAULT;
        }
    }
    else
    {
        // Command is finished
        LE_ERROR("Get NMEA sentences type error");
        StopCommand(locSyncPtr);
        result = LE_FAULT;
    }

    return result;
}

#if defined(QMI_SWILOC_SET_MIN_ELEVATION_REQ_V01)
//--------------------------------------------------------------------------------------------------
/**
 * Set the GNSS minimum elevation.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED request not supported
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_SetMinElevation
(
    uint8_t  minElevation      ///< [IN] Minimum elevation in degrees [range 0..90].
)
{
    le_result_t result;

    SWIQMI_DECLARE_MESSAGE(swiloc_set_min_elevation_req_msg_v01, setMinElevationReq);
    SWIQMI_DECLARE_MESSAGE(swiloc_set_min_elevation_resp_msg_v01, setMinElevationResp);

    LE_DEBUG("Set GNSS minimum elevation %d", minElevation);

    // Set the GNSS minimum elevation.
    setMinElevationReq.min_elevation = minElevation;

    // Send QMI message
    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(SwiLocClient,
                                         QMI_SWILOC_SET_MIN_ELEVATION_REQ_V01,
                                         &setMinElevationReq, sizeof(setMinElevationReq),
                                         &setMinElevationResp, sizeof(setMinElevationResp),
                                         COMM_DEFAULT_PLATFORM_TIMEOUT);

    /* Check the response */
    if (swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWILOC_SET_MIN_ELEVATION_REQ_V01),
                             clientMsgErr,
                             setMinElevationResp.resp.result,
                             setMinElevationResp.resp.error) != LE_OK)
    {
        LE_ERROR("Set GNSS minimum elevation error");
        result = LE_FAULT;
    }
    else
    {
        LE_DEBUG("Set GNSS minimum elevation done");
        result = LE_OK;
    }
    return result;
}
#endif

#if defined(QMI_SWILOC_GET_MIN_ELEVATION_RESP_V01)
//--------------------------------------------------------------------------------------------------
/**
 *   Get the GNSS minimum elevation.
 *
* @return
*  - LE_OK on success
*  - LE_BAD_PARAMETER if minElevationPtr is NULL
*  - LE_FAULT on failure
*  - LE_UNSUPPORTED request not supported
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_GetMinElevation
(
   uint8_t*  minElevationPtr     ///< [OUT] Minimum elevation in degrees [range 0..90].
)
{
    if (NULL == minElevationPtr)
    {
        LE_ERROR("NULL pointer");
        return LE_BAD_PARAMETER;
    }

    SWIQMI_DECLARE_MESSAGE(swiloc_get_min_elevation_resp_msg_v01, getMinElevationResp);

    // Send QMI message
    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(SwiLocClient,
                                         QMI_SWILOC_GET_MIN_ELEVATION_RESP_V01,
                                         NULL, 0,
                                         &getMinElevationResp, sizeof(getMinElevationResp),
                                         COMM_DEFAULT_PLATFORM_TIMEOUT);

    /* Check the response */
    if (swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWILOC_GET_MIN_ELEVATION_RESP_V01),
                                     clientMsgErr,
                                     getMinElevationResp.resp.result,
                                     getMinElevationResp.resp.error) != LE_OK)
    {
        LE_ERROR("Get GNSS minimum elevation error");
        return LE_FAULT;
    }

    *minElevationPtr = 0;
    // Get the GPS constellation mode
    if (getMinElevationResp.min_elevation_valid)
    {
        *minElevationPtr = getMinElevationResp.min_elevation;
    }
    LE_DEBUG("Get GNSS minimum elevation : %d", *minElevationPtr);
    return LE_OK;
}
#endif

#if defined(QMI_SWILOC_MULTI_COORDINATE_POSITION_INFO_REQ_V01)
//--------------------------------------------------------------------------------------------------
/**
 * Convert a location data parameter from/to multi-coordinate system
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_BAD_PARAMETER if locationDataDstPtr is NULL
 *  - LE_UNSUPPORTED request not supported
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_ConvertDataCoordinateSystem
(
    le_gnss_CoordinateSystem_t coordinateSrc,    ///< [IN] Coordinate system to convert from.
    le_gnss_CoordinateSystem_t coordinateDst,    ///< [IN] Coordinate system to convert to.
    le_gnss_LocationDataType_t locationDataType, ///< [IN] Type of location data to convert.
    int64_t locationDataSrc,                     ///< [IN] Data to convert.
    int64_t* locationDataDstPtr                  ///< [OUT] Converted Data.
)
{
    if (NULL == locationDataDstPtr)
    {
        LE_ERROR("NULL pointer");
        return LE_BAD_PARAMETER;
    }

    SWIQMI_DECLARE_MESSAGE(swiloc_multi_coordinate_position_info_req_msg_v01,
                           convertDataCoordinateReq);
    SWIQMI_DECLARE_MESSAGE(swiloc_multi_coordinate_position_info_resp_msg_v01,
                           convertDataCoordinateResp);

    LE_DEBUG("Convert: coord Src %d,  coord Dst %d,  data type %d, data %" PRId64 "",
              coordinateSrc, coordinateDst, locationDataType, locationDataSrc);

    // Set the coordinate type source and destination
    convertDataCoordinateReq.coordinate_type_src = coordinateSrc;
    convertDataCoordinateReq.coordinate_type_dst = coordinateDst;

    // Set the position data source
    switch(locationDataType)
    {
        case LE_GNSS_POS_LATITUDE:
            convertDataCoordinateReq.position_info_src.latitude =
                (double)(locationDataSrc) / SIX_DECIMAL_PLACE_ACCURACY;
            LE_DEBUG("latitude %lf", convertDataCoordinateReq.position_info_src.latitude);
            break;
        case LE_GNSS_POS_LONGITUDE:
            convertDataCoordinateReq.position_info_src.longitude =
                (double)(locationDataSrc) / SIX_DECIMAL_PLACE_ACCURACY;
            LE_DEBUG("longitude %lf", convertDataCoordinateReq.position_info_src.longitude);
            break;
        case LE_GNSS_POS_ALTITUDE:
            convertDataCoordinateReq.position_info_src.altitude =
                (double)(locationDataSrc) / THREE_DECIMAL_PLACE_ACCURACY;
            LE_DEBUG("altitude %lf", convertDataCoordinateReq.position_info_src.altitude);
            break;
        case LE_GNSS_POS_MAX:
        default:
            LE_ERROR("Bad location data type");
            return LE_FAULT;
    };

    // Send QMI message
    qmi_client_error_type clientMsgErr = qmi_client_send_msg_sync(
                                      SwiLocClient,
                                      QMI_SWILOC_MULTI_COORDINATE_POSITION_INFO_REQ_V01,
                                      &convertDataCoordinateReq, sizeof(convertDataCoordinateReq),
                                      &convertDataCoordinateResp, sizeof(convertDataCoordinateResp),
                                      COMM_DEFAULT_PLATFORM_TIMEOUT);

    /* Check the response */
    if (LE_OK != swiQmi_CheckResponse(
                               STRINGIZE_EXPAND(QMI_SWILOC_MULTI_COORDINATE_POSITION_INFO_RESP_V01),
                               clientMsgErr,
                               convertDataCoordinateResp.resp.result,
                               convertDataCoordinateResp.resp.error))
    {
        LE_ERROR("Convert data coordinate error");
        return LE_FAULT;
    }

    if (convertDataCoordinateResp.coordinate_type_dst != coordinateDst)
    {
        LE_ERROR("Coordinate system transferred error req:%d, resp:%d",
                 coordinateDst,
                 convertDataCoordinateResp.coordinate_type_dst);
        return LE_FAULT;
    }

    // Set the position data destination
    switch(locationDataType)
    {
        case LE_GNSS_POS_LATITUDE:
            *locationDataDstPtr = (int64_t)(convertDataCoordinateResp.position_info_dst.latitude *
                SIX_DECIMAL_PLACE_ACCURACY);
            LE_DEBUG("latitude %lf", convertDataCoordinateResp.position_info_dst.latitude);
            break;
        case LE_GNSS_POS_LONGITUDE:
            *locationDataDstPtr = (int64_t)(convertDataCoordinateResp.position_info_dst.longitude *
                SIX_DECIMAL_PLACE_ACCURACY);
            LE_DEBUG("longitude %lf", convertDataCoordinateResp.position_info_dst.longitude);
            break;
        case LE_GNSS_POS_ALTITUDE:
            *locationDataDstPtr = (int64_t)(convertDataCoordinateResp.position_info_dst.altitude *
                THREE_DECIMAL_PLACE_ACCURACY);
            LE_DEBUG("altitude %lf", convertDataCoordinateResp.position_info_dst.altitude);
            break;
        case LE_GNSS_POS_MAX:
        default:
            LE_ERROR("Bad location data type");
            return LE_FAULT;
    };

    LE_DEBUG("Convert data coordinate done: %" PRId64 "", *locationDataDstPtr);
    return LE_OK;
}
#endif

//--------------------------------------------------------------------------------------------------
/**
 * Component initialization function.
 *
 * This is not used because it will be called immediately at process start by the application
 * framework, and we want to wait until we can confirm that GNSS is available before starting
 * the platform adapter.
 *
 * See pa_gnss_Init().
 */
//--------------------------------------------------------------------------------------------------
COMPONENT_INIT
{
}
