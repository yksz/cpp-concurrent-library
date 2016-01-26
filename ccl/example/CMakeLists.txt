# pthread
if(UNIX) # include Linux
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

include_directories(
    /usr/include # for boost
    ${PROJECT_SOURCE_DIR}/src
)

set(examples
    actor_boost_example
    actor_example
)

foreach(example IN LISTS examples)
    add_executable(${example} ${example}.cpp)
endforeach()
