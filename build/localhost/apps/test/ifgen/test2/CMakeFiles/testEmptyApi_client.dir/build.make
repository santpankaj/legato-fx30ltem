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

# Utility rule file for testEmptyApi_client.

# Include the progress variables for this target.
include apps/test/ifgen/test2/CMakeFiles/testEmptyApi_client.dir/progress.make

apps/test/ifgen/test2/CMakeFiles/testEmptyApi_client:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "mkexe 'testEmptyApi_client': /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/bin/testEmptyApi_client"
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/ifgen/test2 && /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/bin/mkexe -o /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/bin/testEmptyApi_client -w /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/ifgen/test2/_build_testEmptyApi_client -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/ifgen/test2 -l /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/lib -t localhost testEmptyApi_client

testEmptyApi_client: apps/test/ifgen/test2/CMakeFiles/testEmptyApi_client
testEmptyApi_client: apps/test/ifgen/test2/CMakeFiles/testEmptyApi_client.dir/build.make

.PHONY : testEmptyApi_client

# Rule to build all files generated by this target.
apps/test/ifgen/test2/CMakeFiles/testEmptyApi_client.dir/build: testEmptyApi_client

.PHONY : apps/test/ifgen/test2/CMakeFiles/testEmptyApi_client.dir/build

apps/test/ifgen/test2/CMakeFiles/testEmptyApi_client.dir/clean:
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/ifgen/test2 && $(CMAKE_COMMAND) -P CMakeFiles/testEmptyApi_client.dir/cmake_clean.cmake
.PHONY : apps/test/ifgen/test2/CMakeFiles/testEmptyApi_client.dir/clean

apps/test/ifgen/test2/CMakeFiles/testEmptyApi_client.dir/depend:
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/ifgen/test2 /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/ifgen/test2 /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/ifgen/test2/CMakeFiles/testEmptyApi_client.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : apps/test/ifgen/test2/CMakeFiles/testEmptyApi_client.dir/depend
