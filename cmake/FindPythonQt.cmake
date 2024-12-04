# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2024 German Aerospace Center (DLR)
include(CMakeFindDependencyMacro )
find_dependency(Python3 COMPONENTS Development)

# Look for the header file.
find_path(PythonQt_INCLUDE_DIR NAMES PythonQt.h PATH_SUFFIXES include)

set (PYTHONQT_LIBNAME_SUFFIX ${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR})

# Look for the library (sorted from most current/relevant entry to least).
find_library(PythonQt_LIBRARY NAMES
    PythonQt-Qt5-Python${PYTHONQT_LIBNAME_SUFFIX}
    PATH_SUFFIXES lib
)

if (UNIX)
find_library(PythonQt_LIBRARY_DEBUG NAMES
    PythonQt-Qt5-Python${PYTHONQT_LIBNAME_SUFFIX}
    PATH_SUFFIXES libDebug
)
else()
find_library(PythonQt_LIBRARY_DEBUG NAMES
    PythonQt-Qt5-Python${PYTHONQT_LIBNAME_SUFFIX}_d
    PATH_SUFFIXES libDebug
)
endif()

# handle the QUIETLY and REQUIRED arguments and set GTlabCore_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PythonQt
                                  REQUIRED_VARS PythonQt_LIBRARY PythonQt_INCLUDE_DIR)

if(PythonQt_FOUND)

  add_library(PythonQt::PythonQt UNKNOWN IMPORTED)
  set_target_properties(PythonQt::PythonQt PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${PythonQt_INCLUDE_DIR}
      IMPORTED_LOCATION ${PythonQt_LIBRARY}
      IMPORTED_IMPLIB ${PythonQt_LIBRARY}
	  IMPORTED_LOCATION_DEBUG ${PythonQt_LIBRARY_DEBUG}
      IMPORTED_IMPLIB_DEBUG ${PythonQt_LIBRARY_DEBUG}
  )

endif()
