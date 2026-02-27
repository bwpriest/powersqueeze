# Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
# powersqueeze Project Developers. See the top-level COPYRIGHT file for details.

cmake_minimum_required(VERSION 3.25)

project(
    powersqueeze
    VERSION 0.1
    DESCRIPTION "HPC power iteration projection toolbox"
    LANGUAGES C CXX
)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(PSQZ_MAIN_PROJECT ON)
endif ()

# Controls whether to set up install boilerplate for project and depencencies.
# Expects CMAKE_INSTALL_PREFIX to be set to a suitable directory.
option(PSQZ_INSTALL_HDKNN "install dependencies and build hdknn library" OFF)
set(PSQZ_ROOT ${CMAKE_SOURCE_DIR})

#
# FetchContent dependencies that can be overridden on the command line.
#
set(YGM_FETCH_GIT
    "https://github.com/LLNL/ygm.git"
    CACHE STRING "GIT to fetch YGM")
set(YGM_FETCH_TAG
    "v0.10-dev"
    CACHE STRING "Tag to fetch YGM")
set(KROWKEE_FETCH_GIT
    "https://github.com/LLNL/krowkee.git"
    CACHE STRING "GIT to fetch krowkee")
set(KROWKEE_FETCH_TAG
    "v0.4-dev"
    CACHE STRING "Tag to fetch krowkee")
set(SALTATLAS_FETCH_GIT
    "https://github.com/LLNL/saltatlas.git"
    CACHE STRING "GIT to fetch saltatlas")
set(SALTATLAS_FETCH_TAG
    "v0.3-dev"
    CACHE STRING "Tag to fetch saltatlas")


if (PSQZ_MAIN_PROJECT)

    # Let's nicely support folders in IDEs
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # Testing only available if this is the main app NOTE: this needs to be done
    # in the main CMakeLists because it calls enable_testing, which must be in
    # the main CMakeLists
    include(CTest)

    # Docs only available if this is the main app
    find_package(Doxygen)
    if (Doxygen_FOUND)
        # add_subdirectory(docs)
        message(STATUS "Doxygen found, skipping docs")
    else ()
        message(STATUS "Doxygen not found, not building docs")
    endif ()

endif ()

file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if (EXISTS "${LOC_PATH}")
    message(
        FATAL_ERROR "Cannot build in a directory with a CMakeLists.txt file. "
                    "Please make a build subdirectory."
    )
endif ()

include(FetchContent)

#
# YGM (internally makes boost available)
#
if (NOT ygm_POPULATED)
    find_package(ygm ${PSQZ_YGM_VERSION} QUIET CONFIG)
    if (NOT ygm_FOUND)
        FetchContent_Declare(
            ygm
            GIT_REPOSITORY ${YGM_FETCH_GIT}
            GIT_TAG ${YGM_FETCH_TAG}
        )
        FetchContent_MakeAvailable(ygm)
    endif ()
endif ()

#
# krowkee
#
if (NOT krowkee_POPULATED)
    find_package(krowkee CONFIG)
    if (NOT krowkee_FOUND)
        FetchContent_Declare(
            krowkee
            GIT_REPOSITORY ${KROWKEE_FETCH_GIT}
            GIT_TAG ${KROWKEE_FETCH_TAG}
        )
        FetchContent_MakeAvailable(krowkee)
    endif ()
endif ()

add_library(psqz INTERFACE)
add_library(psqz::psqz ALIAS psqz)
target_compile_features(psqz INTERFACE cxx_std_20)
target_include_directories(
    psqz INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                   $<INSTALL_INTERFACE:include>
)
target_link_libraries(psqz INTERFACE krowkee::krowkee ygm::ygm Boost::unordered Boost::container)

if (PSQZ_INSTALL_HDKNN)
    #
    # saltatlas
    #
    if (NOT saltatlas_POPULATED)
        find_package(saltatlas CONFIG)
        if (NOT saltatlas_FOUND)
            set(SALTATLAS_USE_METALL OFF)
            set(JUST_INSTALL_SALTATLAS ON)
            FetchContent_Declare(
                saltatlas
                GIT_REPOSITORY ${SALTATLAS_FETCH_GIT}
                GIT_TAG ${SALTATLAS_FETCH_TAG}
            )
            FetchContent_MakeAvailable(saltatlas)
        endif ()
    endif ()

    add_library(hdknn INTERFACE)
    add_library(hdknn::hdknn ALIAS hdknn)
    target_compile_features(hdknn INTERFACE cxx_std_20)
    target_include_directories(
        hdknn
        INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/hdknn/include>
                  $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(hdknn INTERFACE psqz::psqz saltatlas::saltatlas)
endif ()

if (PSQZ_MAIN_PROJECT)
    add_subdirectory(examples)
    enable_testing()
    add_subdirectory(test)
    if (PSQZ_INSTALL_HDKNN)
        add_subdirectory(examples/hdknn)
    endif ()
endif ()
