include(../../../exscalibar.pri)
TARGETDEPS += $$DESTDIR/libgeddei.so $$DESTDIR/libqtextra.so 
LIBS += -lgeddei -lqtextra
INCLUDEPATH += $$SRCDIR/geddei $$SRCDIR/qtextra
TEMPLATE = app 
SOURCES += incremental.cpp 
