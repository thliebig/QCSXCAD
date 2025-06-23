
# define build type
IF( DEFINED CMAKE_BUILD_TYPE )
  SET( CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Set to either \"Release\" or \"Debug\"" )
ELSE()
  SET( CMAKE_BUILD_TYPE Release CACHE STRING "Set to either \"Release\" or \"Debug\"" )
ENDIF()

# In CMake 4, 3.10 is deprecated and 3.5 has been removed.
# use 3.0...3.10 so all of these versions are acceptable as min. version.
# https://cmake.org/cmake/help/latest/command/cmake_minimum_required.html
cmake_minimum_required(VERSION 3.0...3.10)

PROJECT( QCSXCAD CXX C)

# not supported any more by cmake 4.0?
# https://cmake.org/cmake/help/v3.0/policy/CMP0020.html
# if(POLICY CMP0020)
#   cmake_policy(SET CMP0020 NEW)
# endif()

IF(EXISTS ${PROJECT_SOURCE_DIR}/localConfig.cmake)
   include(${PROJECT_SOURCE_DIR}/localConfig.cmake)
ENDIF()

# default
set(LIB_VERSION_MAJOR 0)
set(LIB_VERSION_MINOR 6)
set(LIB_VERSION_PATCH 3)
set(LIB_VERSION_STRING ${LIB_VERSION_MAJOR}.${LIB_VERSION_MINOR}.${LIB_VERSION_PATCH})

set(VERSION "v${LIB_VERSION_STRING}")

# add git revision
# add git revision
IF(EXISTS ${PROJECT_SOURCE_DIR}/.git )
  FIND_PACKAGE(Git)
  # Get the latest abbreviated commit hash of the working branch
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE GITREV
  )
  set(VERSION ${GITREV})
  string(STRIP ${VERSION} VERSION)
  message(STATUS "Found Git repository, ${PROJECT_NAME} version tag: ${VERSION}")
ENDIF()

ADD_DEFINITIONS(-DGIT_VERSION=\"${VERSION}\")

if (WIN32)
    ADD_DEFINITIONS( -DBUILD_QCSXCAD_LIB )
endif (WIN32)


#
# Set up RPATH for the project
#
option(ENABLE_RPATH "Enable rpath support on Linux and Mac" ON)
if(NOT CMAKE_INSTALL_RPATH)
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}")
endif()
if(APPLE AND NOT CMAKE_INSTALL_NAME_DIR)
  set(CMAKE_INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}")
endif()
if(UNIX AND ENABLE_RPATH)
  set(CMAKE_SKIP_BUILD_RPATH FALSE)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

# Set locations of extra CMake modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${openEMS_SOURCE_DIR}/cmake/Modules/")

# CSXCAD
# $ cmake -DCSXCAD_ROOT_DIR=~/opt/openEMS .
# SET(CSXCAD_ROOT_DIR ~/opt/openEMS)
find_library(CSXCAD_LIBRARIES 
  NAMES CSXCAD
  HINTS ${CSXCAD_ROOT_DIR}/lib${LIB_SUFFIX}
  NO_CMAKE_FIND_ROOT_PATH
)
message(STATUS "CSXCAD_LIBRARIES: ${CSXCAD_LIBRARIES}" )
#TODO test if CSXCADs was found
find_path(CSXCAD_INCLUDE_DIR 
    NAMES ContinuousStructure.h 
    HINTS ${CSXCAD_ROOT_DIR}/include
    PATH_SUFFIXES "CSXCAD" ${CSXCAD_INCLUDE_DIR}
    NO_CMAKE_FIND_ROOT_PATH
)
message(STATUS "CSXCAD_INCLUDE_DIR: ${CSXCAD_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( ${CSXCAD_INCLUDE_DIR} ) 

# Set locations of CMake modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${QCSXCAD_SOURCE_DIR}/cmake/Modules/")

# TinyXML module from https://github.com/ros/cmake_modules
find_package(TinyXML REQUIRED)
ADD_DEFINITIONS(  -DTIXML_USE_STL )
INCLUDE_DIRECTORIES( ${TinyXML_INCLUDE_DIR} )

# vtk 9.1 needs MPI find to fail?????
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  find_package(MPI)
endif()

# vtk
find_package(VTK REQUIRED COMPONENTS vtkIOGeometry NO_MODULE)
if (${VTK_VERSION} VERSION_GREATER "9")
   find_package(VTK REQUIRED COMPONENTS CommonCore RenderingCore GUISupportQt RenderingLOD FiltersParallel IOGeometry IOImage IOPLY IOXML InteractionStyle RenderingAnnotation RenderingFreeType NO_MODULE)
else()
    find_package(VTK REQUIRED)
    include(${VTK_USE_FILE})
endif()

message(STATUS "Found package VTK. Using version " ${VTK_VERSION})
INCLUDE_DIRECTORIES (${VTK_INCLUDE_DIRS})

# Qt 
SET(RESOURCES resources.qrc)
set(CMAKE_AUTOMOC ON)

# Both Qt6 support and the variable VTK_QT_VERSION are introduced in vTK
# 9.1. Below VTK 9, we don't know the Qt version used by VTK, so we make
# a guess and hope for the best. Since Qt6 is not officially supported
# before 9.1, we guess Qt4 or Qt5.
# https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7483
if (NOT DEFINED VTK_QT_VERSION)
    FIND_PACKAGE(Qt4 COMPONENTS QtCore QUIET)
    if (Qt4_FOUND)
        SET(VTK_QT_VERSION "4")
    else()
        SET(VTK_QT_VERSION "5")
    # Probably no need to check for Qt6 here, since Qt6 is only supported by VTK 9.1+
    # which exposes VTK_QT_VERSION
    endif()
    message(
      WARNING
      "VTK_QT_VERSION is unsupported before VTK 9.1, "
      "a guess has been made: Qt ${VTK_QT_VERSION}. "
      "Use -DVTK_QT_VERSION=number to override!"
    )
endif()

if(VTK_QT_VERSION VERSION_GREATER "5")
    FIND_PACKAGE(Qt6 COMPONENTS Core Core5Compat Widgets Xml REQUIRED)
    set(QT_LIBRARIES Qt6::Core5Compat Qt6::Widgets Qt6::Xml)
    QT6_ADD_RESOURCES(RESOURCES_SRCS ${RESOURCES})
elseif(VTK_QT_VERSION VERSION_GREATER "4")
    FIND_PACKAGE(Qt5 COMPONENTS Core Widgets Xml REQUIRED)
    set(QT_LIBRARIES Qt5::Widgets Qt5::Xml)
    QT5_ADD_RESOURCES(RESOURCES_SRCS ${RESOURCES})
else()
    # in Qt4, QT_LIBRARIES is set by CMake
    FIND_PACKAGE(Qt4 REQUIRED QtCore QtGui QtXml)
    INCLUDE( ${QT_USE_FILE} )
    INCLUDE_DIRECTORIES(${QT_INCLUDES})
    QT4_ADD_RESOURCES(RESOURCES_SRCS ${RESOURCES})
endif()

message(STATUS "Found/guessed VTK's Qt version. Using version " ${VTK_QT_VERSION})

if("${VTK_MAJOR_VERSION}" GREATER 5)
    set( vtk_LIBS ${VTK_LIBRARIES} )
else()
    set( vtk_LIBS
        vtkCommon
        vtkFiltering
        vtkGraphics
        vtkHybrid
        vtkIO
        vtkRendering
        vtkWidgets
        QVTK
        )
endif()
message(STATUS "vtk libraries " ${vtk_LIBS})
    
set( SOURCES
  QCSXCAD.cpp
  QCSGridEditor.cpp
  QCSPrimEditor.cpp
  QCSPropEditor.cpp
  QCSTreeWidget.cpp
  QParameterGui.cpp
  QVTKStructure.cpp
  VTKPrimitives.cpp
  export_x3d.cpp
  export_pov.cpp
  QCSXCAD_Global.cpp
  vtkInteractorStyleRubberBand2DPlane.cpp
)

set(PUB_HEADERS
  QCSXCAD.h
  QCSGridEditor.h
  QCSPrimEditor.h
  QCSPropEditor.h
  QCSTreeWidget.h
  QParameterGui.h
  QVTKStructure.h
  VTKPrimitives.h
  QCSXCAD_Global.h
  export_x3d.h
  export_pov.h
  vtkInteractorStyleRubberBand2DPlane.h
)

# CSXCAD library
add_library( QCSXCAD SHARED
  ${SOURCES}
  ${MOC_SRCS}
  ${RESOURCES_SRCS}
)

TARGET_LINK_LIBRARIES( QCSXCAD
  ${CSXCAD_LIBRARIES}
  ${TinyXML_LIBRARY}
  ${vtk_LIBS}
  ${QT_LIBRARIES}
)

set_target_properties(QCSXCAD PROPERTIES VERSION ${LIB_VERSION_STRING} SOVERSION ${LIB_VERSION_MAJOR})

INSTALL(TARGETS QCSXCAD DESTINATION lib${LIB_SUFFIX})

INSTALL(FILES ${PUB_HEADERS} DESTINATION include/QCSXCAD)

#TODO tarball, debug, release
