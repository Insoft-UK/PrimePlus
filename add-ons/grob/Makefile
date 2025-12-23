BUILD := build
PROJECT_NAME ?= project
SRC := src
ARCH := $(shell arch)

all: arm64 x86_64 universal
 
arm64:
	mkdir -p build/arm64
	g++ -arch arm64 -std=c++20 -I$(SRC)/libpng/include -I$(SRC)/libz/include $(SRC)/*.cpp $(SRC)/libpng/lib/arm64/libpng.a $(SRC)/libz/lib/arm64/libz.a -o build/arm64/$(PROJECT_NAME) -Os -fno-ident -fno-asynchronous-unwind-tables -Wl,-dead_strip -Wl,-x
	
x86_64:
	mkdir -p build/x86_64
	g++ -arch x86_64 -std=c++20 -I$(SRC)/libpng/include -I$(SRC)/libz/include $(SRC)/*.cpp $(SRC)/libpng/lib/x86_64/libpng.a $(SRC)/libz/lib/x86_64/libz.a -o build/x86_64/$(PROJECT_NAME) -Os -fno-ident -fno-asynchronous-unwind-tables -Wl,-dead_strip -Wl,-x
	
universal:
	# Combine into a universal binary
	lipo -create -output $(BUILD)/$(PROJECT_NAME) $(BUILD)/arm64/$(PROJECT_NAME) $(BUILD)/x86_64/$(PROJECT_NAME)

clean:
	rm -rf $(BUILD)/*

install:
	cp $(BUILD)/$(ARCH)/$(PROJECT_NAME) /usr/local/bin/$(PROJECT_NAME)
	
uninstall:
	rm /usr/local/bin/$(PROJECT_NAME)

