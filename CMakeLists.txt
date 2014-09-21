
PROJECT( QCSXCAD CXX)

cmake_minimum_required(VERSION 2.8)

# default
set(VERSION "v0.5.2")

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

# TODO handle Qt5
FIND_PACKAGE( Qt4 4.6.3 REQUIRED QtCore QtGui QtXml)
INCLUDE( ${QT_USE_FILE} )

# Set locations of CMake modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${QCSXCAD_SOURCE_DIR}/cmake/modules/")

# TinyXML module from https://github.com/ros/cmake_modules
find_package(TinyXML REQUIRED)
ADD_DEFINITIONS(  -DTIXML_USE_STL )


find_package(VTK REQUIRED)

if("${VTK_MAJOR_VERSION}" EQUAL 5)
  set( vtk_LIBS
    QVTK
  )
endif()


## issue with vtk 6.1,
## * the vtkStructuredGridGeometryFilter only for vtk 5
## using the whole set of libs does work... 
if("${VTK_MAJOR_VERSION}" EQUAL 6)
  set( vtk_LIBS
        vtkCommonCore
        vtkCommonDataModel
        vtkIOLegacy
        vtkIOXML
        vtkIOGeometry
        vtkIOPLY
        vtkIOImage
        vtksys
        vtkIOCore
        vtkInteractionStyle
        vtkInteractionWidgets
        vtkFiltersModeling
        vtkGUISupportQt
        vtkRenderingCore
        vtkRenderingVolumeOpenGL
        vtkRenderingOpenGL
        vtkRenderingFreeTypeOpenGL
        vtkRenderingFreeType
        vtkRenderingAnnotation
        vtkRenderingLOD
        vtkRenderingMatplotlib
        vtkRenderingFreeTypeFontConfig
        vtkStructuredGridGeometry
  )
endif()

if(VTK_FOUND)
	message(STATUS "Found package VTK. Using " ${VTK_USE_FILE})
    include(${VTK_USE_FILE})
else(VTK_FOUND)
    message(FATAL_ERROR "VTK not found. Please set VTK_DIR.")
endif(VTK_FOUND)

INCLUDE_DIRECTORIES (${VTK_INCLUDE_DIR})

#message(status "VTK selected: ${vtk_LIBS}")
#message(status "VTK all libs: ${VTK_LIBRARIES}")

INCLUDE_DIRECTORIES (${VTK_INCLUDE_DIR})


INCLUDE_DIRECTORIES( ${CSXCAD_SOURCE_DIR}/src ) #find ContinuousStructure.h


ADD_DEFINITIONS( -DBUILD_QCSXCAD_LIB )

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

#
# files that have Q_OBJECT need to be MOC'ed
# TODO use newer automoc with Qt5
#
SET(MOC_HDRS
  QCSGridEditor.h
  QCSPropEditor.h
  QCSPrimEditor.h
  QCSTreeWidget.h
  QCSXCAD.h
  QParameterGui.h
  QVTKStructure.h
)


set(PUB_HEADERS
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

#
# headers that need to be moc'ed
#
QT4_WRAP_CPP( MOC_SRCS ${MOC_HDRS} )

#
# generate rules for building source files from bitmap resources
#
SET(RESOURCES resources.qrc)
QT4_ADD_RESOURCES(RESOURCES_SRCS ${RESOURCES})


# CSXCAD library
add_library( QCSXCAD SHARED
  ${SOURCES}
  ${MOC_SRCS}
  ${RESOURCES_SRCS}
)

TARGET_LINK_LIBRARIES( QCSXCAD
  CSXCAD
  tinyxml
  #${vtk_LIBS}      # issue with subset and vtk 6
  ${VTK_LIBRARIES}  # give all found libs fo linker
  ${QT_LIBRARIES}
)

INSTALL(TARGETS QCSXCAD DESTINATION lib)

INSTALL(FILES ${PUB_HEADERS} DESTINATION include/QCSXCAD)

#TODO tarball, debug, release
