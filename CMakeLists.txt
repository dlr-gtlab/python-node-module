#             ______________      __
#            / ____/_  __/ /___  / /_
#           / / __  / / / / __ `/ __
#          / /_/ / / / / / /_/ / /_/ /
#         ____/ /_/ /_/\__,_/_.___/
######################################################################

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2024 German Aerospace Center (DLR)

cmake_minimum_required(VERSION 3.15)

project(GTlab-PythonNodeModule VERSION 0.4.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_AUTOMOC ON)

set(Qt5_DIR ${QT_DIR})

find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui Xml Svg)

find_package(Python3 REQUIRED COMPONENTS Development)
set(Python3_VERSION_SUFFIX "${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}")

include(GTlab)

gtlab_standard_setup()
enable_gtlab_devtools()

if(NOT TARGET GTlab::Core)
    find_package(GTlab REQUIRED)
endif()

if(NOT TARGET GTlab::IntelliGraph)
    find_package(GTlabIntelliGraph REQUIRED)
endif()

if(NOT TARGET GTlab::Python)
    find_package(GTlabPython REQUIRED)
endif()

option(BUILD_WITH_COVERAGE "Build with code coverage (linux only)" OFF)
option(BUILD_UNITTESTS "Build Unittests" OFF)

if (BUILD_WITH_COVERAGE)
    set(CODE_COVERAGE_VERBOSE ON)
    include(CodeCoverage)
    append_coverage_compiler_flags()
endif(BUILD_WITH_COVERAGE)

add_subdirectory(src)

if (BUILD_UNITTESTS)
    enable_testing()
    add_subdirectory(tests/unittests)
endif ()

if (BUILD_UNITTESTS AND BUILD_WITH_COVERAGE)
    setup_target_for_coverage_lcov(
            NAME test-coverage
            EXECUTABLE GTlabPythonNodeUnitTests
            EXCLUDE "tests/*" "build*" "/usr/*" "/opt/*" "/home/gitlab-runner/*"
            BASE_DIRECTORY "${PROJECT_SOURCE_DIR}")
endif()

include(CPack)
set(CPACK_PACKAGE_VENDOR "DLR AT")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")

