# File generated by kdevelop's qmake manager. 
# ------------------------------------------- 
# Subdir relative project main directory: ./src/processors/math
# Target is a library:  

include(../../../exscalibar.pri)

INSTALLS += target 
target.path = $$PREFIX/plugins/geddei 
TARGETDEPS += $$DESTDIR/libqtextra.so \
              $$DESTDIR/libgeddei.so 
LIBS += -lgeddei \
        -lqtextra 
INCLUDEPATH += ../../../src/geddei \
               ../../../src/qtextra 
TEMPLATE = lib 
CONFIG += plugin
SOURCES += exp.cpp \
           mean.cpp \
           normalise.cpp \
           magnitude.cpp \
           sum.cpp \
           log.cpp \
           math.cpp 
