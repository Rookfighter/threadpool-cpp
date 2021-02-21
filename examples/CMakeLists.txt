# CMakeLists.txt
#
#     Author: Fabian Meyer
# Created On: 21 Feb 2021

add_executable(simple_work "simple_work.cpp")
target_link_libraries(simple_work Threads::Threads)