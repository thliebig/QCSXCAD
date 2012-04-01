TEMPLATE = lib
TARGET = QCSXCAD
VERSION = 0.1.3

# add git revision
GITREV = $$system(git describe --tags)
DEFINES += GIT_VERSION=\\\"$$GITREV\\\"

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
    VTK_DIR = ..\vtk
    VTK_BIN_DIR = ..\vtk\bin
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
        ..\CSXCAD \
	..\tinyxml
    
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
        ..\CSXCAD\release\CSXCAD.dll \
	..\tinyxml\release\tinyxml2.dll
}

# ..\Gym2XML\release\Gym2XML.dll
unix { 
    INCLUDEPATH += ../CSXCAD \
        ../tinyxml
    LIBS += -L../CSXCAD \
        -lCSXCAD
    INCLUDEPATH += /usr/include/vtk-5.2 \
		/usr/include/vtk-5.4 \
		/usr/include/vtk-5.6 \
		/usr/include/vtk-5.8 \
		/usr/include/vtk
    INCLUDEPATH += /usr/include/CSXCAD
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
QMAKE_CXXFLAGS_DEBUG = -O0 -g




#
# create tar file
#
tarball.target = tarball
tarball.commands = git archive --format=tar --prefix=QCSXCAD-$$VERSION/ HEAD | bzip2 > QCSXCAD-$${VERSION}.tar.bz2

QMAKE_EXTRA_TARGETS += tarball


#
# INSTALL
#
install.target = install
install.commands = mkdir -p \"$(INSTALL_ROOT)/usr/lib$$LIB_SUFFIX\"
install.commands += && mkdir -p \"$(INSTALL_ROOT)/usr/include/QCSXCAD\"
install.commands += && cp -at \"$(INSTALL_ROOT)/usr/include/QCSXCAD/\" $$HEADERS
install.commands += && cp -at \"$(INSTALL_ROOT)/usr/lib$$LIB_SUFFIX/\" libQCSXCAD.so*

QMAKE_EXTRA_TARGETS += install


#
# create .PHONY target
#
phony.target = .PHONY
phony.depends = $$QMAKE_EXTRA_TARGETS
QMAKE_EXTRA_TARGETS += phony
