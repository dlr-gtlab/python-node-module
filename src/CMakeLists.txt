#             ______________      __
#            / ____/_  __/ /___  / /_
#           / / __  / / / / __ `/ __
#          / /_/ / / / / / /_/ / /_/ /
#         ____/ /_/ /_/\__,_/_.___/
#
######################################################################

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2024 German Aerospace Center (DLR)

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(HEADERS
    gtpn_module.h
    nodes/gtpy_pythonnode.h
    gui/gtpn_pythonscriptdialog.h
)

set(SOURCES
    gtpn_module.cpp
    nodes/gtpy_pythonnode.cpp
    gui/gtpn_pythonscriptdialog.cpp
)

set(RESOURCES
    ./resources/pyNodes_resources.qrc
)
add_gtlab_module(PythonNode SOURCES ${HEADERS} ${SOURCES} ${RESOURCES}
    MODULE_ID "PythonNode"
    README_FILE "${PROJECT_SOURCE_DIR}/README.md"
    CHANGELOG_FILE "${PROJECT_SOURCE_DIR}/CHANGELOG.md"
)

target_link_libraries(PythonNode
    PUBLIC GTlab::IntelliGraph
    PRIVATE
    Qt5::Widgets
    Qt5::Gui
    Qt5::Xml
    Qt5::Svg
    GTlab::Logging
    GTlab::Core
    GTlab::Gui
    GTlab::Python
)

add_library(GTlab::PythonNode ALIAS PythonNode)

target_include_directories(PythonNode PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/.>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/data>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/gui>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/nodes>
)


######################################################################

