project(usbLUKSkey)
cmake_minimum_required(VERSION 2.6)

# link statically
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
# use C++11 (stream constructors accept strings)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory("src")
