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

# Utility rule file for dcsTest.

# Include the progress variables for this target.
include apps/test/dataConnectionService/dataConnectionServiceTest/CMakeFiles/dcsTest.dir/progress.make

apps/test/dataConnectionService/dataConnectionServiceTest/CMakeFiles/dcsTest:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "mkapp 'dcsTest': /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/apps/dcsTest.localhost"
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/dataConnectionService/dataConnectionServiceTest && PATH=/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/bin:/usr/bin/:/opt/swi/y22-ext-fx30lteM/sysroots/x86_64-pokysdk-linux/usr/bin:/opt/swi/y22-ext-fx30lteM/sysroots/x86_64-pokysdk-linux/usr/sbin:/opt/swi/y22-ext-fx30lteM/sysroots/x86_64-pokysdk-linux/bin:/opt/swi/y22-ext-fx30lteM/sysroots/x86_64-pokysdk-linux/sbin:/opt/swi/y22-ext-fx30lteM/sysroots/x86_64-pokysdk-linux/usr/bin/../x86_64-pokysdk-linux/bin:/opt/swi/y22-ext-fx30lteM/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi:/opt/swi/y22-ext-fx30lteM/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-uclibc:/opt/swi/y22-ext-fx30lteM/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-musl:/home/pankaj/bin:/home/pankaj/.local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin:/home/pankaj/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin:/home/pankaj/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin:/home/pankaj/raspitools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin:/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/bin:/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/bin /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/bin/mkapp dcsTest.adef -t localhost -w /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/dataConnectionService/dataConnectionServiceTest/_build_dcsTest.localhost -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/dataConnectionService/dataConnectionServiceTest -c /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/dataConnectionService/dataConnectionServiceTest -o /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/apps -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/interfaces/modemServices

dcsTest: apps/test/dataConnectionService/dataConnectionServiceTest/CMakeFiles/dcsTest
dcsTest: apps/test/dataConnectionService/dataConnectionServiceTest/CMakeFiles/dcsTest.dir/build.make

.PHONY : dcsTest

# Rule to build all files generated by this target.
apps/test/dataConnectionService/dataConnectionServiceTest/CMakeFiles/dcsTest.dir/build: dcsTest

.PHONY : apps/test/dataConnectionService/dataConnectionServiceTest/CMakeFiles/dcsTest.dir/build

apps/test/dataConnectionService/dataConnectionServiceTest/CMakeFiles/dcsTest.dir/clean:
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/dataConnectionService/dataConnectionServiceTest && $(CMAKE_COMMAND) -P CMakeFiles/dcsTest.dir/cmake_clean.cmake
.PHONY : apps/test/dataConnectionService/dataConnectionServiceTest/CMakeFiles/dcsTest.dir/clean

apps/test/dataConnectionService/dataConnectionServiceTest/CMakeFiles/dcsTest.dir/depend:
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/dataConnectionService/dataConnectionServiceTest /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/dataConnectionService/dataConnectionServiceTest /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/dataConnectionService/dataConnectionServiceTest/CMakeFiles/dcsTest.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : apps/test/dataConnectionService/dataConnectionServiceTest/CMakeFiles/dcsTest.dir/depend

