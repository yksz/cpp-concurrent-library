include_directories(
    ${PROJECT_SOURCE_DIR}/src
    /usr/local/include
)
link_directories(/usr/local/lib)

set(tests
    actor_test
    blocking_queue_test
    countdown_latch_test
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
