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

# Utility rule file for powerMgrTest.

# Include the progress variables for this target.
include apps/test/powerMgr/powerMgrTest/CMakeFiles/powerMgrTest.dir/progress.make

apps/test/powerMgr/powerMgrTest/CMakeFiles/powerMgrTest:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "mkapp 'powerMgrTest': /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/apps/powerMgrTest.localhost"
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/powerMgr/powerMgrTest && PATH=/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/bin:/usr/bin/:/opt/swi/y22-ext-fx30lteM/sysroots/x86_64-pokysdk-linux/usr/bin:/opt/swi/y22-ext-fx30lteM/sysroots/x86_64-pokysdk-linux/usr/sbin:/opt/swi/y22-ext-fx30lteM/sysroots/x86_64-pokysdk-linux/bin:/opt/swi/y22-ext-fx30lteM/sysroots/x86_64-pokysdk-linux/sbin:/opt/swi/y22-ext-fx30lteM/sysroots/x86_64-pokysdk-linux/usr/bin/../x86_64-pokysdk-linux/bin:/opt/swi/y22-ext-fx30lteM/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi:/opt/swi/y22-ext-fx30lteM/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-uclibc:/opt/swi/y22-ext-fx30lteM/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-musl:/home/pankaj/bin:/home/pankaj/.local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin:/home/pankaj/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin:/home/pankaj/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin:/home/pankaj/raspitools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin:/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/bin:/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/bin /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/bin/mkapp powerMgrTest.adef -t localhost -w /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/powerMgr/powerMgrTest/_build_powerMgrTest.localhost -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/powerMgr/powerMgrTest -c /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/powerMgr/powerMgrTest -o /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/apps

powerMgrTest: apps/test/powerMgr/powerMgrTest/CMakeFiles/powerMgrTest
powerMgrTest: apps/test/powerMgr/powerMgrTest/CMakeFiles/powerMgrTest.dir/build.make

.PHONY : powerMgrTest

# Rule to build all files generated by this target.
apps/test/powerMgr/powerMgrTest/CMakeFiles/powerMgrTest.dir/build: powerMgrTest

.PHONY : apps/test/powerMgr/powerMgrTest/CMakeFiles/powerMgrTest.dir/build

apps/test/powerMgr/powerMgrTest/CMakeFiles/powerMgrTest.dir/clean:
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/powerMgr/powerMgrTest && $(CMAKE_COMMAND) -P CMakeFiles/powerMgrTest.dir/cmake_clean.cmake
.PHONY : apps/test/powerMgr/powerMgrTest/CMakeFiles/powerMgrTest.dir/clean

apps/test/powerMgr/powerMgrTest/CMakeFiles/powerMgrTest.dir/depend:
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/powerMgr/powerMgrTest /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/powerMgr/powerMgrTest /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/powerMgr/powerMgrTest/CMakeFiles/powerMgrTest.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : apps/test/powerMgr/powerMgrTest/CMakeFiles/powerMgrTest.dir/depend
