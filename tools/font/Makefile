BUILD := build
PROJECT_NAME ?= project

all:
	mkdir -p $(BUILD)
	g++ -arch x86_64 -arch arm64 -std=c++23 src/*.cpp -o $(BUILD)/$(PROJECT_NAME) -Os -fno-ident -fno-asynchronous-unwind-tables -Wl,-dead_strip -Wl,-x
	
install:
	cp $(BUILD)/$(PROJECT_NAME) /usr/local/bin/$(PROJECT_NAME)
	
clean:
	rm /usr/local/bin/$(PROJECT_NAME)
