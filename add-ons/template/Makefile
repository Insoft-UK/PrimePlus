BUILD := build
PROJECT_NAME ?= addon

all:
	mkdir -p $(BUILD)
	g++ -arch x86_64 -arch arm64 -std=c++23 src/*.cpp -o $(BUILD)/$(PROJECT_NAME) -Os -fno-ident -fno-asynchronous-unwind-tables -Wl,-dead_strip -Wl,-x

clean:
	rm -rf $(BUILD)/*

install:
	cp $(BUILD)/$(PROJECT_NAME) /Applications/HP/Xprime.app/Contents/Resources/Developer/usr/bin/$(PROJECT_NAME)
	
uninstall:
	rm /Applications/HP/Xprime.app/Contents/Resources/Developer/usr/bin/$(PROJECT_NAME)

