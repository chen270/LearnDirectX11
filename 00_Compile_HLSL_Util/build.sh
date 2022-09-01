#!/bin/bash -e
rm -rf out
mkdir -p build
cd build

cmake -G "Visual Studio 16 2019" -A x64 ../
cmake --build . --config Release
cd ..
rm -rf build