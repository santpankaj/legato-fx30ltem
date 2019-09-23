#include "legato.h"
#include "interfaces.h"

static void PrintUsage()
{
    int idx;
    bool sandboxed = (getuid() != 0);
    const char * usagePtr[] = {
            "Usage of the 'send' tool is:",
            "   send <destNumber> <textMessage>",
            "",
            "Warning:",
            "   If the <textMessage> is a known command, it will",
            "   only be handled locally and not sent to the requested",
            "   destination.",
    };

    for (idx = 0; idx < NUM_ARRAY_MEMBERS(usagePtr); idx++)
    {
        if (sandboxed)
        {
            LE_INFO("%s", usagePtr[idx]);
        }
        else
        {
            fprintf(stderr, "%s\n", usagePtr[idx]);
        }
    }
}

COMPONENT_INIT
{
    if (le_arg_NumArgs() == 2)
    {
        const char* telPtr     = le_arg_GetArg(0);
        const char* messagePtr = le_arg_GetArg(1);
        if (NULL == telPtr)
        {
            LE_ERROR("telPtr is NULL");
            exit(EXIT_FAILURE);
        }
        if (NULL == messagePtr)
        {
            LE_ERROR("messagePtr is NULL");
            exit(EXIT_FAILURE);
        }
        le_result_t result = send_SendMessage(telPtr, messagePtr);

        if (result != LE_OK)
        {
            LE_FATAL("Unable to send message.");
        }
    }
    else
    {
        PrintUsage();
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
