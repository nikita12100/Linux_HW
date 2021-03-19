cmake_minimum_required(VERSION 3.16)
project(my_fs C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_executable(my_fs src/main.c src/my_filesystem.c)
