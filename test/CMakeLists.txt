# CMakeLists.txt
#
#     Author: Fabian Meyer
# Created On: 21 Feb 2021

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}
    ${CATCH2_INCLUDE_DIR}
)

set(TEST_SRC
    "main.cpp"
)

add_executable(run_tests ${TEST_SRC})
target_link_libraries(run_tests Threads::Threads)