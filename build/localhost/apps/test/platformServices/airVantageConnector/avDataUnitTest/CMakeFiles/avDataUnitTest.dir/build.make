# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost

# Utility rule file for avDataUnitTest.

# Include the progress variables for this target.
include apps/test/platformServices/airVantageConnector/avDataUnitTest/CMakeFiles/avDataUnitTest.dir/progress.make

apps/test/platformServices/airVantageConnector/avDataUnitTest/CMakeFiles/avDataUnitTest:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "mkexe 'avDataUnitTest': /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/bin/avDataUnitTest"
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/platformServices/airVantageConnector/apps/test/avDataUnitTest && /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/bin/mkexe -o /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/bin/avDataUnitTest -w /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/platformServices/airVantageConnector/avDataUnitTest/_build_avDataUnitTest -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/platformServices/airVantageConnector/apps/test/avDataUnitTest -l /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/lib -t localhost assetDataComp . -i assetDataComp -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/platformServices/airVantageConnector//apps/test/avDataUnitTest/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/platformServices/airVantageConnector//apps/platformServices/airvantageConnector/avcDaemon -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/platformServices/airVantageConnector//avcClient/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/platformServices/airVantageConnector//avcDaemon/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/platformServices/airVantageConnector//avcAppUpdate/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/platformServices/airVantageConnector//packageDownloader/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/liblegato -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/components/watchdogChain/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/components/appCfg/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/liblegato/linux/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/daemons/linux/configTree -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/3rdParty/Lwm2mCore/include/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/3rdParty/Lwm2mCore/include/platform-specific/linux/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/3rdParty/Lwm2mCore/3rdParty/wakaama/core/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/3rdParty/Lwm2mCore/include/lwm2mcore/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/3rdParty/Lwm2mCore/3rdParty/wakaama/core/er-coap-13/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/3rdParty/inc/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/3rdParty/Lwm2mCore/packageDownloader/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/3rdParty/Lwm2mCore/sessionManager/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/3rdParty/Lwm2mCore/objectManager/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/3rdParty/Lwm2mCore/tests/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/3rdParty/Lwm2mCore/3rdParty/tinydtls/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/3rdParty/tinycbor/src -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/interfaces/airVantage/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/interfaces/modemServices/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/interfaces/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/platformAdaptor/simu/components/le_pa_avc -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/components/airVantage/platformAdaptor/inc/ -s /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/3rdParty/Lwm2mCore/include/lwm2mcore/ -C -fvisibility=default

avDataUnitTest: apps/test/platformServices/airVantageConnector/avDataUnitTest/CMakeFiles/avDataUnitTest
avDataUnitTest: apps/test/platformServices/airVantageConnector/avDataUnitTest/CMakeFiles/avDataUnitTest.dir/build.make

.PHONY : avDataUnitTest

# Rule to build all files generated by this target.
apps/test/platformServices/airVantageConnector/avDataUnitTest/CMakeFiles/avDataUnitTest.dir/build: avDataUnitTest

.PHONY : apps/test/platformServices/airVantageConnector/avDataUnitTest/CMakeFiles/avDataUnitTest.dir/build

apps/test/platformServices/airVantageConnector/avDataUnitTest/CMakeFiles/avDataUnitTest.dir/clean:
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/platformServices/airVantageConnector/avDataUnitTest && $(CMAKE_COMMAND) -P CMakeFiles/avDataUnitTest.dir/cmake_clean.cmake
.PHONY : apps/test/platformServices/airVantageConnector/avDataUnitTest/CMakeFiles/avDataUnitTest.dir/clean

apps/test/platformServices/airVantageConnector/avDataUnitTest/CMakeFiles/avDataUnitTest.dir/depend:
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/platformServices/airVantageConnector/apps/test/avDataUnitTest /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/platformServices/airVantageConnector/avDataUnitTest /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/platformServices/airVantageConnector/avDataUnitTest/CMakeFiles/avDataUnitTest.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : apps/test/platformServices/airVantageConnector/avDataUnitTest/CMakeFiles/avDataUnitTest.dir/depend
