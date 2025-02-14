#!/bin/bash
DIR=$(dirname "$0")
cd $DIR
clear

BUILD=installer/package-root/Applications/HP/PrimeSDK/bin
IDENTITY=$(security find-identity -v -p codesigning | grep "Developer ID Application" | awk '{print $2}')

make -j$(sysctl -n hw.ncpu) all
codesign -s "$IDENTITY" ./$BUILD/*

read -p "Press Enter to exit!"

# Close the Terminal window
osascript -e 'tell application "Terminal" to close window 1' & exit
