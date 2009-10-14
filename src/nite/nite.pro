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
DEPENDPATH += ../../src/geddei \
	../../src/qtextra
FORMS += geddeinitebase.ui
HEADERS += GeddeiNite.h \
	ProcessorView.h \
	ProcessorsView.h \
	ProcessorsScene.h \
	ProcessorItem.h \
	SubProcessorItem.h \
	DomProcessorItem.h \
	InputItem.h \
	ConnectionItem.h \
	OutputItem.h \
	IncompleteConnectionItem.h \
	MultipleConnectionItem.h \
	MultiProcessorItem.h \
	BaseItem.h \
	PauseItem.h \
	Magnetic.h
SOURCES += GeddeiNite.cpp \
	main.cpp \
	ProcessorView.cpp \
	ProcessorsView.cpp \
	ProcessorsScene.cpp \
	ProcessorItem.cpp \
	DomProcessorItem.cpp \
	SubProcessorItem.cpp \
	InputItem.cpp \
	ConnectionItem.cpp \
	OutputItem.cpp \
	IncompleteConnectionItem.cpp \
	MultipleConnectionItem.cpp \
	MultiProcessorItem.cpp \
	BaseItem.cpp \
	PauseItem.cpp \
	Magnetic.cpp
!isEmpty(COMPOSE):system("$$COMPOSE $$SOURCES") {
	DEPLOYMENT += $$SOURCES
	SOURCES = .composed.cpp
	OBJECTS_DIR = $$OBJECTS_DIR/nite
}
TEMPLATE = app
RESOURCES += Icons.qrc
