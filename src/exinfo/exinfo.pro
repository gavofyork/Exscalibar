# File generated by kdevelop's qmake manager.
# -------------------------------------------
# Subdir relative project main directory: ./src/nite
# Target is an application:

include( ../../global.pro )

INSTALLS += target
target.path = $$PREFIX/bin
TARGETDEPS += ../../src/geddei/libgeddei.so ../../src/qtextra/libqtextra.so
LIBS +=	-lqtextra \
	-lgeddei
QMAKE_LIBDIR = ../../src/qtextra \
               ../../src/geddei
INCLUDEPATH += ../../src/geddei \
               ../../src/qtextra
TEMPLATE = app
SOURCES += exinfo.cpp
