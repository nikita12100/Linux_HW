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
CMAKE_SOURCE_DIR = /home/nikita/CLionProjects/Linux_HW/fs_extended

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/nikita/CLionProjects/Linux_HW/fs_extended

# Include any dependencies generated for this target.
include CMakeFiles/my_fs2.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/my_fs2.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/my_fs2.dir/flags.make

CMakeFiles/my_fs2.dir/src/main.c.o: CMakeFiles/my_fs2.dir/flags.make
CMakeFiles/my_fs2.dir/src/main.c.o: src/main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/nikita/CLionProjects/Linux_HW/fs_extended/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/my_fs2.dir/src/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/my_fs2.dir/src/main.c.o   -c /home/nikita/CLionProjects/Linux_HW/fs_extended/src/main.c

CMakeFiles/my_fs2.dir/src/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/my_fs2.dir/src/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/nikita/CLionProjects/Linux_HW/fs_extended/src/main.c > CMakeFiles/my_fs2.dir/src/main.c.i

CMakeFiles/my_fs2.dir/src/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/my_fs2.dir/src/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/nikita/CLionProjects/Linux_HW/fs_extended/src/main.c -o CMakeFiles/my_fs2.dir/src/main.c.s

# Object files for target my_fs2
my_fs2_OBJECTS = \
"CMakeFiles/my_fs2.dir/src/main.c.o"

# External object files for target my_fs2
my_fs2_EXTERNAL_OBJECTS =

my_fs2: CMakeFiles/my_fs2.dir/src/main.c.o
my_fs2: CMakeFiles/my_fs2.dir/build.make
my_fs2: libfs_net.a
my_fs2: libfs_core.a
my_fs2: CMakeFiles/my_fs2.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/nikita/CLionProjects/Linux_HW/fs_extended/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable my_fs2"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/my_fs2.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/my_fs2.dir/build: my_fs2

.PHONY : CMakeFiles/my_fs2.dir/build

CMakeFiles/my_fs2.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/my_fs2.dir/cmake_clean.cmake
.PHONY : CMakeFiles/my_fs2.dir/clean

CMakeFiles/my_fs2.dir/depend:
	cd /home/nikita/CLionProjects/Linux_HW/fs_extended && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/nikita/CLionProjects/Linux_HW/fs_extended /home/nikita/CLionProjects/Linux_HW/fs_extended /home/nikita/CLionProjects/Linux_HW/fs_extended /home/nikita/CLionProjects/Linux_HW/fs_extended /home/nikita/CLionProjects/Linux_HW/fs_extended/CMakeFiles/my_fs2.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/my_fs2.dir/depend
