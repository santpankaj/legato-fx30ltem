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

# Utility rule file for packageDownloadHost.

# Include the progress variables for this target.
include apps/test/platformServices/airVantageConnector/packageDownloadHost/CMakeFiles/packageDownloadHost.dir/progress.make

apps/test/platformServices/airVantageConnector/packageDownloadHost/CMakeFiles/packageDownloadHost:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "mkexe 'packageDownloadHost': /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/bin/packageDownloadHost"
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/platformServices/airVantageConnector/apps/test/packageDownloadHost && /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/bin/mkexe -o /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/bin/packageDownloadHost -w /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/platformServices/airVantageConnector/packageDownloadHost/_build_packageDownloadHost -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/platformServices/airVantageConnector/apps/test/packageDownloadHost -l /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/lib -t localhost . packageDownloadComp -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/liblegato -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/3rdParty/Lwm2mCore/include -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/3rdParty/Lwm2mCore/include/lwm2mcore -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/3rdParty/Lwm2mCore/include/platform-specific/linux -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/3rdParty/Lwm2mCore/packageDownloader -C -fvisibility=default -C -Wno-error

packageDownloadHost: apps/test/platformServices/airVantageConnector/packageDownloadHost/CMakeFiles/packageDownloadHost
packageDownloadHost: apps/test/platformServices/airVantageConnector/packageDownloadHost/CMakeFiles/packageDownloadHost.dir/build.make

.PHONY : packageDownloadHost

# Rule to build all files generated by this target.
apps/test/platformServices/airVantageConnector/packageDownloadHost/CMakeFiles/packageDownloadHost.dir/build: packageDownloadHost

.PHONY : apps/test/platformServices/airVantageConnector/packageDownloadHost/CMakeFiles/packageDownloadHost.dir/build

apps/test/platformServices/airVantageConnector/packageDownloadHost/CMakeFiles/packageDownloadHost.dir/clean:
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/platformServices/airVantageConnector/packageDownloadHost && $(CMAKE_COMMAND) -P CMakeFiles/packageDownloadHost.dir/cmake_clean.cmake
.PHONY : apps/test/platformServices/airVantageConnector/packageDownloadHost/CMakeFiles/packageDownloadHost.dir/clean

apps/test/platformServices/airVantageConnector/packageDownloadHost/CMakeFiles/packageDownloadHost.dir/depend:
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/platformServices/airVantageConnector/apps/test/packageDownloadHost /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/platformServices/airVantageConnector/packageDownloadHost /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/platformServices/airVantageConnector/packageDownloadHost/CMakeFiles/packageDownloadHost.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : apps/test/platformServices/airVantageConnector/packageDownloadHost/CMakeFiles/packageDownloadHost.dir/depend

