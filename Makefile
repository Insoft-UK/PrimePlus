NAME := p+

all:
	mkdir -p build
	g++ -arch x86_64 -arch arm64 -std=c++20 src/*.cpp -o build/$(NAME) -Os -fno-ident -fno-asynchronous-unwind-tables

install:
	cp build/$(NAME) /usr/local/bin/$(NAME)
	
uninstall:
	rm /usr/local/bin/$(NAME)
	
clean:
	rm -rf build/*
