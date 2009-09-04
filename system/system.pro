# system .pro file
include( ../global.pro )

TEMPLATE = lib

INSTALLS += pc headers
headers.path = $$PREFIX/include
headers.files = Geddei SignalTypes rGeddei QtExtra *.h
pc.path = /usr/lib/pkgconfig
pc.files = *.pc

