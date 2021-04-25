cmake_minimum_required(VERSION 3.5)

set(CMAKE_C_STANDARD 11)

project(my_fs2)

find_package(Threads REQUIRED)
link_libraries(${CMAKE_THREAD_LIBS_INIT})

add_library(fs_core STATIC src/fs.c src/manage.c src/myutil.c)
target_include_directories(fs_core PUBLIC include)

add_library(fs_net STATIC src/net.c src/serializers.c)
target_link_libraries(fs_net fs_core)

add_executable(my_fs2 src/main.c)
target_link_libraries(my_fs2 fs_net)
