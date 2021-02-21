# CMakeLists.txt
#
#     Author: Fabian Meyer
# Created On: 21 Feb 2021

add_executable(full_work_example "full_work_example.cpp")
target_link_libraries(full_work_example Threads::Threads)

add_executable(single_work "single_work.cpp")
target_link_libraries(single_work Threads::Threads)

add_executable(time_improvement "time_improvement.cpp")
target_link_libraries(time_improvement Threads::Threads)