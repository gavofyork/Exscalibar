# Filename: tutorial13.pro
# Copyright (c)2005 Gav Wood.
#
# This file is public domain - you may do with it what you want.
#
# To be translated into a Makefile with:
# qmake

TEMPLATE = lib
SOURCES += tutorial13.cpp
LIBS += -lgeddei
INCLUDEPATH += ${EXSCALIBAR_LOCATION}/include
QMAKE_LIBDIR += ${EXSCALIBAR_LOCATION}/lib
CONFIG += qt thread exceptions debug warn_on
VERSION = 1.0.0
INSTALLS += target
target.path = ${EXSCALIBAR_LOCATION}/plugins/geddei
