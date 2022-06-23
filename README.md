build command:
conan install . --install-folder cmake-build-release --build=missing && cd cmake-build-release && cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release && cmake --build .
