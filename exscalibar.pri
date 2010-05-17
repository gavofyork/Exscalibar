PREFIX = $$[EXSCALIBAR_PREFIX]
isEmpty(PREFIX): PREFIX = $$[QT_INSTALL_PREFIX]

CONFIG += debug \
warn_on \
qt \
thread \
exceptions

COMPOSE = $$PWD/compose.sh

QT *= network

BUILD_ROOT = $$PWD
OURVERSION = $$system("grep EXSCALIBAR_VERSION \"$$PWD/system/exscalibar.h\" | sed \"s:.define EXSCALIBAR_VERSION ::g\"")

OBJECTS_DIR = $$PWD/.objects
MOC_DIR = $$PWD/.objects
UI_DIR = $$PWD/.objects
RCC_DIR = $$PWD/.objects
DESTDIR = $$PWD/build
INCLUDEPATH += $$PWD/system
SRCDIR = $$PWD/src
QMAKE_LIBDIR += $$DESTDIR
DEFINES += __GEDDEI_BUILD

QMAKE_CXXFLAGS_RELEASE += -DRELEASE \
-g0 \
-ffast-math \
-fomit-frame-pointer \
-pipe \
-fexceptions \
-Os \
-I.

QMAKE_CXXFLAGS_DEBUG += -DDEBUG \
-g3 \
-ffast-math \
-fno-inline \
-pipe \
-O0 \
-Wall \
-fexceptions \
-fvisibility-inlines-hidden \
-fvisibility=hidden \
-I.

include("$$PWD/packageconfig.prf")

