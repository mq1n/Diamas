mkdir build

pushd build

cmake .. -A Win32 -DCMAKE_BUILD_TYPE=%1

cmake --build . --config %1 --target ALL_BUILD -- /m /property:GenerateFullPaths=true

popd

echo "build done!"