all:
	g++ -arch x86_64 -arch arm64 -std=c++20 src/*.cpp -o build/p+ -Os -fno-ident -fno-asynchronous-unwind-tables

clean:
    rm -rf build/*
