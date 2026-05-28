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
    gui/gtpn_pythonnodewidget.h
)

set(SOURCES
    gtpn_module.cpp
    nodes/gtpy_pythonnode.cpp
    gui/gtpn_pythonscriptdialog.cpp
    gui/gtpn_pythonnodewidget.cpp
)

set(RESOURCES
    ./resources/pyNodes_resources.qrc
)
add_gtlab_module(PythonNode SOURCES ${HEADERS} ${SOURCES} ${RESOURCES}
    MODULE_ID "PythonNode"
    README_FILE "${PROJECT_SOURCE_DIR}/README.md"
    CHANGELOG_FILE "${PROJECT_SOURCE_DIR}/CHANGELOG.md"
	EXAMPLES_DIR "${PROJECT_SOURCE_DIR}/examples/"
)

target_link_libraries(PythonNode
    PUBLIC GTlab::IntelliGraph
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Svg
    GTlab::Logging
    GTlab::Core
    GTlab::Gui
    GTlab::Python
)

# include SvgWidgets, if Qt major version is 6 or higher
if (QT_VERSION_MAJOR GREATER_EQUAL 6)
    target_link_libraries(PythonNode
        PRIVATE Qt6::SvgWidgets
    )
endif()

add_library(GTlab::PythonNode ALIAS PythonNode)

target_include_directories(PythonNode PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/.>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/data>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/gui>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/nodes>
)


######################################################################

