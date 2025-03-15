@echo off
set START_DIR=%cd%

cd source

echo "Removing all exe builds in ./source"
del *.exe
echo "Removing the linux build in ./source"
del comic-hero

echo "Setting build for Linux - AMD64"
set GOOS=linux
set GOARCH=amd64

echo "Building the program"
go build

echo "Building finished"
cd %START_DIR%
