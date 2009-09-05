include(../../../../exscalibar.pri)
TARGETDEPS += $$DESTDIR/libqtextra.so $$DESTDIR/libgeddei.so $$DESTDIR/libtoolkit.so $$DESTDIR/librgeddei.so
LIBS += -lqtextra -lgeddei -ltoolkit -lrgeddei
INCLUDEPATH += $$SRCDIR/qtextra $$SRCDIR/geddei $$SRCDIR/processors/toolkit $$SRCDIR/remote/rgeddei
TEMPLATE = app 
SOURCES += testrspeed.cpp
