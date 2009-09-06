# File generated by kdevelop's qmake manager.
# -------------------------------------------
# Subdir relative project main directory: ./src/nite
# Target is an application:
PACKAGES = "sndfile:1.0.0" \
	"alsa:0.9"
include(../../exscalibar.pri)
QT += xml
INSTALLS += target
target.path = $$PREFIX/bin
TARGETDEPS += $$DESTDIR/libqtextra.so \
	$$DESTDIR/libgeddei.so
LIBS += -lqtextra \
	-lgeddei
INCLUDEPATH += ../../src/geddei \
	../../src/qtextra
FORMS += geddeinitebase.ui
HEADERS += geddeinite.h \
	watchprocessor.h \
	refresher.h \
	processorview.h \
	processorsview.h \
	processoritem.h \
	inputitem.h \
	connectionitem.h \
	outputitem.h \
	incompleteconnectionitem.h
SOURCES += geddeinite.cpp \
	main.cpp \
	watchprocessor.cpp \
	refresher.cpp \
	processorview.cpp \
	processorsview.cpp \
	processoritem.cpp \
	inputitem.cpp \
	connectionitem.cpp \
	outputitem.cpp \
	incompleteconnectionitem.cpp
!isEmpty(COMPOSE):system("$$COMPOSE $$SOURCES") {
	DEPLOYMENT += $$SOURCES
	SOURCES = .composed.cpp
	OBJECTS_DIR = $$OBJECTS_DIR/nite
}
TEMPLATE = app
