include(../../../exscalibar.pri)
TARGETDEPS += $$DESTDIR/libgeddei.so $$DESTDIR/libqtextra.so 
LIBS += -lgeddei -lqtextra
INCLUDEPATH += $$SRCDIR/geddei $$SRCDIR/qtextra

INSTALLS += headers \
            target
target.path = $$PREFIX/lib
headers.files += *.h
headers.path = $$PREFIX/include/rgeddei
TEMPLATE = lib
HEADERS += abstractprocessor.h \
           abstractprocessorgroup.h \
           hostprocessorforwarder.h \
           localdomprocessor.h \
           localprocessor.h \
           localsession.h \
           remotedomprocessor.h \
           remoteprocessor.h \
           remotesession.h \
           rgeddei.h \
           sessionserver.h \
           abstractdomprocessor.h \
           abstractprocessorport.h \
           commcodes.h
SOURCES += abstractprocessor.cpp \
           abstractprocessorgroup.cpp \
           hostprocessorforwarder.cpp \
           localdomprocessor.cpp \
           localprocessor.cpp \
           localsession.cpp \
           remotedomprocessor.cpp \
           remoteprocessor.cpp \
           remotesession.cpp \
           sessionserver.cpp \
           abstractdomprocessor.cpp \
           abstractprocessorport.cpp

