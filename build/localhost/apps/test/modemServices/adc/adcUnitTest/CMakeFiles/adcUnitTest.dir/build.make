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

# Utility rule file for adcUnitTest.

# Include the progress variables for this target.
include apps/test/modemServices/adc/adcUnitTest/CMakeFiles/adcUnitTest.dir/progress.make

apps/test/modemServices/adc/adcUnitTest/CMakeFiles/adcUnitTest:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "mkexe 'adcUnitTest': /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/bin/adcUnitTest"
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/modemServices/adc/adcUnitTest && /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/bin/mkexe -o /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/bin/adcUnitTest -w /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/modemServices/adc/adcUnitTest/_build_adcUnitTest -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/modemServices/adc/adcUnitTest -l /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/lib -t localhost . -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/components/modemServices//modemDaemon -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/components/modemServices//platformAdaptor/inc -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/liblegato -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/platformAdaptor/simu/components/le_pa -s /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/platformAdaptor

adcUnitTest: apps/test/modemServices/adc/adcUnitTest/CMakeFiles/adcUnitTest
adcUnitTest: apps/test/modemServices/adc/adcUnitTest/CMakeFiles/adcUnitTest.dir/build.make

.PHONY : adcUnitTest

# Rule to build all files generated by this target.
apps/test/modemServices/adc/adcUnitTest/CMakeFiles/adcUnitTest.dir/build: adcUnitTest

.PHONY : apps/test/modemServices/adc/adcUnitTest/CMakeFiles/adcUnitTest.dir/build

apps/test/modemServices/adc/adcUnitTest/CMakeFiles/adcUnitTest.dir/clean:
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/modemServices/adc/adcUnitTest && $(CMAKE_COMMAND) -P CMakeFiles/adcUnitTest.dir/cmake_clean.cmake
.PHONY : apps/test/modemServices/adc/adcUnitTest/CMakeFiles/adcUnitTest.dir/clean

apps/test/modemServices/adc/adcUnitTest/CMakeFiles/adcUnitTest.dir/depend:
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/modemServices/adc/adcUnitTest /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/modemServices/adc/adcUnitTest /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/modemServices/adc/adcUnitTest/CMakeFiles/adcUnitTest.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : apps/test/modemServices/adc/adcUnitTest/CMakeFiles/adcUnitTest.dir/depend
