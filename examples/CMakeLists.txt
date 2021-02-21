# CMakeLists.txt
#
#     Author: Fabian Meyer
# Created On: 21 Feb 2021

add_executable(helloworld "helloworld.cpp")
target_link_libraries(helloworld Threads::Threads)