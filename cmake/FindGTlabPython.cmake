# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2024 German Aerospace Center (DLR)

# - Find GTlabPython
# Find the native GTlabPython headers and libraries.
#
#  GTlabPython_FOUND          - True if GTlabPython found.
#
# Creates the CMake target GTlab::Python

# Look for the header file.

include(CMakeFindDependencyMacro )
 
find_dependency(Python3 REQUIRED COMPONENTS Development)
find_dependency(PythonQt)

find_path(GTlabPython_INCLUDE_DIR NAMES gt_python.h PATH_SUFFIXES python${Python3_VERSION_SUFFIX})

# Look for the library (sorted from most current/relevant entry to least).
find_library(GTlabPython_LIBRARY NAMES
    GTlabPython${Python3_VERSION_SUFFIX}
    PATH_SUFFIXES python${Python3_VERSION_SUFFIX}
)

find_library(GTlabPython_LIBRARY_DEBUG NAMES
    GTlabPython${Python3_VERSION_SUFFIX}-d
    PATH_SUFFIXES python${Python3_VERSION_SUFFIX}
)

# handle the QUIETLY and REQUIRED arguments and set GTlabCore_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GTlabPython
                                  REQUIRED_VARS GTlabPython_LIBRARY  GTlabPython_INCLUDE_DIR)

if(GTlabPython_FOUND)
  add_library(GTlab::Python UNKNOWN IMPORTED)
  set_target_properties(GTlab::Python PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${GTlabPython_INCLUDE_DIR}
      IMPORTED_LOCATION ${GTlabPython_LIBRARY}
      IMPORTED_IMPLIB ${GTlabPython_LIBRARY}
      IMPORTED_LOCATION_DEBUG ${GTlabPython_LIBRARY_DEBUG}
      IMPORTED_IMPLIB_DEBUG ${GTlabPython_LIBRARY_DEBUG}
  )

  set_property(TARGET GTlab::Python APPEND PROPERTY IMPORTED_LINK_INTERFACE_LIBRARIES
  GTlab::Core
  PythonQt::PythonQt
  Python3::Python
  )
endif()
