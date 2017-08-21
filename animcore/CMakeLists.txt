cmake_minimum_required(VERSION 3.0)

set( CMAKE_CXX_FLAGS "-std=c++14" )

include_directories(../)

#include ( CMakeToolsHelpers OPTIONAL )

set( CONTAINER_SRCS
    containers/array.h
    containers/singleton.h
    containers/string.h
)

set( INTERFACE_SRCS
    interface/engine_interface.cpp
    interface/engine_interface.h
)

set( MATH_SRCS
    math/fixed_precision.h
    math/hash64.h
    math/quaternion.h
    math/utils.h
    math/vector3.h
)

set( MEMORY_SRCS
    memory/default_allocator.h
    memory/pointers.h
)

set ( OBJECT_MODEL_SRCS
    objectmodel/managed_object.cpp
    objectmodel/managed_object.h
    objectmodel/object_id.cpp
    objectmodel/object_id.h
    objectmodel/object_manager.h
    objectmodel/object.h
    objectmodel/reference.h
)

set( REMOTE_PROTOCOL_SRCS
    remoteprotocol/object_wrapper.h
	remoteprotocol/pipe_server.h
	remoteprotocol/pipe_server.cpp
	remoteprotocol/pipe_client.h
	remoteprotocol/pipe_client.cpp
	remoteprotocol/message.h
)

set( SERIALIZATION_SRCS
    serialization/object_serializer.cpp
    serialization/object_serializer.h
    serialization/serialization.h
)

set( UTIL_SRCS
    util/assert.h
    util/namespace.h
)

add_library( animcore
    ${CONTAINER_SRCS}
    ${INTERFACE_SRCS}
    ${MATH_SRCS}
    ${MEMORY_SRCS}
    ${OBJECT_MODEL_SRCS}
    ${REMOTE_PROTOCOL_SRCS}
    ${SERIALIZATION_SRCS}
    ${UTIL_SRCS}
)

target_link_libraries( animcore
    animpublic
    RTTR::Core_Lib
)


source_group( containers
    FILES
    ${CONTAINER_SRCS}
)

source_group( interfaces
    FILES
    ${INTERFACE_SRCS}
)

source_group( math
    FILES
    ${MATH_SRCS}
)

source_group ( memory
    FILES
    ${MEMORY_SRCS}
)

source_group ( natvis
    FILES
    natvis/animcore.natvis
)

source_group ( objectmodel
    FILES
    ${OBJECT_MODEL_SRCS}
)

source_group ( remoteprotocol
    FILES
    ${REMOTE_PROTOCOL_SRCS}
)

source_group ( serialization
    FILES
    ${SERIALIZATION_SRCS}
)

source_group ( util
    FILES
    ${UTIL_SRCS}
)
