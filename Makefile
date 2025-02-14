NAME := p+
PRIMESDK := Applications/HP/PrimeSDK
BUILD := installer/package-root/$PRIMESDK/bin
IDENTITY := $(security find-identity -v -p codesigning | grep "Developer ID Application" | awk '{print $2}')

all:
	g++ -arch x86_64 -arch arm64 -std=c++20 src/*.cpp -o $BUILD/$(NAME) -Os -fno-ident -fno-asynchronous-unwind-tables
	codesign -s "$IDENTITY" ./$BUILD/*
	
install:
	cp $BUILD/$(NAME) /$PRIMESDK/bin/$(NAME)
	
clean:
	rm -rf $BUILD/*
