cmake_minimum_required(VERSION 3.5.1)
project(embedding)

find_package(Threads REQUIRED)
find_package(BLAS REQUIRED)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

INCLUDE_DIRECTORIES(
  /home/anabur/Github/libraries/arrayfire/include
  /home/anabur/Github/include
)

ADD_LIBRARY( 
  progress_bar
  ../RDF_parser/progress_bar.cpp
)

ADD_EXECUTABLE(embedding embedding.cpp)
set_target_properties(progress_bar embedding PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
TARGET_LINK_LIBRARIES( embedding LINK_PUBLIC progress_bar Threads::Threads blas)
TARGET_LINK_DIRECTORIES(embedding PRIVATE /home/anabur/Github/libraries/arrayfire/lib64 PRIVATE /home/anabur/Github/libraries/cuda/cuda-10.0/lib64)