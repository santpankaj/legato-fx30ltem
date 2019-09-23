/**
 * @file atLinker.c: Implements AT commands forwarding service
 *
 * @note: This process is seperated into 3 threads, main, reader, writer
 *          - main thread:
 *            - Initializes atLinker service, epoll and socket.
 *            - Blocks signal TERMINATED sent when doing a app stop to be able
 *              to cleanup before exiting.
 *            - Upon receipt of a signal, an event is raised and the Cleanup
 *              handler function is called.
 *
 *          - writer thread:
 *            - Waits for signals raised by the atLinker service.
 *            - Upon receipt of an at cmd it sends it through the socket fd to
 *              atBinder service. If the socket is not properly initilizes then
 *              it sends an ERROR response to the atLinker service
 *
 *          - reader thread:
 *            - Handles the connection to the atBinder service.
 *            - Reads and sends to the atLinker service the received responses.
 * <HR>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <legato.h>
#include <interfaces.h>
#include "atLinker.h"

//--------------------------------------------------------------------------------------------------
/**
 * Socket system path
 */
//--------------------------------------------------------------------------------------------------
#define SOCKPATH                "/tmp/sock0"

//--------------------------------------------------------------------------------------------------
/**
 * Writer thread reference
 */
//--------------------------------------------------------------------------------------------------
static le_thread_Ref_t WriterRef;

//--------------------------------------------------------------------------------------------------
/**
 * Reader thread reference
 */
//--------------------------------------------------------------------------------------------------
static le_thread_Ref_t ReaderRef;

//--------------------------------------------------------------------------------------------------
/**
 * Linker struct initialization
 */
//--------------------------------------------------------------------------------------------------
static AtLinker_t AtLinker = {
    .init = NULL,
    .registerCmds = NULL,
    .release = NULL,
};

//--------------------------------------------------------------------------------------------------
/**
 * Event struct initialization
 */
//--------------------------------------------------------------------------------------------------
static AtEvent_t AtEvent = {
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .cond = PTHREAD_COND_INITIALIZER,
    .cmd = {0},
};

//--------------------------------------------------------------------------------------------------
/**
 * At Command
 */
//--------------------------------------------------------------------------------------------------
static char AtCmd[MAX_CMD_LEN] = { 0 };

//--------------------------------------------------------------------------------------------------
/**
 * Socket file descriptor
 */
//--------------------------------------------------------------------------------------------------
static int SockFd = -1;

//--------------------------------------------------------------------------------------------------
/**
 * Epoll file descriptor
 */
//--------------------------------------------------------------------------------------------------
static int EpollFd = -1;

//--------------------------------------------------------------------------------------------------
/**
 * Close a fd and raise a warning in case of an error
 */
//--------------------------------------------------------------------------------------------------
static void CloseWarn
(
    int fd
)
{
    if (-1 == close(fd))
    {
        LE_WARN("failed to close fd %d: %m", fd);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Write to a file descriptor
 */
//--------------------------------------------------------------------------------------------------
static int FdWrite
(
    int     fd,
    void*   bufPtr,
    size_t  size
)
{
    ssize_t count;

    while (size)
    {
        count = write(fd, bufPtr, size);
        if (-1 == count && (EINTR == errno))
        {
            continue;
        }
        if (-1 == count)
        {
            LE_ERROR("write failed: %m");
            return -1;
        }
        if (0 <= count)
        {
            size -= count;
            bufPtr += count;
        }
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Handle socket connection
 */
//--------------------------------------------------------------------------------------------------
static int HandleSocketConnection
(
    int*    sockFdPtr,
    int*    timeoutPtr
)
{
    struct sockaddr_un addr;

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKPATH, sizeof(addr.sun_path)-1);

    if (-1 == connect(*sockFdPtr, (struct sockaddr *)&addr, sizeof(addr)))
    {
        switch (errno)
        {
            case ENOENT:
            case ECONNREFUSED:
                break;
            case EISCONN:
                *timeoutPtr = -1;
                break;
            default:
                LE_ERROR("connect failed: %m");
                return -1;
        }
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize socket and epoll
 */
//--------------------------------------------------------------------------------------------------
static int InitSocket
(
    void
)
{
    struct epoll_event event = { 0 };

    EpollFd = epoll_create1(0);
    if (-1 == EpollFd)
    {
        LE_ERROR("epoll_create1 failed: %m");
        return -1;
    }

    SockFd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (-1 == SockFd)
    {
        LE_ERROR("socket failed: %m");
        goto epoll_err;
    }

    event.events = EPOLLIN | EPOLLET;
    event.data.fd = SockFd;

    if (-1 == epoll_ctl(EpollFd, EPOLL_CTL_ADD, SockFd, &event))
    {
        LE_ERROR("epoll_ctl failed: %m");
        goto sock_err;
    }

    return 0;

sock_err:
    CloseWarn(SockFd);
epoll_err:
    CloseWarn(EpollFd);
    return -1;
}

//--------------------------------------------------------------------------------------------------
/**
 * Cleanup function
 */
//--------------------------------------------------------------------------------------------------
static void DoCleanup
(
    void
)
{
    le_thread_Cancel(ReaderRef);
    le_thread_Join(ReaderRef, NULL);

    le_thread_Cancel(WriterRef);
    le_thread_Join(WriterRef, NULL);

    CloseWarn(SockFd);

    CloseWarn(EpollFd);

    if (AtLinker.release() == -1)
    {
        LE_ERROR("Failed release atLinker");
    }

    exit(EXIT_SUCCESS);
}

//--------------------------------------------------------------------------------------------------
/**
 * A thread function that reads atServer responses
 */
//--------------------------------------------------------------------------------------------------
static void* Reader
(
    void* ctxPtr
)
{
    int rc, timeout = 100;
    struct epoll_event event = { 0 };
    char buf[MAX_RESP_LEN] = { 0 };
    ssize_t count;

    while (1)
    {
        rc = epoll_wait(EpollFd, &event, 1, timeout);
        if ( (-1 == rc) && (EINTR != errno) )
        {
            LE_ERROR("epoll_wait failed: %m");
            goto sys_err;
        }

        if (event.events & (EPOLLHUP | EPOLLERR))
        {
            if (-1 == HandleSocketConnection(&SockFd, &timeout))
            {
                goto sys_err;
            }
        }

        if (event.events & EPOLLIN)
        {
            memset(buf, 0, MAX_RESP_LEN);
            count = read(SockFd, buf, MAX_RESP_LEN);
            if ( (-1 == count) && (EINTR != errno) )
            {
                LE_ERROR("read failed: %m");
                goto sys_err;
            }
            if (!count)
            {
                LE_ERROR("connection lost");
                goto sys_err;
            }
            if (-1 == AtLinker.sendResponse(buf, count))
            {
                LE_ERROR("Failed to send response");
            }
        }
    }

sys_err:
    DoCleanup();
    return NULL;
}

//--------------------------------------------------------------------------------------------------
/**
 * A thread function that sends forwarded commands to atServer
 */
//--------------------------------------------------------------------------------------------------
static void* Writer
(
    void* ctxPtr
)
{
    while (1) {
        pthread_mutex_lock(&AtEvent.mutex);
        if (!strlen(AtEvent.cmd))
        {
            pthread_cond_wait(&AtEvent.cond, &AtEvent.mutex);
        }

        memcpy(AtCmd, AtEvent.cmd, strlen(AtEvent.cmd));
        AtEvent.cmd[0] = '\0';
        pthread_mutex_unlock(&AtEvent.mutex);

        LE_DEBUG("cmd: `%s'", AtCmd);

        if (!strcmp("invalid", AtCmd))
        {
            if (-1 == AtLinker.sendResponse("\r\nERROR\r\n", 9))
            {
                LE_ERROR("Failed to send response");
            }
        }
        else
        {
            if (-1 == FdWrite(SockFd, AtCmd, strlen(AtCmd)))
            {
                LE_ERROR("failed to send forwarded command");
                if (-1 == AtLinker.sendResponse("\r\nERROR\r\n", 9))
                {
                    LE_ERROR("Failed to send response");
                }
            }
        }
        memset(AtCmd, 0, MAX_CMD_LEN);
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/**
 * Signal handler
 */
//--------------------------------------------------------------------------------------------------
static void SigHandler
(
    int sigNum
)
{
    if (SIGTERM != sigNum)
    {
        LE_ERROR("Received an unexpected signal %d", sigNum);
    }

    DoCleanup();
}

//--------------------------------------------------------------------------------------------------
/**
 * Main
 */
//--------------------------------------------------------------------------------------------------
COMPONENT_INIT
{
    int rc;
    char *cmds[] = {"+WDSC", "+WDSE", "+WDSG", "+WDSR", "+WDSS", "+WDSI", "+WDSW"};

    atLinker_Init(&AtLinker);

    rc = AtLinker.init(&AtEvent);
    if (-1 == rc)
    {
        LE_ERROR("Failed to initialize atLinker");
        exit(EXIT_FAILURE);
    }

    rc = AtLinker.registerCmds(cmds, NUM_ARRAY_MEMBERS(cmds));
    if (-1 == rc)
    {
        LE_ERROR("Failed to register AT commands");
        exit(EXIT_FAILURE);
    }

    if (-1 == InitSocket())
    {
        if (AtLinker.release() == -1)
        {
            LE_ERROR("Failed to release atLinker");
        }
        exit(EXIT_FAILURE);
    }

    WriterRef = le_thread_Create("Writer", Writer, NULL);
    le_thread_SetJoinable(WriterRef);

    ReaderRef = le_thread_Create("Reader", Reader, NULL);
    le_thread_SetJoinable(ReaderRef);

    le_sig_Block(SIGTERM);
    le_sig_SetEventHandler(SIGTERM, SigHandler);

    le_thread_Start(WriterRef);
    le_thread_Start(ReaderRef);

    return;
}
