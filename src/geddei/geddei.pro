include(../../exscalibar.pri)
INSTALLS += headers \
    target
target.path = $$PREFIX/lib
headers.files += [a-z]*.h
headers.path = $$PREFIX/include/geddei/
TARGETDEPS += "$$DESTDIR/libqtextra.so"
LIBS += -lqtextra
INCLUDEPATH += ../../src/qtextra
HEADERS += buffer.h \
    bufferinfo.h \
    bufferdata.h \
    bufferdatas.h \
    bufferreader.h \
    combination.h \
    commandcodes.h \
    connection.h \
    domprocessor.h \
    drcoupling.h \
    dscoupling.h \
    dxcoupling.h \
    geddei.h \
    llconnection.h \
    lmconnection.h \
    lrconnection.h \
    lxconnection.h \
    lxconnectionnull.h \
    lxconnectionreal.h \
    matrix.h \
    mlconnection.h \
    multiplicative.h \
    multiprocessor.h \
    multiprocessorcreator.h \
    multisink.h \
    multisource.h \
    processor.h \
    processorfactory.h \
    processorforwarder.h \
    processorgroup.h \
    properties.h \
    rlconnection.h \
    rscoupling.h \
    signaltype.h \
    signaltyperefs.h \
    sink.h \
    source.h \
    spectrum.h \
    splitter.h \
    subprocessor.h \
    subprocessorfactory.h \
    wave.h \
    xlconnection.h \
    xlconnectionreal.h \
    xscoupling.h \
    xxcoupling.h \
    globals.h \
    signaltypes.h \
    processorport.h \
    signaltyperef.h \
    value.h
SOURCES += buffer.cpp \
    bufferinfo.cpp \
    bufferdata.cpp \
    bufferdatas.cpp \
    bufferreader.cpp \
    combination.cpp \
    connection.cpp \
    domprocessor.cpp \
    drcoupling.cpp \
    dscoupling.cpp \
    dxcoupling.cpp \
    llconnection.cpp \
    lmconnection.cpp \
    lrconnection.cpp \
    lxconnection.cpp \
    lxconnectionnull.cpp \
    lxconnectionreal.cpp \
    matrix.cpp \
    mlconnection.cpp \
    multiplicative.cpp \
    multiprocessor.cpp \
    multiprocessorcreator.cpp \
    multisink.cpp \
    multisource.cpp \
    processor.cpp \
    processorfactory.cpp \
    processorforwarder.cpp \
    processorgroup.cpp \
    properties.cpp \
    rlconnection.cpp \
    rscoupling.cpp \
    signaltype.cpp \
    signaltyperefs.cpp \
    sink.cpp \
    source.cpp \
    spectrum.cpp \
    splitter.cpp \
    subprocessor.cpp \
    subprocessorfactory.cpp \
    wave.cpp \
    xlconnection.cpp \
    xlconnectionreal.cpp \
    xscoupling.cpp \
    xxcoupling.cpp \
    processorport.cpp \
    signaltyperef.cpp \
    globals.cpp
!isEmpty(COMPOSE):system("$$COMPOSE $$SOURCES") { 
    DEPLOYMENT += $$SOURCES
    SOURCES = .composed.cpp
    OBJECTS_DIR = $$OBJECTS_DIR/geddei
}
TEMPLATE = lib
VERSION = $$OURVERSION
