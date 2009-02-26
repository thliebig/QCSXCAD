TEMPLATE = lib
CONFIG += release
TARGET = QCSXCAD
QT += core \
    gui
HEADERS += QCSXCAD.h \
    QCSGridEditor.h \
    QCSPrimEditor.h \
    QCSPropEditor.h \
    QCSTreeWidget.h \
    QParameterGui.h \
    QVTKStructure.h \
    VTKPrimitives.h \
    QCSXCAD_Global.h
SOURCES += QCSXCAD.cpp \
    QCSGridEditor.cpp \
    QCSPrimEditor.cpp \
    QCSPropEditor.cpp \
    QCSTreeWidget.cpp \
    QParameterGui.cpp \
    QVTKStructure.cpp \
    VTKPrimitives.cpp
VTK_DIR = c:\opt\vtk
INCLUDEPATH += . \
    $$VTK_DIR\
    $$VTK_DIR\Common \
    $$VTK_DIR\Common\Testing\Cxx \
    $$VTK_DIR\Filtering \
    $$VTK_DIR\GUISupport\Qt \
    $$VTK_DIR\GenericFiltering \
    $$VTK_DIR\Graphics \
    $$VTK_DIR\Hybrid \
    $$VTK_DIR\IO \
    $$VTK_DIR\Imaging \
    $$VTK_DIR\Rendering \
    $$VTK_DIR\Utilities \
    $$VTK_DIR\Widgets \
    ..\CSXCAD \
    ..\Gym2XML
VTK_BIN_DIR += c:\opt\VTK\bin
LIBS += $$VTK_BIN_DIR\libQVTK.dll \
    $$VTK_BIN_DIR\libvtkHybrid.dll \
    $$VTK_BIN_DIR\libvtkIO.dll \
    $$VTK_BIN_DIR\libvtkImaging.dll \
    $$VTK_BIN_DIR\libvtkRendering.dll \
    $$VTK_BIN_DIR\libvtkWidgets.dll \
    $$VTK_BIN_DIR\libvtkGraphics.dll \
    $$VTK_BIN_DIR\libvtkFiltering.dll \
    $$VTK_BIN_DIR\libvtkGenericFiltering.dll \
    $$VTK_BIN_DIR\libvtkCommon.dll \
    $$VTK_BIN_DIR\libvtkexoIIc.dll \
    $$VTK_BIN_DIR\libvtkexpat.dll \
    $$VTK_BIN_DIR\libvtkftgl.dll \
    $$VTK_BIN_DIR\libvtksys.dll \
    $$VTK_BIN_DIR\libvtkzlib.dll \
    $$VTK_BIN_DIR\libvtkfreetype.dll \
    ..\CSXCAD\Release\libCSXCAD.dll \
    ..\Gym2XML\Release\libGym2XML.dll
FORMS += 
RESOURCES += resources.qrc
DEFINES += BUILD_QCSXCAD_LIB
