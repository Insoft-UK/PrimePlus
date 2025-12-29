BUILD := build
PROJECT_NAME ?= project

all:
	mkdir -p $(BUILD)
	clang++ -arch x86_64 -arch arm64 -std=c++23 \
	-Isrc/librfmt/include src/librfmt/lib/librfmt.a \
	-Isrc/libmin/include src/libmin/lib/libmin.a \
	-Isrc/libhpprgm/include src/libhpprgm/lib/libhpprgm.a \
	src/*.cpp \
	-o $(BUILD)/$(PROJECT_NAME) -Os -fno-ident -fno-asynchronous-unwind-tables -Wl,-dead_strip -Wl,-x

clean:
	rm -rf $(BUILD)/*
	
install:
	cp $(BUILD)/$(PROJECT_NAME) /usr/local/bin/$(PROJECT_NAME)
	
uninstall:
	rm /usr/local/bin/$(PROJECT_NAME)
