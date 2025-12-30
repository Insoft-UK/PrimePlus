PROJECT_NAME ?= project

all: arm64 x86_64 universal

arm64:
	mkdir -p build
	clang++ -arch arm64 -std=c++23 \
	-Isrc/librfmt/include src/librfmt/lib/arm64/librfmt.a \
	-Isrc/libmin/include src/libmin/lib/arm64/libmin.a \
	-Isrc/libhpprgm/include src/libhpprgm/lib/arm64/libhpprgm.a \
	src/*.cpp \
	-o build/$(PROJECT_NAME) -Os -fno-ident -fno-asynchronous-unwind-tables -Wl,-dead_strip -Wl,-x

x86_64:
	mkdir -p build
	clang++ -arch x86_64 -std=c++23 \
	-Isrc/librfmt/include src/librfmt/lib/x86_64/librfmt.a \
	-Isrc/libmin/include src/libmin/lib/x86_64/libmin.a \
	-Isrc/libhpprgm/include src/libhpprgm/lib/x86_64/libhpprgm.a \
	src/*.cpp \
	-o build/$(PROJECT_NAME) -Os -fno-ident -fno-asynchronous-unwind-tables -Wl,-dead_strip -Wl,-x

universal:
	# Combine into a universal binary
	lipo -create -output build/$(PROJECT_NAME) build/arm64/$(PROJECT_NAME) build/x86_64/$(PROJECT_NAME)
	
clean:
	rm -rf build/*
	
install:
	cp build/$(PROJECT_NAME) /usr/local/bin/$(PROJECT_NAME)
	
uninstall:
	rm /usr/local/bin/$(PROJECT_NAME)
