/**
 * @file pa_riPin_qmi.c
 *
 * QMI implementation of PA Ring Indicator signal.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "pa_riPin.h"
#include "swiQmi.h"


//--------------------------------------------------------------------------------------------------
// Symbol and Enum definitions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Ring Indicator pin name.
 */
//--------------------------------------------------------------------------------------------------
#define RI_PIN_NAME     "RI"

//--------------------------------------------------------------------------------------------------
/**
 * Path to configure GPIOs through sysfs.
 */
//--------------------------------------------------------------------------------------------------
#define GPIO_PATH       "/sys/class/gpio"

//--------------------------------------------------------------------------------------------------
/**
 * Ring Indicator signal owners:
 *  - 0 means that the modem core is the owner of the Ring Indicator.
 *  - 1 means that the application core is the owner of the Ring Indicator.
 */
//--------------------------------------------------------------------------------------------------
#define MODEM_CORE_OWNER    0
#define APP_CORE_OWNER      1


//--------------------------------------------------------------------------------------------------
// Static declarations
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * The MGS (M2M General Service) client.
 * Must be obtained by calling swiQmi_GetClientHandle() before it can be used.
 */
//--------------------------------------------------------------------------------------------------
static qmi_client_type MgsClient;

//--------------------------------------------------------------------------------------------------
/**
 * Open a file descriptor, retrying if interrupted by a signal.
 *
 * @return
 *  - LE_OK     The function succeeded.
 *  - LE_FAULT  The function failed.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t OpenFd
(
    const char* filePathPtr,    ///< [IN]  File path
    int flags,                  ///< [IN]  Flags
    int* fdPtr                  ///< [OUT] File descriptor
)
{
    do
    {
        *fdPtr = open(filePathPtr, flags);
    }
    while ((*fdPtr == -1) && (errno == EINTR));

    if (-1 == *fdPtr)
    {
        LE_ERROR("Failed to open %s: %d (%m)", filePathPtr, errno);
        return LE_FAULT;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Close a file descriptor, retrying if interrupted by a signal.
 *
 * @return
 *  - LE_OK     The function succeeded.
 *  - LE_FAULT  The function failed.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t CloseFd
(
    int fd  ///< [IN] File descriptor
)
{
    int rc = -1;

    do
    {
        rc = close(fd);
    }
    while ((rc == -1) && (errno == EINTR));

    if (rc != 0)
    {
        LE_ERROR("Failed to close file descriptor %d: %d (%m)", fd, errno);
        return LE_FAULT;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Write to a file descriptor, retrying if interrupted by a signal.
 *
 * @return
 *  - LE_OK     The function succeeded.
 *  - LE_FAULT  The function failed.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t WriteToFd
(
    int fd,                 ///< [IN] File descriptor
    const void* bufPtr,     ///< [IN] Buffer
    size_t bufSize          ///< [IN] Buffer size
)
{
    ssize_t size = 0;

    do
    {
        size = write(fd, bufPtr, bufSize);
    }
    while ((size != bufSize) && (errno == EINTR));

    if (size != bufSize)
    {
        LE_ERROR("Failed to write to fd %d: %d (%m)", fd, errno);
        return LE_FAULT;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function exports a GPIO.
 *
 * @return
 *  - LE_OK     The function succeeded.
 *  - LE_FAULT  The function failed.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ExportGpio
(
    const char *gpioPtr     ///< [IN] GPIO to export
)
{
    int fd = -1;
    char filePathStr[PATH_MAX] = {0};

    // Check whether GPIO has been already exported
    snprintf(filePathStr, sizeof(filePathStr), "%s/gpio%s", GPIO_PATH, gpioPtr);
    if (0 != access(filePathStr, X_OK))
    {
        // Export GPIO to user space
        char exportPathStr[PATH_MAX] = {0};
        snprintf(exportPathStr, sizeof(exportPathStr), "%s/export", GPIO_PATH);
        if (LE_OK != OpenFd(exportPathStr, O_WRONLY, &fd))
        {
            return LE_FAULT;
        }
        if (LE_OK != WriteToFd(fd, gpioPtr, strlen(gpioPtr)+1))
        {
            LE_ERROR("Failed to export GPIO %s to user space", gpioPtr);
            CloseFd(fd);
            return LE_FAULT;
        }
        if (LE_OK != CloseFd(fd))
        {
            LE_ERROR("Failed to close %s", exportPathStr);
            return LE_FAULT;
        }
    }

    // Set GPIO as OUTPUT
    snprintf(filePathStr, sizeof(filePathStr), "%s/gpio%s/direction", GPIO_PATH, gpioPtr);
    if (LE_OK != OpenFd(filePathStr, O_WRONLY, &fd))
    {
        return LE_FAULT;
    }
    if (LE_OK != WriteToFd(fd, "out", strlen("out")+1))
    {
        LE_ERROR("Failed to set output direction for GPIO %s)", gpioPtr);
        CloseFd(fd);
        return LE_FAULT;
    }
    if (LE_OK != CloseFd(fd))
    {
        LE_ERROR("Failed to close %s", filePathStr);
        return LE_FAULT;
    }

    LE_INFO("GPIO %s successfully exported to user space", gpioPtr);
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function unexports a GPIO.
 *
 * @return
 *  - LE_OK     The function succeeded.
 *  - LE_FAULT  The function failed.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t UnexportGpio
(
    const char *gpioPtr     ///< [IN] GPIO to unexport
)
{
    int fd = -1;
    char filePathStr[PATH_MAX] = {0};

    // Check whether GPIO is exported
    snprintf(filePathStr, sizeof(filePathStr), "%s/gpio%s", GPIO_PATH, gpioPtr);
    if (0 == access(filePathStr, X_OK))
    {
        // Set GPIO as INPUT
        char unexportPathStr[PATH_MAX] = {0};
        snprintf(filePathStr, sizeof(filePathStr), "%s/gpio%s/direction", GPIO_PATH, gpioPtr);
        if (LE_OK != OpenFd(filePathStr, O_WRONLY, &fd))
        {
            return LE_FAULT;
        }
        if (LE_OK != WriteToFd(fd, "in", strlen("in")+1))
        {
            LE_ERROR("Failed to set input direction for GPIO %s", gpioPtr);
            CloseFd(fd);
            return LE_FAULT;
        }
        if (LE_OK != CloseFd(fd))
        {
            LE_ERROR("Failed to close %s", filePathStr);
            return LE_FAULT;
        }

        // Unexport GPIO from user space
        snprintf(unexportPathStr, sizeof(unexportPathStr), "%s/unexport", GPIO_PATH);
        if (LE_OK != OpenFd(unexportPathStr, O_WRONLY, &fd))
        {
            return LE_FAULT;
        }
        if (LE_OK != WriteToFd(fd, gpioPtr, strlen(gpioPtr)+1))
        {
            LE_ERROR("Failed to unexport GPIO %s from user space", gpioPtr);
            CloseFd(fd);
            return LE_FAULT;
        }
        if (LE_OK != CloseFd(fd))
        {
            LE_ERROR("Failed to close %s", unexportPathStr);
            return LE_FAULT;
        }
    }

    LE_INFO("GPIO %s successfully unexported from user space", gpioPtr);
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function sets the owner of the Ring Indicator signal.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_UNSUPPORTED    The platform does not support this operation.
 *  - LE_FAULT          The function failed.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SetRingSignalOwner
(
    uint8_t owner   ///< [IN] New Ring Indicator signal owner
)
{
    qmi_client_error_type err;
    SWIQMI_DECLARE_MESSAGE(swi_m2m_set_ri_owner_req_msg_v01, setReq);
    SWIQMI_DECLARE_MESSAGE(swi_m2m_set_ri_owner_resp_msg_v01, setResp);

    // 0 – The RI owned by Modem core
    // 1 – The RI owned by application core
    setReq.ri_owner = owner;

    err = qmi_client_send_msg_sync(MgsClient,
                                   QMI_SWI_M2M_SET_RI_OWNER_REQ_V01,
                                   &setReq, sizeof(setReq),
                                   &setResp, sizeof(setResp),
                                   COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_SET_RI_OWNER_REQ_V01),
                                      err,
                                      setResp.resp.result,
                                      setResp.resp.error))
    {
        if (QMI_ERR_NOT_SUPPORTED_V01 == setResp.resp.error)
        {
            LE_ERROR("Setting the owner of the Ring Indicator signal is not supported");
            return LE_UNSUPPORTED;
        }

        LE_ERROR("Failed to set the owner of the Ring Indicator signal");
        return LE_FAULT;
    }

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
// Public functions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Check whether the application core is the current owner of the Ring Indicator signal.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_BAD_PARAMETER  Bad input parameter.
 *  - LE_FAULT          The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_riPin_AmIOwnerOfRingSignal
(
    bool* amIOwnerPtr  ///< [OUT] True when application core is the owner of the Ring Indicator
                       ///        signal.
                       ///        False when modem core is the owner of the Ring Indicator signal.
)
{
    qmi_client_error_type err;
    SWIQMI_DECLARE_MESSAGE(swi_m2m_get_ri_owner_resp_msg_v01, getResp);

    // Check input pointer
    if (!amIOwnerPtr)
    {
        LE_ERROR("Null pointer");
        return LE_BAD_PARAMETER;
    }

    err = qmi_client_send_msg_sync(MgsClient,
                                   QMI_SWI_M2M_GET_RI_OWNER_REQ_V01,
                                   NULL, 0,
                                   &getResp, sizeof(getResp),
                                   COMM_DEFAULT_PLATFORM_TIMEOUT);

    if (LE_OK != swiQmi_CheckResponse(STRINGIZE_EXPAND(QMI_SWI_M2M_GET_RI_OWNER_REQ_V01),
                                      err,
                                      getResp.resp.result,
                                      getResp.resp.error))
    {
        LE_ERROR("Failed to get the owner of the Ring Indicator signal");
        return LE_FAULT;
    }

    if (getResp.ri_owner)
    {
        LE_DEBUG("Application core is the owner of RI pin");
        *amIOwnerPtr = true;
    }
    else
    {
        LE_DEBUG("Modem core is the owner of RI pin");
        *amIOwnerPtr = false;
    }
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * The application core takes the control of the Ring Indicator signal.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_UNSUPPORTED    The platform does not support this operation.
 *  - LE_FAULT          The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_riPin_TakeRingSignal
(
    void
)
{
    // Take control of Ring Indicator signal
    le_result_t result = SetRingSignalOwner(APP_CORE_OWNER);

    switch (result)
    {
        case LE_OK:
            // Application core has control of Ring Indicator signal, now export GPIO
            if (LE_OK != ExportGpio(RI_PIN_NAME))
            {
                result = LE_FAULT;
            }
            break;

        case LE_UNSUPPORTED:
            LE_WARN("TakeRingSignal is not supported");
            break;

        default:
            LE_ERROR("TakeRingSignal error");
            break;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * The application core releases the control of the Ring Indicator signal.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_UNSUPPORTED    The platform does not support this operation.
 *  - LE_FAULT          The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_riPin_ReleaseRingSignal
(
    void
)
{
    // Unexport GPIO while application core has control of Ring Indicator signal
    if (LE_OK != UnexportGpio(RI_PIN_NAME))
    {
        return LE_FAULT;
    }

    // Release control of Ring Indicator signal
    le_result_t result = SetRingSignalOwner(MODEM_CORE_OWNER);

    switch (result)
    {
        case LE_OK:
            LE_DEBUG("ReleaseRingSignal success");
            break;

        case LE_UNSUPPORTED:
            LE_WARN("ReleaseRingSignal is not supported");
            break;

        default:
            LE_ERROR("ReleaseRingSignal error");
            break;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set RI GPIO value
 */
//--------------------------------------------------------------------------------------------------
void pa_riPin_Set
(
    uint8_t set     ///< [IN] 1 to Pull up GPIO RI or 0 to lower it down
)
{
    int fd = -1;
    char filePathStr[PATH_MAX] = {0};

    snprintf(filePathStr, sizeof(filePathStr), "%s/gpio%s/value", GPIO_PATH, RI_PIN_NAME);
    if (LE_OK != OpenFd(filePathStr, O_WRONLY, &fd))
    {
        return;
    }

    snprintf(filePathStr, sizeof(filePathStr), "%d", set);
    if (LE_OK != WriteToFd(fd, filePathStr, strlen(filePathStr)+1))
    {
        LE_ERROR("Failed to set the GPIO %s to the value %d", RI_PIN_NAME, set);
    }

    if (LE_OK == CloseFd(fd))
    {
        LE_DEBUG("Successfully set riPin to %d", set);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the PA Ring Indicator signal module.
 *
 * @return
 *  - LE_OK     The function succeeded.
 *  - LE_FAULT  The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_riPin_Init
(
    void
)
{
    bool isAppCoreOwner;

    LE_INFO("pa_riPin_qmi init");

    // Get the QMI client service handle for later use.
    MgsClient = swiQmi_GetClientHandle(QMI_SERVICE_MGS);
    if (!MgsClient)
    {
        LE_ERROR("Could not get the QMI service");
        return LE_FAULT;
    }

    // Retrieve RI pin owner
    if (LE_OK != pa_riPin_AmIOwnerOfRingSignal(&isAppCoreOwner))
    {
        LE_INFO("pa_riPin_qmi init failed");
        return LE_FAULT;
    }

    // Check RI pin owner
    if (true == isAppCoreOwner)
    {
        // Application core is the owner of RI signal, export the GPIO RI
        if (LE_OK != ExportGpio(RI_PIN_NAME))
        {
            LE_INFO("pa_riPin_qmi init failed");
            return LE_FAULT;
        }

        LE_INFO("RI pin owner is application core");
        LE_INFO("pa_riPin_qmi init done");
        return LE_OK;
    }

    // Modem core is the owner of RI signal
    LE_INFO("RI pin owner is modem core");
    LE_INFO("pa_riPin_qmi init done");
    return LE_OK;
}
