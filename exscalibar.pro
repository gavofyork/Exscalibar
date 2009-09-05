TEMPLATE = subdirs
SUBDIRS = src system

include("exscalibar.pri")
system("cat system/exscalibar.pc.tmpl > system/exscalibar.pc"):
system("echo Version: $${OURVERSION} >> system/exscalibar.pc"):
system("echo Libs: -L$${PREFIX}/lib -lqtextra -lgeddei -lrgeddei >> system/exscalibar.pc"):
system("echo Cflags: -fexceptions -I$${PREFIX}/include >> system/exscalibar.pc"):

