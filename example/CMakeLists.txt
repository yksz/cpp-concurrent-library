# pthread
if(UNIX) # include Linux
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

include_directories(
    /usr/include # boost
    /usr/local/include # boost
    ${PROJECT_SOURCE_DIR}/src
)

set(examples
    actor_boost_example
    actor_example
    actor_performance_example
    blocking_queue_example
    channel_example
    continuation_example
    countdown_latch_example
    pubsub_boost_example
    pubsub_example
    scheduler_example
    thread_pool_example
)

foreach(example IN LISTS examples)
    add_executable(${example} ${example}.cpp)
endforeach()
