include(../../exscalibar.pri)
INSTALLS += headers \
	target
target.path = $$PREFIX/lib
headers.files += *.h
headers.path = $$PREFIX/include/qtextra/
LIBS += -lnewmat
SOURCES += qtextra.cpp \
	qpca.cpp \
	qsocketsession.cpp \
	qsubappengine.cpp \
	qtask.cpp \
	qscheduler.cpp \
	qworker.cpp \
	qkohonennet.cpp \
    rdtsc.cpp
HEADERS += qcleaner.h \
	qfactory.h \
	qfactoryexporter.h \
	qfactorymanager.h \
	qsubappengine.h \
	qsubapp.h \
	qsocketsession.h \
	qfastwaitcondition.h \
	qcounter.h \
	qtextra.h \
	qtask.h \
	qscheduler.h \
	qworker.h \
	qpca.h \
	qkohonennet.h \
    rdtsc.h
!isEmpty(COMPOSE):system("$$COMPOSE $$SOURCES") {
	DEPLOYMENT += $$SOURCES
	SOURCES = .composed.cpp
	OBJECTS_DIR = $$OBJECTS_DIR/qtextra
}
TEMPLATE = lib
VERSION = $$OURVERSION
