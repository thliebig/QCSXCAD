TEMPLATE = lib
TARGET = QCSXCAD

# add git revision
QMAKE_CXXFLAGS += -DGIT_VERSION=\\\"`git \
    describe \
    --tags`\\\"
MOC_DIR = moc
OBJECTS_DIR = obj
QT += core \
    gui \
    xml
HEADERS += QCSXCAD.h \
    QCSGridEditor.h \
    QCSPrimEditor.h \
    QCSPropEditor.h \
    QCSTreeWidget.h \
    QParameterGui.h \
    QVTKStructure.h \
    VTKPrimitives.h \
    QCSXCAD_Global.h \
    export_x3d.h \
    export_pov.h \
    QGeometryPlot.h
SOURCES += QCSXCAD.cpp \
    QCSGridEditor.cpp \
    QCSPrimEditor.cpp \
    QCSPropEditor.cpp \
    QCSTreeWidget.cpp \
    QParameterGui.cpp \
    QVTKStructure.cpp \
    VTKPrimitives.cpp \
    export_x3d.cpp \
    export_pov.cpp \
    QGeometryPlot.cpp
win32 { 
    DEFINES += BUILD_QCSXCAD_LIB
    
    # DEFINES += __GYM2XML__
    include(localPathes.pri)
    INCLUDEPATH += . \
        $$VTK_BIN_DIR\.. \
        $$VTK_DIR \
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
        ..\CSXCAD
    
    # ..\Gym2XML
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
        ..\CSXCAD\release\CSXCAD.dll
}

# ..\Gym2XML\release\Gym2XML.dll
unix { 
    VERSION = 0.2.0
    INCLUDEPATH += ../CSXCAD \
        ../tinyxml
    LIBS += -L../CSXCAD \
        -lCSXCAD
    INCLUDEPATH += /usr/include/vtk-5.2 \
		/usr/include/vtk-5.4 \
		/usr/include/vtk-5.6
	LIBS += -lvtkCommon \
        -lvtkDICOMParser \
        -lvtkFiltering \
        -lvtkGenericFiltering \
        -lvtkGraphics \
        -lvtkHybrid \
        -lvtkIO \
        -lvtkImaging \
        -lvtkParallel \
        -lvtkRendering \
        -lvtkVolumeRendering \
        -lvtkWidgets \
        -lvtkexoIIc \
        -lvtkftgl \
        -lvtksys \
        -lQVTK
}
FORMS += 
RESOURCES += resources.qrc
DEFINES += BUILD_QCSXCAD_LIB
QMAKE_CXXFLAGS_DEBUG = -O0 \
    -g

# to use ABI2 target:
# qmake CONFIG+="ABI2 bits64" -o Makefile.ABI2-64 QCSXCAD.pro
# make -fMakefile.ABI2-64
ABI2 { 
    CONFIG -= debug \
        debug_and_release
    CONFIG += release
    QMAKE_CFLAGS_RELEASE = -O2 \
        -fabi-version=2
    QMAKE_CXXFLAGS_RELEASE = -O2 \
        -fabi-version=2
    QMAKE_CC = apgcc
    QMAKE_CXX = apg++
    QMAKE_LINK = apg++
    QMAKE_LINK_SHLIB = apg++
    QMAKE_LFLAGS_RPATH = 
    QMAKE_LFLAGS = \'-Wl,-rpath,\$$ORIGIN/lib\'
}
bits64 { 
    QMAKE_CXXFLAGS_RELEASE += -m64 \
        -march=athlon64
    QMAKE_LFLAGS_RELEASE += -m64 \
        -march=athlon64
    OBJECTS_DIR = ABI2-64
    LIBS = ../CSXCAD/ABI2-64/libCSXCAD.so
}
bits32 { 
    QMAKE_CXXFLAGS_RELEASE += -m32 \
        -march=pentium3
    QMAKE_LFLAGS_RELEASE += -m32 \
        -march=pentium3
    OBJECTS_DIR = ABI2-32
    LIBS = ../CSXCAD/ABI2-32/libCSXCAD.so
}
ABI2 { 
    DESTDIR = $$OBJECTS_DIR
    MOC_DIR = $$OBJECTS_DIR
    UI_DIR = $$OBJECTS_DIR
    RCC_DIR = $$OBJECTS_DIR
}
