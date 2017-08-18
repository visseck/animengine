cmake_minimum_required(VERSION 3.0)

set( CMAKE_CXX_FLAGS "-std=c++14" )

include_directories(../)

#include ( CMakeToolsHelpers OPTIONAL )

set( COMMAND_SRCS
    commands/core_commands.h
    commands/editor_commands.h
)

set( INTERFACES_SRCS
    interfaces/i_engine_interface.h
)

set( DEFAULT_SRCS
    guid.cpp
    guid.h
    i_array_interface.h
    namespace.h
    opaque_handle.h
    types.h
)

add_library( animpublic
    ${COMMAND_SRCS}
    ${INTERFACES_SRCS}
    ${DEFAULT_SRCS}
)