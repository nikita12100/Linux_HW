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
CMAKE_COMMAND = /opt/clion-2020.1/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /opt/clion-2020.1/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/nikita/CLionProjects/fat_filesystem

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/nikita/CLionProjects/fat_filesystem/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/my_files.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/my_files.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/my_files.dir/flags.make

CMakeFiles/my_files.dir/src/main.c.o: CMakeFiles/my_files.dir/flags.make
CMakeFiles/my_files.dir/src/main.c.o: ../src/main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/nikita/CLionProjects/fat_filesystem/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/my_files.dir/src/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/my_files.dir/src/main.c.o   -c /home/nikita/CLionProjects/fat_filesystem/src/main.c

CMakeFiles/my_files.dir/src/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/my_files.dir/src/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/nikita/CLionProjects/fat_filesystem/src/main.c > CMakeFiles/my_files.dir/src/main.c.i

CMakeFiles/my_files.dir/src/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/my_files.dir/src/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/nikita/CLionProjects/fat_filesystem/src/main.c -o CMakeFiles/my_files.dir/src/main.c.s

CMakeFiles/my_files.dir/src/fat_filesystem.c.o: CMakeFiles/my_files.dir/flags.make
CMakeFiles/my_files.dir/src/fat_filesystem.c.o: ../src/fat_filesystem.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/nikita/CLionProjects/fat_filesystem/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/my_files.dir/src/fat_filesystem.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/my_files.dir/src/fat_filesystem.c.o   -c /home/nikita/CLionProjects/fat_filesystem/src/fat_filesystem.c

CMakeFiles/my_files.dir/src/fat_filesystem.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/my_files.dir/src/fat_filesystem.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/nikita/CLionProjects/fat_filesystem/src/fat_filesystem.c > CMakeFiles/my_files.dir/src/fat_filesystem.c.i

CMakeFiles/my_files.dir/src/fat_filesystem.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/my_files.dir/src/fat_filesystem.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/nikita/CLionProjects/fat_filesystem/src/fat_filesystem.c -o CMakeFiles/my_files.dir/src/fat_filesystem.c.s

# Object files for target my_files
my_files_OBJECTS = \
"CMakeFiles/my_files.dir/src/main.c.o" \
"CMakeFiles/my_files.dir/src/fat_filesystem.c.o"

# External object files for target my_files
my_files_EXTERNAL_OBJECTS =

../bin/my_files: CMakeFiles/my_files.dir/src/main.c.o
../bin/my_files: CMakeFiles/my_files.dir/src/fat_filesystem.c.o
../bin/my_files: CMakeFiles/my_files.dir/build.make
../bin/my_files: CMakeFiles/my_files.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/nikita/CLionProjects/fat_filesystem/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable ../bin/my_files"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/my_files.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/my_files.dir/build: ../bin/my_files

.PHONY : CMakeFiles/my_files.dir/build

CMakeFiles/my_files.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/my_files.dir/cmake_clean.cmake
.PHONY : CMakeFiles/my_files.dir/clean

CMakeFiles/my_files.dir/depend:
	cd /home/nikita/CLionProjects/fat_filesystem/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/nikita/CLionProjects/fat_filesystem /home/nikita/CLionProjects/fat_filesystem /home/nikita/CLionProjects/fat_filesystem/cmake-build-debug /home/nikita/CLionProjects/fat_filesystem/cmake-build-debug /home/nikita/CLionProjects/fat_filesystem/cmake-build-debug/CMakeFiles/my_files.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/my_files.dir/depend

