# File generated by kdevelop's qmake manager.
# -------------------------------------------
# Subdir relative project main directory: ./src/processors/toolkit
# Target is a library:
PACKAGES = "sndfile:1.0.0" \
	"vorbisfile:1.0.0" \
	"mad:0.15" \
	"jack:0.90.0" \
	"alsa:0.9"
include(../../../exscalibar.pri)
INSTALLS += headers \
	target
target.path = $$PREFIX/plugins/geddei
headers.files += *.h
headers.path = $$PREFIX/include/geddeiprocessors
TARGETDEPS += $$DESTDIR/libqtextra.so \
	$$DESTDIR/libgeddei.so
LIBS += -lqtextra \
	-lgeddei
INCLUDEPATH += $$SRCDIR/qtextra \
	$$SRCDIR/geddei
HEADERS += monitor.h \
	multiplayer.h \
	player.h \
	recorder.h
SOURCES += monitor.cpp \
	multiplayer.cpp \
	player.cpp \
	toolkit.cpp \
	alsacapturer.cpp \
	jackcapturer.cpp \
	dumper.cpp \
	fan.cpp \
	stress.cpp \
	recorder.cpp \
	alsaplayer.cpp \
    slur.cpp
!isEmpty(COMPOSE):system("$$COMPOSE $$SOURCES") {
	DEPLOYMENT += $$SOURCES
	SOURCES = .composed.cpp
	OBJECTS_DIR = $$OBJECTS_DIR/toolkit
}
TEMPLATE = lib
CONFIG += plugin
VERSION = $$OURVERSION
