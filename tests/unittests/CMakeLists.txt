# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2024 German Aerospace Center (DLR)

set(CMAKE_AUTORCC TRUE)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if (NOT TARGET gtest)
    include(AddGoogleTest)
endif()

add_executable(GTlabPythonNodeUnitTests
    main.cpp
)

set_property(TARGET GTlabPythonNodeUnitTests PROPERTY AUTOMOC ON)
target_link_libraries(GTlabPythonNodeUnitTests PRIVATE GTlab::PythonNode gtest)

include(GoogleTest)
gtest_discover_tests(GTlabPythonNodeUnitTests TEST_PREFIX "PythonNode." DISCOVERY_MODE PRE_TEST)
