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

# Utility rule file for portServiceUnitTest.

# Include the progress variables for this target.
include apps/test/portService/portServiceUnitTest/CMakeFiles/portServiceUnitTest.dir/progress.make

apps/test/portService/portServiceUnitTest/CMakeFiles/portServiceUnitTest:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "mkexe 'portServiceUnitTest': /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/bin/portServiceUnitTest"
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/portService/portServiceUnitTest && /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/bin/mkexe -o /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/bin/portServiceUnitTest -w /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/portService/portServiceUnitTest/_build_portServiceUnitTest -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/portService/portServiceUnitTest -l /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/lib -t localhost portServiceComp . -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/components/portService/portDaemon -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/components/atServices -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/components/atServices/Common -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/liblegato/linux/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/interfaces/atServices/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/interfaces/portService/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/interfaces/ -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/components/watchdogChain/ -C -fvisibility=default\ -g

portServiceUnitTest: apps/test/portService/portServiceUnitTest/CMakeFiles/portServiceUnitTest
portServiceUnitTest: apps/test/portService/portServiceUnitTest/CMakeFiles/portServiceUnitTest.dir/build.make

.PHONY : portServiceUnitTest

# Rule to build all files generated by this target.
apps/test/portService/portServiceUnitTest/CMakeFiles/portServiceUnitTest.dir/build: portServiceUnitTest

.PHONY : apps/test/portService/portServiceUnitTest/CMakeFiles/portServiceUnitTest.dir/build

apps/test/portService/portServiceUnitTest/CMakeFiles/portServiceUnitTest.dir/clean:
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/portService/portServiceUnitTest && $(CMAKE_COMMAND) -P CMakeFiles/portServiceUnitTest.dir/cmake_clean.cmake
.PHONY : apps/test/portService/portServiceUnitTest/CMakeFiles/portServiceUnitTest.dir/clean

apps/test/portService/portServiceUnitTest/CMakeFiles/portServiceUnitTest.dir/depend:
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/portService/portServiceUnitTest /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/portService/portServiceUnitTest /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/portService/portServiceUnitTest/CMakeFiles/portServiceUnitTest.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : apps/test/portService/portServiceUnitTest/CMakeFiles/portServiceUnitTest.dir/depend

