# File generated by kdevelop's qmake manager.
# -------------------------------------------
# Subdir relative project main directory: ./src/processors/example
# Target is a library:
include(../../../exscalibar.pri)
TARGETDEPS += $$DESTDIR/libqtextra.so \
    $$DESTDIR/libgeddei.so
LIBS += -lqtextra \
    -lgeddei
INCLUDEPATH += $$SRCDIR/qtextra \
    $$SRCDIR/geddei
SOURCES += oscilloscope.cpp \
    viewers.cpp \
    spectroscope.cpp \
    Matrigraph.cpp \
    grapher.cpp
!isEmpty(COMPOSE):system("$$COMPOSE $$SOURCES") { 
    DEPLOYMENT += $$SOURCES
    SOURCES = .composed.cpp
    OBJECTS_DIR = $$OBJECTS_DIR/viewers
}
TEMPLATE = lib
CONFIG += plugin
VERSION = $$OURVERSION
