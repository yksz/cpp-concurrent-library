### Coverage
# usage: 1. make test
#        2. gcov *.gcno
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")

include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/vendor/googletest/googletest/include
#    /usr/local/include # if googletest is installed
)
link_directories(
#    /usr/local/lib # if googletest is installed
)

set(tests
    actor_test
    blocking_queue_test
    channel_test
    countdown_latch_test
    pubsub_test
    scheduler_test
    thread_pool_test
)

set(test_libraries
    gtest
    gtest_main
    pthread
)

foreach(test IN LISTS tests)
    add_executable(${test} ${test}.cpp odr_check.cpp)
    target_link_libraries(${test} ${test_libraries})
    add_test(${test} ${test})
endforeach()
