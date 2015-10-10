TEMPLATE = lib
TARGET = QCSXCAD

VERSION = 0.6.0

# add git revision
GITREV = $$system(git describe --tags)
isEmpty(GITREV):GITREV=$$VERSION
DEFINES += GIT_VERSION=\\\"$$GITREV\\\"

# vtk includes deprecated header files; silence the corresponding warning
QMAKE_CXXFLAGS += -Wno-deprecated

#cgal needs
QMAKE_CXXFLAGS += -frounding-math

# remove unnecessary webkit define
DEFINES -= QT_WEBKIT

exists(localPaths.pri) {
    include(localPaths.pri)
}

MOC_DIR = moc
OBJECTS_DIR = obj
QT += core \
    gui \
    xml
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

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
    vtkInteractorStyleRubberBand2DPlane.h
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
    QCSXCAD_Global.cpp \
    vtkInteractorStyleRubberBand2DPlane.cpp

win32 { 
    DEFINES += BUILD_QCSXCAD_LIB

    isEmpty(WIN32_LIB_ROOT) {
        WIN32_LIB_ROOT = ..
    }

    # CSXCAD
    isEmpty(CSXCAD_ROOT) {
     CSXCAD_ROOT = $$WIN32_LIB_ROOT/CSXCAD
    }
    INCLUDEPATH += $$CSXCAD_ROOT/include/CSXCAD
    LIBS += -L$$CSXCAD_ROOT/lib -lCSXCAD0

    # #3rd party libraries#
    # tinyxml
    INCLUDEPATH += $$WIN32_LIB_ROOT/tinyxml/include
    LIBS += -L$$WIN32_LIB_ROOT/tinyxml/bin -ltinyxml2

    # vtk
    INCLUDEPATH +=   $$WIN32_LIB_ROOT/vtk/include/vtk-5.10
    LIBS += -L$$WIN32_LIB_ROOT/vtk/bin -lvtkCommon -lQVTK -lvtkRendering -lvtkGraphics -lvtkFiltering -lvtkIO -lvtkHybrid -lvtkWidgets

    #boost, needed by cgal
    INCLUDEPATH += $$WIN32_LIB_ROOT/boost/include
    LIBS += -L$$WIN32_LIB_ROOT/boost/lib -lboost_thread -lboost_system -lboost_date_time -lboost_serialization
    #cgal
    INCLUDEPATH += $$WIN32_LIB_ROOT/cgal/include
    LIBS += -L$$WIN32_LIB_ROOT/cgal/bin -lCGAL
}

unix:!macx { 
    # CSXCAD
    isEmpty(CSXCAD_ROOT) {
     CSXCAD_ROOT = /usr
    }
    INCLUDEPATH += $$CSXCAD_ROOT/include/CSXCAD
    LIBS += -L$$CSXCAD_ROOT/lib -lCSXCAD

    # #3rd party libraries#
    # tinyxml
    DEFINES += TIXML_USE_STL

    # vtk
    isEmpty(VTK_INCLUDEPATH) {
        INCLUDEPATH += /usr/include/vtk-5.2 \
        /usr/include/vtk-5.4 \
        /usr/include/vtk-5.6 \
        /usr/include/vtk-5.8 \
        /usr/include/vtk-5.10 \
        /usr/include/vtk-6.0 \
        /usr/include/vtk-6.1 \
        /usr/include/vtk-6.2 \
        /usr/include/vtk-6.3 \
        /usr/include/vtk
    } else {
        INCLUDEPATH += $$VTK_INCLUDEPATH
    }
    isEmpty(VTK_LIBRARYPATH){
    } else {
        LIBS +=-L$$VTK_LIBRARYPATH
    }
    !contains(VTK_6, 1) {
        LIBS += -lvtkCommon \
        -lvtkFiltering \
        -lvtkGraphics \
        -lvtkHybrid \
        -lvtkIO \
        -lvtkRendering \
        -lvtkWidgets \
        -lQVTK
    } else {
        LIBS += -lvtkCommonCore$$VTK_6_LIBSUFFIX \
        -lvtkCommonDataModel$$VTK_6_LIBSUFFIX \
        -lvtkIOLegacy$$VTK_6_LIBSUFFIX \
        -lvtkIOXML$$VTK_6_LIBSUFFIX \
        -lvtkIOGeometry$$VTK_6_LIBSUFFIX \
        -lvtkIOPLY$$VTK_6_LIBSUFFIX \
        -lvtkIOImage$$VTK_6_LIBSUFFIX \
        -lvtksys$$VTK_6_LIBSUFFIX \
        -lvtkIOCore$$VTK_6_LIBSUFFIX \
        -lvtkInteractionStyle$$VTK_6_LIBSUFFIX \
        -lvtkInteractionWidgets$$VTK_6_LIBSUFFIX \
        -lvtkFiltersModeling$$VTK_6_LIBSUFFIX \
        -lvtkGUISupportQt$$VTK_6_LIBSUFFIX \
        -lvtkRenderingCore$$VTK_6_LIBSUFFIX \
        -lvtkRenderingVolumeOpenGL$$VTK_6_LIBSUFFIX \
        -lvtkRenderingOpenGL$$VTK_6_LIBSUFFIX \
        -lvtkRenderingFreeTypeOpenGL$$VTK_6_LIBSUFFIX \
        -lvtkRenderingFreeType$$VTK_6_LIBSUFFIX \
        -lvtkRenderingAnnotation$$VTK_6_LIBSUFFIX \
        -lvtkRenderingLOD$$VTK_6_LIBSUFFIX
    }
}

macx { 
    # CSXCAD
    isEmpty(CSXCAD_ROOT) {
     CSXCAD_ROOT = /usr
    }
    INCLUDEPATH += $$CSXCAD_ROOT/include/CSXCAD
    LIBS += -L$$CSXCAD_ROOT/lib -lCSXCAD

    # #3rd party libraries#
    # tinyxml
    DEFINES += TIXML_USE_STL

    # vtk
    isEmpty(VTK_INCLUDEPATH) {
        INCLUDEPATH += \
        /usr/local/opt/vtk5/include  \
        /usr/local/opt/vtk5/include/vtk-5.10

    } else {
        INCLUDEPATH += $$VTK_INCLUDEPATH
    }

    LIBS += -L/usr/local/opt/vtk5/lib/vtk-5.10  -lvtkCommon \
        -lvtkFiltering \
        -lvtkGraphics \
        -lvtkHybrid \
        -lvtkIO \
        -lvtkRendering \
        -lvtkWidgets \
        -lQVTK

    LIBS += -ltinyxml

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
isEmpty(PREFIX) {
    PREFIX = /usr/local
}
install.target = install
install.commands = mkdir -p \"$$PREFIX/lib$$LIB_SUFFIX\"
install.commands += && mkdir -p \"$$PREFIX/include/QCSXCAD\"
install.commands += && cp $$HEADERS \"$$PREFIX/include/QCSXCAD/\"
unix:!macx:install.commands += && cp -at \"$$PREFIX/lib$$LIB_SUFFIX/\" libQCSXCAD.so*
win32:install.commands += && cp -at \"$$PREFIX/lib$$LIB_SUFFIX/\" release/QCSXCAD0.dll
macx:install.commands += && cp libQCSXCAD*.dylib \"$$PREFIX/lib$$LIB_SUFFIX/\"

QMAKE_EXTRA_TARGETS += install


#
# create .PHONY target
#
phony.target = .PHONY
phony.depends = $$QMAKE_EXTRA_TARGETS
QMAKE_EXTRA_TARGETS += phony
