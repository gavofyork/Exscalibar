# File generated by kdevelop's qmake manager. 
# ------------------------------------------- 
# Subdir relative project main directory: ./src/processors/toolkit
# Target is a library:  

include( ../../../globalplugin.pro )
INSTALLS += headers \
            target 
target.path = $$PREFIX/plugins/geddei 
headers.files += *.h 
headers.path = $$PREFIX/include/geddeiprocessors 
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
           recorder.cpp 
TARGETDEPS += ../../../src/qtextra/libqtextra.so \
../../../src/geddei/libgeddei.so

LIBS += -lgeddei \
-lqtextra

INCLUDEPATH += ../../../src/geddei \
../../../src/qtextra
QMAKE_LIBDIR = ../../../src/geddei \
../../../src/qtextra
TEMPLATE = lib
