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

# Utility rule file for user_docs.

# Include the progress variables for this target.
include framework/doc/CMakeFiles/user_docs.dir/progress.make

framework/doc/CMakeFiles/user_docs:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating HTML for API documentation"
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/framework/doc && cp /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/doc/static/legato.css /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/framework/doc
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/framework/doc && mkdir -p /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/doc/user && /usr/bin/doxygen /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/framework/doc/doxygen.user.cfg
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/framework/doc && cp /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/doc/static/navtree.css /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/doc/user/html
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/framework/doc && cp /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/doc/static/tabs.css /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/doc/user/html
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/framework/doc && cp /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/doc/static/fonts.css /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/doc/user/html
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/framework/doc && cp /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/doc/static/opensans.ttf /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/doc/user/html
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/framework/doc && cp /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/doc/static/opensans-light.ttf /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/doc/user/html
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/framework/doc && cp /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/doc/static/opensans-bold.ttf /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/doc/user/html
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/framework/doc && cp /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/doc/static/favicon.ico /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/doc/user/html
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/framework/doc && cp /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/doc/static/swi-ico-medium.png /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/doc/user/html
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/framework/doc && cp /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/doc/static/ftv2mnode.png /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/doc/user/html
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/framework/doc && cp /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/doc/static/ftv2mlastnode.png /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/doc/user/html
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/framework/doc && cp /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/doc/static/ftv2pnode.png /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/doc/user/html
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/framework/doc && cp /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/doc/static/ftv2plastnode.png /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/doc/user/html
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/framework/doc && cp /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/doc/static/ftv2doc.png /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/doc/user/html
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/framework/doc && cp /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/doc/static/ftv2folderopen.png /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/doc/user/html
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/framework/doc && cp /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/doc/static/resize.js /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/doc/user/html
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/framework/doc && cp /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/doc/static/search.css /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/doc/user/html/search

user_docs: framework/doc/CMakeFiles/user_docs
user_docs: framework/doc/CMakeFiles/user_docs.dir/build.make

.PHONY : user_docs

# Rule to build all files generated by this target.
framework/doc/CMakeFiles/user_docs.dir/build: user_docs

.PHONY : framework/doc/CMakeFiles/user_docs.dir/build

framework/doc/CMakeFiles/user_docs.dir/clean:
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/framework/doc && $(CMAKE_COMMAND) -P CMakeFiles/user_docs.dir/cmake_clean.cmake
.PHONY : framework/doc/CMakeFiles/user_docs.dir/clean

framework/doc/CMakeFiles/user_docs.dir/depend:
	cd /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/doc /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/framework/doc /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/framework/doc/CMakeFiles/user_docs.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : framework/doc/CMakeFiles/user_docs.dir/depend

