NAME := p+
INSTALL := /usr/local/bin
BUILD := build

all:
	g++ -arch x86_64 -arch arm64 -std=c++20 src/*.cpp -o $(BUILD)/$(NAME) -Os -fno-ident -fno-asynchronous-unwind-tables
	
install:
	cp $(BUILD)/$(NAME) $(INSTALL)/$(NAME)
	
clean:
	rm -rf $(BUILD)/*
