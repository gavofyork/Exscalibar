# File generated by kdevelop's qmake manager. 
# ------------------------------------------- 
# Subdir relative project main directory: ./src/tests/testmulti
# Target is an application:  

include( ../../../global.pro )

TARGETDEPS += ../../../src/geddei/libgeddei.so \
              ../../../src/qtextra/libqtextra.so
LIBS += -lgeddei \
        -lqtextra
INCLUDEPATH = ../../../src/processors/toolkit \
              ../../../src/geddei \
              ../../../src/qtextra 
QMAKE_LIBDIR = ../../../src/geddei \
               ../../../src/qtextra \
	       ../../../src/processors/toolkit
TEMPLATE = app 
SOURCES += testflow.cpp 
