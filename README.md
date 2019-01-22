# How to build

You'll need to build this on a Raspberry Pi... I have not played with cross-compilation toolchains.

1. Download mbedtls sources. Put the dir in `libs/` and rename it to `mbedtls/`.
2. Build mbedtls. This is just running `make` from within `mbedtls/`.
3. Install mbedtls. This will need to be installed locally, so create a directory `mbedtls/out/` and call `make install DESTDIR=$(pwd)/out` (this assumes you are in `mbedtls/`).
4. Download curl sources. Put the dir in `libs/` and rename it to `curl/`.
5. Configure curl by running `curl/configure --prefix=<full_path_to_libs/curl/out> --without-zlib --disable-shared --without-ssl --with-mbedtls=<full_path_to_libs/mbedtls/out>`.
6. Build curl. Just run `make` inside `curl/`.
7. Install mbedtls. Create directory `curl/out/` and call `make install`.
8. Download jsmn sources. Put in `libs/` and rename to `jsmn/`. Build with `make`.
9. Download tomlc99 sources. Put in `libs/` and rename to `tomlc99/`. Build with `make`.

To build unistatus, you'll need to create the directory `bin/`.

Next, you'll have to install whatever display "driver" you wish to use. For testing of non-graphical elements, build unistatus with `make DISPLAY_DRIVER=dummy`.

To use SDL and OpenGL, SDL must be installed system-wide (such that the `sdl2-config` command is available) and OpenGL must be installed as a shared library in a system-wide location. On Ubuntu, these can be installed with `apt install libsdl2-dev libgl1-mesa-dev`. Then unistatus will need to be compiled with `make DISPLAY_DRIVER=opengl`.

To use the Unicorn HAT, download the ws2811 library from `https://github.com/jgarff/rpi_ws281x`. Put it in `libs/rpi_ws281x` and build it with `scons` (you'll need scons installed, on Ubuntu that's `apt install scons`). Then build unistatus with `make DISPLAY_DRIVER=ws2811`.
