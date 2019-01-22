SOURCES = src/main.cpp src/json.cpp src/colors.cpp
HEADERS = src/json.hpp src/display.hpp src/colors.hpp
CFLAGS = -std=c++14 -pthread -Wall -g -Ilib/curl/out/include -Ilib/jsmn -Ilib/tomlc99
LFLAGS = -Llib/curl/out/lib -l:libcurl.a -Llib/jsmn -l:libjsmn.a -Llib/tomlc99 -l:libtoml.a -Llib/mbedtls/out/lib -l:libmbedtls.a -l:libmbedx509.a -l:libmbedcrypto.a
OUT = bin/unistatus

DISPLAY_DRIVER = dummy

ifeq ($(DISPLAY_DRIVER), opengl)
	CFLAGS += `sdl2-config --cflags`
	LFLAGS += `sdl2-config --libs` -lGL
	SOURCES += src/display_opengl.cpp
else ifeq ($(DISPLAY_DRIVER), dummy)
	SOURCES += src/display_dummy.cpp
else ifeq ($(DISPLAY_DRIVER), ws2811)
	CFLAGS += -Ilib/rpi_ws281x
	LFLAGS += -Llib/rpi_ws281x -lws2811
	SOURCES += src/display_ws2811.cpp
else
	# Do whatever to connect to the actual Unicorn HAT
endif

$(OUT): $(SOURCES) $(HEADERS)
	g++ -o $@ $(CFLAGS) $(SOURCES) $(LFLAGS)
