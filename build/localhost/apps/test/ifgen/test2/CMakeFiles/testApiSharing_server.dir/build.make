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

# Utility rule file for testApiSharing_server.

# Include the progress variables for this target.
include apps/test/ifgen/test2/CMakeFiles/testApiSharing_server.dir/progress.make

apps/test/ifgen/test2/CMakeFiles/testApiSharing_server:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "mkexe 'testApiSharing_server': /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/bin/testApiSharing_server"
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/ifgen/test2 && /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/bin/mkexe -o /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/bin/testApiSharing_server -w /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/ifgen/test2/_build_testApiSharing_server -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/ifgen/test2 -l /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/lib -t localhost testApiSharing_server

testApiSharing_server: apps/test/ifgen/test2/CMakeFiles/testApiSharing_server
testApiSharing_server: apps/test/ifgen/test2/CMakeFiles/testApiSharing_server.dir/build.make

.PHONY : testApiSharing_server

# Rule to build all files generated by this target.
apps/test/ifgen/test2/CMakeFiles/testApiSharing_server.dir/build: testApiSharing_server

.PHONY : apps/test/ifgen/test2/CMakeFiles/testApiSharing_server.dir/build

apps/test/ifgen/test2/CMakeFiles/testApiSharing_server.dir/clean:
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/ifgen/test2 && $(CMAKE_COMMAND) -P CMakeFiles/testApiSharing_server.dir/cmake_clean.cmake
.PHONY : apps/test/ifgen/test2/CMakeFiles/testApiSharing_server.dir/clean

apps/test/ifgen/test2/CMakeFiles/testApiSharing_server.dir/depend:
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/ifgen/test2 /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/ifgen/test2 /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/ifgen/test2/CMakeFiles/testApiSharing_server.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : apps/test/ifgen/test2/CMakeFiles/testApiSharing_server.dir/depend

