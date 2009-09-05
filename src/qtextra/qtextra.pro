include(../../exscalibar.pri)

INSTALLS += headers \
            target 
target.path = $$PREFIX/lib 
headers.files += *.h 
headers.path = $$PREFIX/include/qtextra/ 

SOURCES += qtextra.cpp \
           qsocketsession.cpp \
	   qsubappengine.cpp

HEADERS += qcleaner.h \
           qfactory.h \
           qfactoryexporter.h \
           qfactorymanager.h \
           qsubappengine.h \
           qsubapp.h \
           qsocketsession.h \
           qfastwaitcondition.h \
           qcounter.h \
           qtextra.h

newmat:SOURCES += qpca.cpp qkohonennet.cpp
newmat:HEADERS += qpca.h qkohonennet.h

!isEmpty(COMPOSE):system("$$COMPOSE $$SOURCES") {
	DEPLOYMENT += $$SOURCES
	SOURCES = .composed.cpp
	OBJECTS_DIR = $$OBJECTS_DIR/qtextra
}

TEMPLATE = lib
