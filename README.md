build command:
conan install . --install-folder cmake-build-release --build=missing && cd cmake-build-release && cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release && cmake --build .

example usage:

signature calculation:
./filediff --signature --infile A --outfile A.sig

delta printing:
./filediff --delta --sigfile A.sig --newdata A
