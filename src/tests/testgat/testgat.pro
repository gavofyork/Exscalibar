# File generated by kdevelop's qmake manager. 
# ------------------------------------------- 
# Subdir relative project main directory: ./src/tests/testgat
# Target is an application:  

include( ../../../global.pro )

TARGETDEPS += ../../../src/geddei/libgeddei.so
LIBS += -lgeddei -lqtextra
INCLUDEPATH += ../../../src/geddei \
               ../../../src/qtextra 
QMAKE_LIBDIR = ../../../src/geddei \
               ../../../src/qtextra 
TEMPLATE = app 
SOURCES += testgat.cpp 
