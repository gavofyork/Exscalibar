include(../../../../exscalibar.pri)
TARGETDEPS += $$DESTDIR/libqtextra.so $$DESTDIR/libgeddei.so $$DESTDIR/librgeddei.so
LIBS += -lqtextra -lgeddei -lrgeddei
INCLUDEPATH += $$SRCDIR/qtextra $$SRCDIR/geddei $$SRCDIR/remote/rgeddei
TEMPLATE = app 
SOURCES += testnodeclient.cpp
