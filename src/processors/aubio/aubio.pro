# File generated by kdevelop's qmake manager.
# -------------------------------------------
# Subdir relative project main directory: ./src/processors/example
# Target is a library:
PACKAGES = "aubio:0.3.2"
include(../../../exscalibar.pri)
TARGETDEPS += $$DESTDIR/libqtextra.so \
	$$DESTDIR/libgeddei.so
LIBS += -lqtextra \
	-lgeddei
INCLUDEPATH += $$SRCDIR/qtextra \
	$$SRCDIR/geddei
SOURCES += aubio.cpp \
	aubiotracker.cpp
!isEmpty(COMPOSE):system("$$COMPOSE $$SOURCES") {
	DEPLOYMENT += $$SOURCES
	SOURCES = .composed.cpp
	OBJECTS_DIR = $$OBJECTS_DIR/aubio
}
TARGET = aubiogeddei
TEMPLATE = lib
CONFIG += plugin
VERSION = $$OURVERSION
