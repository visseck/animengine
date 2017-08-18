cmake_minimum_required(VERSION 3.0)

set( CMAKE_CXX_FLAGS "-std=c++14" )

include_directories(../)

#include ( CMakeToolsHelpers OPTIONAL )

set( ANIM_EDITOR_SRCS
    main.cpp 
)

add_executable( animeditor
    main.cpp
    #${ANIM_EDITOR_SRCS}
)

target_link_libraries( animeditor
    animcore
    animpublic
    RTTR::Core_Lib
)