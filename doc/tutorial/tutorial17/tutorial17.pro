# Filename: tutorial17.pro
# Copyright (c)2005 Gav Wood.
#
# This file is public domain - you may do with it what you want.
#
# To be translated into a Makefile with:
# qmake

TEMPLATE = app
SOURCES += tutorial17.cpp
LIBS += -lrgeddei
INCLUDEPATH += ${EXSCALIBAR_LOCATION}/include
QMAKE_LIBDIR += ${EXSCALIBAR_LOCATION}/lib
CONFIG += qt thread exceptions debug warn_on
