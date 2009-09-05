CONFIG += debug \
warn_on \
qt \
thread \
exceptions
QT *= qt3support

BUILD_ROOT = $$PWD
OURVERSION = $$system("grep EXSCALIBAR_VERSION \"$$PWD/system/exscalibar.h\" | sed \"s:.define EXSCALIBAR_VERSION ::g\"")

OBJECTS_DIR = $$PWD/.objects
MOC_DIR = $$PWD/.objects
UI_DIR = $$PWD/.objects
RCC_DIR = $$PWD/.objects
DESTDIR = $$PWD/build
INCLUDEPATH += $$PWD/system
QMAKE_LIBDIR += $$DESTDIR

# Set in the .pro files.
#PACKAGES = "newmat:0.10" "sndfile:1.0.0" "vorbisfile:1.0.0" "mad:0.15" "fftw3f:3.0.0" "jack:0.90.0" "alsa:0.9"

QMAKE_CXXFLAGS_RELEASE += -DRELEASE \
-g0 \
-ffast-math \
-fomit-frame-pointer \
-pipe \
-fexceptions \
-Os

QMAKE_CXXFLAGS_DEBUG += -DDEBUG \
-g3 \
-ffast-math \
-fno-inline \
-pipe \
-O0 \
-Wall \
-fexceptions \
-fvisibility-inlines-hidden \
-fvisibility=hidden

include("$$PWD/packageconfig.prf")

