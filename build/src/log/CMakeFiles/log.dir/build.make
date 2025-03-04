# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.26

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/share/cmake-3.26.0/bin/cmake

# The command to remove a file.
RM = /usr/share/cmake-3.26.0/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/torch/code/linux-c++/newServer

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/torch/code/linux-c++/newServer/build

# Include any dependencies generated for this target.
include src/log/CMakeFiles/log.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/log/CMakeFiles/log.dir/compiler_depend.make

# Include the progress variables for this target.
include src/log/CMakeFiles/log.dir/progress.make

# Include the compile flags for this target's objects.
include src/log/CMakeFiles/log.dir/flags.make

src/log/CMakeFiles/log.dir/log.cpp.o: src/log/CMakeFiles/log.dir/flags.make
src/log/CMakeFiles/log.dir/log.cpp.o: /home/torch/code/linux-c++/newServer/src/log/log.cpp
src/log/CMakeFiles/log.dir/log.cpp.o: src/log/CMakeFiles/log.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/torch/code/linux-c++/newServer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/log/CMakeFiles/log.dir/log.cpp.o"
	cd /home/torch/code/linux-c++/newServer/build/src/log && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/log/CMakeFiles/log.dir/log.cpp.o -MF CMakeFiles/log.dir/log.cpp.o.d -o CMakeFiles/log.dir/log.cpp.o -c /home/torch/code/linux-c++/newServer/src/log/log.cpp

src/log/CMakeFiles/log.dir/log.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/log.dir/log.cpp.i"
	cd /home/torch/code/linux-c++/newServer/build/src/log && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/torch/code/linux-c++/newServer/src/log/log.cpp > CMakeFiles/log.dir/log.cpp.i

src/log/CMakeFiles/log.dir/log.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/log.dir/log.cpp.s"
	cd /home/torch/code/linux-c++/newServer/build/src/log && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/torch/code/linux-c++/newServer/src/log/log.cpp -o CMakeFiles/log.dir/log.cpp.s

# Object files for target log
log_OBJECTS = \
"CMakeFiles/log.dir/log.cpp.o"

# External object files for target log
log_EXTERNAL_OBJECTS =

src/log/liblog.a: src/log/CMakeFiles/log.dir/log.cpp.o
src/log/liblog.a: src/log/CMakeFiles/log.dir/build.make
src/log/liblog.a: src/log/CMakeFiles/log.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/torch/code/linux-c++/newServer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library liblog.a"
	cd /home/torch/code/linux-c++/newServer/build/src/log && $(CMAKE_COMMAND) -P CMakeFiles/log.dir/cmake_clean_target.cmake
	cd /home/torch/code/linux-c++/newServer/build/src/log && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/log.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/log/CMakeFiles/log.dir/build: src/log/liblog.a
.PHONY : src/log/CMakeFiles/log.dir/build

src/log/CMakeFiles/log.dir/clean:
	cd /home/torch/code/linux-c++/newServer/build/src/log && $(CMAKE_COMMAND) -P CMakeFiles/log.dir/cmake_clean.cmake
.PHONY : src/log/CMakeFiles/log.dir/clean

src/log/CMakeFiles/log.dir/depend:
	cd /home/torch/code/linux-c++/newServer/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/torch/code/linux-c++/newServer /home/torch/code/linux-c++/newServer/src/log /home/torch/code/linux-c++/newServer/build /home/torch/code/linux-c++/newServer/build/src/log /home/torch/code/linux-c++/newServer/build/src/log/CMakeFiles/log.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/log/CMakeFiles/log.dir/depend

