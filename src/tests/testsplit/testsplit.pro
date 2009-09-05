include(../../../exscalibar.pri)
TARGETDEPS += $$DESTDIR/libgeddei.so $$DESTDIR/libqtextra.so $$DESTDIR/libtoolkit.so
LIBS += -lgeddei -lqtextra -ltoolkit
INCLUDEPATH += $$SRCDIR/geddei $$SRCDIR/qtextra $$SRCDIR/processors/toolkit
TEMPLATE = app 
SOURCES += testsplit.cpp 
