cmake_minimum_required(VERSION 3.0)

set( CMAKE_CXX_FLAGS "-std=c++14" )

include_directories(../)

#include ( CMakeToolsHelpers OPTIONAL )

SET( TEST_SRCS
    core_commands_integration.cpp
    core_commands_integration.h
    main.cpp
)

add_executable( animtest
    ${TEST_SRCS}
)

target_link_libraries( animtest
    animcore
    animpublic
    RTTR::Core_Lib
)
