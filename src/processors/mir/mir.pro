# File generated by kdevelop's qmake manager.
# -------------------------------------------
# Subdir relative project main directory: ./src/processors/mir
# Target is a library:

PACKAGES = "fftw3f:3.0.0"
include(../../../exscalibar.pri)

INSTALLS += target
target.path = $$PREFIX/plugins/geddei

TARGETDEPS += $$DESTDIR/libqtextra.so $$DESTDIR/libgeddei.so
LIBS += -lqtextra -lgeddei
INCLUDEPATH += $$SRCDIR/qtextra $$SRCDIR/geddei

SOURCES += bandwise.cpp \
		   bark.cpp \
		   cepstrum.cpp \
		   checkerboard.cpp \
		   crosssimilarity.cpp \
		   cutoff.cpp \
		   diagonalsum.cpp \
		   downsample.cpp \
		   fft.cpp \
		   mfcc.cpp \
		   sone.cpp \
		   mir.cpp \
		   rectify.cpp \
		   selfsimilarity.cpp \
		   similarity.cpp \
		   simplesplit.cpp \
		   wavegen.cpp \
		   weightedsum.cpp \
		   pulser.cpp

!isEmpty(COMPOSE):system("$$COMPOSE $$SOURCES") {
	DEPLOYMENT += $$SOURCES
	SOURCES = .composed.cpp
	OBJECTS_DIR = $$OBJECTS_DIR/mir
}

TEMPLATE = lib
CONFIG += plugin
VERSION = $$OURVERSION
