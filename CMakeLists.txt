
# define build type
IF( DEFINED CMAKE_BUILD_TYPE )
  SET( CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Set to either \"Release\" or \"Debug\"" )
ELSE()
  SET( CMAKE_BUILD_TYPE Release CACHE STRING "Set to either \"Release\" or \"Debug\"" )
ENDIF()

PROJECT( QCSXCAD CXX)

cmake_minimum_required(VERSION 2.8)

IF(EXISTS ${PROJECT_SOURCE_DIR}/localConfig.cmake)
   include(${PROJECT_SOURCE_DIR}/localConfig.cmake)
ENDIF()

# default
set(LIB_VERSION_MAJOR 0)
set(LIB_VERSION_MINOR 6)
set(LIB_VERSION_PATCH 0)
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
  PATH_SUFFIXES "libCSXCAD" ${CSXCAD_ROOT_DIR}/lib${LIB_SUFFIX}
  NO_CMAKE_FIND_ROOT_PATH
)
message(STATUS "CSXCAD_LIBRARIES: ${CSXCAD_LIBRARIES}" )
#TODO test if CSXCADs was found
find_path(CSXCAD_INCLUDE_DIR 
  NAMES ContinuousStructure.h 
  PATH_SUFFIXES "CSXCAD" ${CSXCAD_ROOT_DIR}/include
  NO_CMAKE_FIND_ROOT_PATH
)
message(STATUS "CSXCAD_INCLUDE_DIR: ${CSXCAD_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( ${CSXCAD_INCLUDE_DIR} ) 

# Set locations of CMake modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${QCSXCAD_SOURCE_DIR}/cmake/Modules/")

# TinyXML module from https://github.com/ros/cmake_modules
find_package(TinyXML REQUIRED)
ADD_DEFINITIONS(  -DTIXML_USE_STL )

# vtk
if (WIN32)
    find_package(VTK 6.1 REQUIRED)
else()
    # prefer >=6.1, fallback to >=5.4
    find_package(VTK REQUIRED)
endif()

message(STATUS "Found package VTK. Using version " ${VTK_VERSION})
include(${VTK_USE_FILE})
INCLUDE_DIRECTORIES (${VTK_INCLUDE_DIR})

# Qt 
# require Qt4 if vtk 5.x is used
# require Qt5 if vtk 6.x is used
set(CMAKE_AUTOMOC ON)
#
# generate rules for building source files from bitmap resources
#
SET(RESOURCES resources.qrc)

if("${VTK_MAJOR_VERSION}" EQUAL 5)
    FIND_PACKAGE(Qt4 REQUIRED QtCore QtGui QtXml)
    INCLUDE( ${QT_USE_FILE} )
    QT4_ADD_RESOURCES(RESOURCES_SRCS ${RESOURCES})
elseif("${VTK_MAJOR_VERSION}" EQUAL 6)
    # prefer Qt5 for vtk 6.x        
    find_package(Qt5Widgets)
    if(Qt5Widgets_FOUND)
        include_directories(${Qt5Widgets_INCLUDE_DIRS})
        find_package(Qt5Xml REQUIRED)
        include_directories(${Qt5Xml_INCLUDE_DIRS})
        INCLUDE_DIRECTORIES(${Qt5_INCLUDE_DIRS})
        set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
        QT5_ADD_RESOURCES(RESOURCES_SRCS ${RESOURCES})
    else()
        FIND_PACKAGE(Qt4 REQUIRED QtCore QtGui QtXml)
        INCLUDE( ${QT_USE_FILE} )
        QT4_ADD_RESOURCES(RESOURCES_SRCS ${RESOURCES})
    endif()
endif()

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

if(Qt5Widgets_FOUND)
    qt5_use_modules(QCSXCAD Widgets Xml)
endif()

TARGET_LINK_LIBRARIES( QCSXCAD
  ${CSXCAD_LIBRARIES}
  tinyxml
  ${VTK_LIBRARIES}  # give all found libs fo linker
  ${QT_LIBRARIES}
)

set_target_properties(QCSXCAD PROPERTIES VERSION ${LIB_VERSION_STRING} SOVERSION ${LIB_VERSION_MAJOR})

INSTALL(TARGETS QCSXCAD DESTINATION lib${LIB_SUFFIX})

INSTALL(FILES ${PUB_HEADERS} DESTINATION include/QCSXCAD)

#TODO tarball, debug, release
