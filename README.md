#### Build command:
>conan install . --install-folder cmake-build-release --build=missing && cd cmake-build-release && cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release && cmake --build .

NOTE: As it's seen in CMakeList.txt file C++ standard is set to C++20 and some features are being in use so to be able to compile the project you need to use at least g++10 or above.

#### Example usage:

signature calculation:
`./filediff --signature --infile A --outfile A.sig`

delta printing:
`./filediff --delta --sigfile A.sig --newdata A`

#### Examples:

##### A)
1) Base file state:
>cat A
```
aaa
bbb
ccc
ddd
eee
```
2) Calculate the signature file for base file:
>./cmake-build-release/filediff --signature --infile A --outfile A.sig
3) Update the file:
>cat A
```
ccc
aaa
bbb
ddd
eee
```
4) Calculate the delta for updated file against signature:
>./cmake-build-release/filediff --delta --sigfile A.sig --newdata A
```
255012a
ccc
255012a

```
###### Explanation:
255012a is hash in hex for the chunk that is found to be either new or removed. If next line after the hex is empty then the chunk was removed, if there is a text then it means that it's new entry. Yet if there are two entries for the same hash one containing text and second empty, then it means that this chunk was moved within the file.

##### B)
1) Base file state:
>cat B
```
A
B
C
```
2) Calculate the signature file for base file:
>./cmake-build-release/filediff --signature --infile B --outfile B.sig
3) Update the file:
>cat B
```
A
Z
X
C
```
4) Calculate the delta for updated file against signature:
>./cmake-build-release/filediff --delta --sigfile B.sig --newdata B
```
430043

5b005b
Z
590059
X
```
###### Explanation:
In this case one entry was replaced with two new entries.

##### C)
1) Base file state:
>cat C
```
A
B
C
```
2) Calculate the signature file for base file:
>./cmake-build-release/filediff --signature --infile C --outfile C.sig
3) Update the file:
>cat C
```
X
C
B
A
```
4) Calculate the delta for updated file against signature:
>./cmake-build-release/filediff --delta --sigfile C.sig --newdata C
```
420042

440044

590059
X
430043
B
420042
A
```
###### Explanation:
This example shows how delta for shufled file with new entry at the beginning will be calculated. At first two moved entries will be detected as removed and then new entry detected, and in the end two moved entries detected as new (moved).
