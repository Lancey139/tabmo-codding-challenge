#!/bin/sh

mkdir install
mkdir -p build/default
cd build/default

cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=OFF -DBUILD_CTL=OFF -DBUILD_ORM=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ../../
cmake --build . -- -v

