cmake_minimum_required(VERSION 3.0)
include ( CMakeToolsHelpers OPTIONAL )

project(animengine)

set( BUILD_RTTR_DYNAMIC OFF CACHE BOOL "Build RTTR Dynamic")
set( BUILD_STATIC ON CACHE BOOL "Build RTTR Static")
set( BUILD_INSTALLER OFF CACHE BOOL "Build RTTR Installer")
add_subdirectory(extern/rttr)

add_subdirectory(animpublic)
add_subdirectory(animcore)
add_subdirectory(animeditor)
#add_subdirectory(animruntime)
add_subdirectory(animtest)