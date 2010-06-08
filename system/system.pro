include(../exscalibar.pri)
system("cat exscalibar.pc.tmpl > exscalibar.pc"):
system("echo Version: $${OURVERSION} >> exscalibar.pc"):
system("echo Libs: -L$${PREFIX}/lib -lqtextra -lgeddei -lrgeddei >> exscalibar.pc"):
system("echo Cflags: -fexceptions -I$${PREFIX}/include >> exscalibar.pc"):
TEMPLATE = subdirs
INSTALLS += pc headers
headers.path = $$PREFIX/include
headers.files = Geddei CoreTypes rGeddei QtExtra *.h
pc.path = /usr/lib/pkgconfig
pc.files = *.pc

