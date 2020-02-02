cmake_minimum_required(VERSION 3.10)
project(feistel)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -maes -std=c++2a -O3 -g")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -maes -std=c++2a  -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mavx2 -maes -msha -mbmi2 -std=c++1z -O3 -ggdb -fno-omit-frame-pointer")


# performance testing
add_executable(shootout
    shootout.cpp
    LazyFisherYates.h
    AesFunc.h
    ShaFeistel.h
    Fnv1aCiphers.h
    simdfeistel.h
    donothing.cpp)

# statistical testing
add_executable(binaryrng
    binaryrng.cpp)

# statistical testing with big crush
# http://www.pcg-random.org/posts/how-to-test-with-testu01.html
add_executable(runbigcrush
    PlaygroundFeistel.h
    GenericFeistel.h
    bigcrush.cpp)
target_link_libraries(runbigcrush
    PRIVATE
   testu01
   testu01probdist
   testu01mylib)

#murmur crypt
add_executable(mrurmurcrypt murmurmain.cpp murmurcrypt.cpp murmurcrypt.h)
