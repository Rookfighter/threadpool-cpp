# CMakeLists.txt
#
#     Author: Fabian Meyer
# Created On: 21 Feb 2021

add_executable(simple_work "simple_work.cpp")
target_link_libraries(simple_work Threads::Threads)

add_executable(time_improvement "time_improvement.cpp")
target_link_libraries(time_improvement Threads::Threads)