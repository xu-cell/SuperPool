# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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
CMAKE_SOURCE_DIR = /home/ecs-user/SuperPool

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ecs-user/SuperPool/build

# Include any dependencies generated for this target.
include CMakeFiles/SuperPool.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/SuperPool.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/SuperPool.dir/flags.make

CMakeFiles/SuperPool.dir/src/threadpool.cc.o: CMakeFiles/SuperPool.dir/flags.make
CMakeFiles/SuperPool.dir/src/threadpool.cc.o: ../src/threadpool.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ecs-user/SuperPool/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/SuperPool.dir/src/threadpool.cc.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/SuperPool.dir/src/threadpool.cc.o -c /home/ecs-user/SuperPool/src/threadpool.cc

CMakeFiles/SuperPool.dir/src/threadpool.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/SuperPool.dir/src/threadpool.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ecs-user/SuperPool/src/threadpool.cc > CMakeFiles/SuperPool.dir/src/threadpool.cc.i

CMakeFiles/SuperPool.dir/src/threadpool.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/SuperPool.dir/src/threadpool.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ecs-user/SuperPool/src/threadpool.cc -o CMakeFiles/SuperPool.dir/src/threadpool.cc.s

# Object files for target SuperPool
SuperPool_OBJECTS = \
"CMakeFiles/SuperPool.dir/src/threadpool.cc.o"

# External object files for target SuperPool
SuperPool_EXTERNAL_OBJECTS =

libSuperPool.so: CMakeFiles/SuperPool.dir/src/threadpool.cc.o
libSuperPool.so: CMakeFiles/SuperPool.dir/build.make
libSuperPool.so: CMakeFiles/SuperPool.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ecs-user/SuperPool/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX shared library libSuperPool.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/SuperPool.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/SuperPool.dir/build: libSuperPool.so

.PHONY : CMakeFiles/SuperPool.dir/build

CMakeFiles/SuperPool.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/SuperPool.dir/cmake_clean.cmake
.PHONY : CMakeFiles/SuperPool.dir/clean

CMakeFiles/SuperPool.dir/depend:
	cd /home/ecs-user/SuperPool/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ecs-user/SuperPool /home/ecs-user/SuperPool /home/ecs-user/SuperPool/build /home/ecs-user/SuperPool/build /home/ecs-user/SuperPool/build/CMakeFiles/SuperPool.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/SuperPool.dir/depend

