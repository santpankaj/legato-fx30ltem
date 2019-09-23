#*******************************************************************************
# Copyright (C) Sierra Wireless Inc.
#*******************************************************************************

# Tests
enable_testing()

# Output
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/tests/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/tests/lib)
set(APP_OUTPUT_PATH ${CMAKE_BINARY_DIR}/tests/apps)

file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})
file(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH})
file(MAKE_DIRECTORY ${APP_OUTPUT_PATH})

if(LEGATO_COMPONENTS_MODEM MATCHES "QMI")
    #add_subdirectory(tests/modem/sim)
    #add_subdirectory(tests/modem/voice)
    #add_subdirectory(tests/modem/sms)
    #add_subdirectory(tests/modem/mdc)
    #add_subdirectory(tests/modem/mrc)
    if((INCLUDE_ECALL EQUAL 1) AND ((LEGATO_TARGET MATCHES "ar7") OR (LEGATO_TARGET MATCHES "ar86")))
        #add_subdirectory(tests/modem/ecall)
    endif()
endif()

if(LEGATO_COMPONENTS_GNSS MATCHES "QMI")
    #add_subdirectory(tests/positioning/position)
    #add_subdirectory(tests/positioning/xtra)
    #add_subdirectory(tests/positioning/supl)
endif()

