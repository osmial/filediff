build command:
conan install . --install-folder cmake-build-release --build=missing && cd cmake-build-release && cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release && cmake --build .

example usage:

signature calculation:
./filediff --signature --infile A --outfile A.sig

delta printing:
./filediff --delta --sigfile A.sig --newdata A

NOTE: during the testing it was found that recent changes have broken delta calculation when it comes to detecting shifted chunks, there are appropriate unit tests (disabled) that reproduces the issue.
