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

# Utility rule file for testFwUsers.

# Include the progress variables for this target.
include apps/test/framework/user/CMakeFiles/testFwUsers.dir/progress.make

apps/test/framework/user/CMakeFiles/testFwUsers:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "mkexe 'testFwUsers': /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/bin/testFwUsers"
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/framework/user && /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/bin/mkexe -o /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/bin/testFwUsers -w /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/framework/user/_build_testFwUsers -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/framework/user -l /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/tests/lib -t localhost userTest.c -i /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/liblegato/linux

testFwUsers: apps/test/framework/user/CMakeFiles/testFwUsers
testFwUsers: apps/test/framework/user/CMakeFiles/testFwUsers.dir/build.make

.PHONY : testFwUsers

# Rule to build all files generated by this target.
apps/test/framework/user/CMakeFiles/testFwUsers.dir/build: testFwUsers

.PHONY : apps/test/framework/user/CMakeFiles/testFwUsers.dir/build

apps/test/framework/user/CMakeFiles/testFwUsers.dir/clean:
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/framework/user && $(CMAKE_COMMAND) -P CMakeFiles/testFwUsers.dir/cmake_clean.cmake
.PHONY : apps/test/framework/user/CMakeFiles/testFwUsers.dir/clean

apps/test/framework/user/CMakeFiles/testFwUsers.dir/depend:
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/framework/user /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/framework/user /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/apps/test/framework/user/CMakeFiles/testFwUsers.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : apps/test/framework/user/CMakeFiles/testFwUsers.dir/depend
