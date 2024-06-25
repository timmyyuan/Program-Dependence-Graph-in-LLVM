cmake_minimum_required(VERSION 3.1)
project(pdg)

# To support both in- and out-of-source builds,
# we check for the presence of the add_llvm_loadable_module command.
# - if this command is not present, we are building out-of-source
if(NOT COMMAND add_llvm_library)
    if (DEFINED LLVM_DIR)
        set(ENV{LLVM_DIR} "${LLVM_DIR}")
    endif()
    if (DEFINED ENV{LLVM_DIR})
        # We need to match the build environment for LLVM:
        # In particular, we need C++14 and the -fno-rtti flag
        set(CMAKE_CXX_STANDARD 17)
        set(CMAKE_CXX_STANDARD_REQUIRED ON)
        # add -std=gnu++14
        set(CMAKE_CXX_EXTENSIONS ON)

        set(COMMON_FLAGS "-fPIC -Werror -pthread")

        if(CMAKE_BUILD_TYPE MATCHES "Debug")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} -O0 -fno-rtti")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS} -O0")
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} -O3 -fno-rtti")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS} -O3")
        endif()

        find_package(LLVM REQUIRED CONFIG PATHS /usr/local/lib/cmake)
        list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
        include(AddLLVM)

        get_filename_component(LLVM_PREFIX_DIR "${LLVM_DIR}/../../.." ABSOLUTE)
        message("LLVM STATUS:
            Definitions     ${LLVM_DEFINITIONS}
            Includes        ${LLVM_INCLUDE_DIRS}
            Libraries       ${LLVM_LIBRARY_DIRS}
            Targets         ${LLVM_TARGETS_TO_BUILD}
            LLVM_DIR        ${LLVM_DIR}
            LLVM_PREFIX_DIR ${LLVM_PREFIX_DIR}"
        )

        add_definitions(${LLVM_DEFINITIONS})
        include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})

        if (LLVM_LINK_LLVM_DYLIB)
            set(llvm_libs LLVM)
        else()
            llvm_map_components_to_libnames(llvm_libs
                passes bitwriter core ipo irreader instcombine instrumentation 
                target linker analysis scalaropts support transformutils)
        endif()

    else()
        message(FATAL_ERROR "\
WARNING: The LLVM_DIR var was not set (required for an out-of-source build)!\n\
Please set this to environment variable to point to the LLVM build directory\
(e.g. on linux: export LLVM_DIR=/path/to/llvm/build/dir)")
    endif()
else()
        set(IN_SOURCE_BUILD 1)
endif()

set (CMAKE_CXX_STANDARD 17)

#
# We will build one library: libtmplugin.so.  It corresponds to a plugin that we
# run when compiling.
#
#option(USE_CXX_EXCEPTIONS "Enable C++ exception support" ON)
#
# Files associated with libtmplugin.so
#

include_directories("include")
include_directories("src")

file(GLOB TPPSRC "src/*.tpp")
file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "include/*.hpp")

#build static libray to be used by pdgtest
add_library(pdgStatic STATIC
    include/tree.hh
    ${HEADERS}
    ${TPPSRC}
    ${SOURCES}
)

# build plugin to be used by opt
add_library(pdg MODULE
    include/tree.hh
    ${HEADERS}
    ${TPPSRC}
    ${SOURCES}
)

#
# Turn on C++11, turn off C++ RTTI.
#
# target_compile_features(pdg PRIVATE cxx_range_for cxx_auto_type)
# set_target_properties(pdg PROPERTIES COMPILE_FLAGS "-fno-rtti")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} -Wno-error=deprecated-declarations")

#
# OS X-specific configuration
#
if(APPLE)
    set_target_properties(pdg PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
endif(APPLE)

