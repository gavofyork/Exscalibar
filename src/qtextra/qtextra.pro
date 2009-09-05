include(../../exscalibar.pri)

INSTALLS += headers \
            target 
target.path = $$PREFIX/lib 
headers.files += *.h 
headers.path = $$PREFIX/include/qtextra/ 

HEADERS += qcleaner.h \
           qfactory.h \
           qfactoryexporter.h \
           qfactorymanager.h \
           qsubapp.h \
           qsocketsession.h \
           qfastwaitcondition.h \
           qcounter.h \
           qtextra.h
SOURCES += qsubapp.cpp \
           qtextra.cpp \
           qsocketsession.cpp

newmat:SOURCES += qpca.cpp qkohonennet.cpp
newmat:HEADERS += qpca.h qkohonennet.h

TEMPLATE = lib
