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

# Utility rule file for infoUnitTest.

# Include the progress variables for this target.
include apps/test/modemServices/info/infoUnitTest/CMakeFiles/infoUnitTest.dir/progress.make

apps/test/modemServices/info/infoUnitTest/CMakeFiles/infoUnitTest:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "mkexe 'infoUnitTest': /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/bin/infoUnitTest"
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/modemServices/info/infoUnitTest && /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/bin/mkexe -o /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/bin/infoUnitTest -w /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/modemServices/info/infoUnitTest/_build_infoUnitTest -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/modemServices/info/infoUnitTest -l /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/lib -t localhost . -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/components/modemServices//modemDaemon -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/components/modemServices//platformAdaptor/inc -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/liblegato -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/platformAdaptor/simu/components/le_pa -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/platformAdaptor/simu/components/simuConfig -s /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/platformAdaptor --cflags="-DWITHOUT_SIMUCONFIG"

infoUnitTest: apps/test/modemServices/info/infoUnitTest/CMakeFiles/infoUnitTest
infoUnitTest: apps/test/modemServices/info/infoUnitTest/CMakeFiles/infoUnitTest.dir/build.make

.PHONY : infoUnitTest

# Rule to build all files generated by this target.
apps/test/modemServices/info/infoUnitTest/CMakeFiles/infoUnitTest.dir/build: infoUnitTest

.PHONY : apps/test/modemServices/info/infoUnitTest/CMakeFiles/infoUnitTest.dir/build

apps/test/modemServices/info/infoUnitTest/CMakeFiles/infoUnitTest.dir/clean:
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/modemServices/info/infoUnitTest && $(CMAKE_COMMAND) -P CMakeFiles/infoUnitTest.dir/cmake_clean.cmake
.PHONY : apps/test/modemServices/info/infoUnitTest/CMakeFiles/infoUnitTest.dir/clean

apps/test/modemServices/info/infoUnitTest/CMakeFiles/infoUnitTest.dir/depend:
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/modemServices/info/infoUnitTest /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/modemServices/info/infoUnitTest /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/modemServices/info/infoUnitTest/CMakeFiles/infoUnitTest.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : apps/test/modemServices/info/infoUnitTest/CMakeFiles/infoUnitTest.dir/depend

