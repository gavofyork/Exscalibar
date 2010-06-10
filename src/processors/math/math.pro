# File generated by kdevelop's qmake manager.
# -------------------------------------------
# Subdir relative project main directory: ./src/processors/math
# Target is a library:

include(../../../exscalibar.pri)

INSTALLS += target
target.path = $$PREFIX/plugins/geddei

TARGETDEPS += $$DESTDIR/libqtextra.so $$DESTDIR/libgeddei.so
LIBS += -lqtextra -lgeddei
INCLUDEPATH += $$SRCDIR/qtextra $$SRCDIR/geddei

SOURCES += exp.cpp \
		   mean.cpp \
		   normalise.cpp \
		   magnitude.cpp \
		   sum.cpp \
		   log.cpp \
		   rotate.cpp \
		   math.cpp \
    Gain.cpp

!isEmpty(COMPOSE):system("$$COMPOSE $$SOURCES") {
	DEPLOYMENT += $$SOURCES
	SOURCES = .composed.cpp
	OBJECTS_DIR = $$OBJECTS_DIR/math
}

TEMPLATE = lib
CONFIG += plugin
VERSION = $$OURVERSION
