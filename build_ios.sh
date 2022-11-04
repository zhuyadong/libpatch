#!/bin/sh
mkdir -p build_ios
pushd build_ios
cmake .. -G Xcode -DCMAKE_TOOLCHAIN_FILE=../cmake/ios.toolchain.cmake -DPLATFORM=OS64
cmake --build . --config Release
popd