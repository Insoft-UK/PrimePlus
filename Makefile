p+:
	g++ -std=c++20 src/*.cpp -o bin/arm/p+ -Os -fno-ident -fno-asynchronous-unwind-tables -target arm64-apple-macos11
	g++ -std=c++20 src/*.cpp -o bin/p+ -Os -fno-ident -fno-asynchronous-unwind-tables -target x86_64-apple-macos10.12
