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

# Utility rule file for le_mrc_if.

# Include the progress variables for this target.
include interfaces/CMakeFiles/le_mrc_if.dir/progress.make

interfaces/CMakeFiles/le_mrc_if: interfaces/le_mrc_interface.h


interfaces/le_mrc_interface.h: ../../interfaces/modemServices/le_mrc.api
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "ifgen 'modemServices/le_mrc.api': /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/interfaces/le_mrc_interface.h"
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/interfaces && ../../../bin/ifgen --gen-interface /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/interfaces/modemServices/le_mrc.api --import-dir /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/interfaces/audio --import-dir /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/interfaces/modemServices --import-dir /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/interfaces/atServices --import-dir /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/interfaces

le_mrc_if: interfaces/CMakeFiles/le_mrc_if
le_mrc_if: interfaces/le_mrc_interface.h
le_mrc_if: interfaces/CMakeFiles/le_mrc_if.dir/build.make

.PHONY : le_mrc_if

# Rule to build all files generated by this target.
interfaces/CMakeFiles/le_mrc_if.dir/build: le_mrc_if

.PHONY : interfaces/CMakeFiles/le_mrc_if.dir/build

interfaces/CMakeFiles/le_mrc_if.dir/clean:
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/interfaces && $(CMAKE_COMMAND) -P CMakeFiles/le_mrc_if.dir/cmake_clean.cmake
.PHONY : interfaces/CMakeFiles/le_mrc_if.dir/clean

interfaces/CMakeFiles/le_mrc_if.dir/depend:
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/interfaces /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/interfaces /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/interfaces/CMakeFiles/le_mrc_if.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : interfaces/CMakeFiles/le_mrc_if.dir/depend

