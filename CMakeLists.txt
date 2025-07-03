cmake_minimum_required(VERSION 3.10)
project(Tuana_Aydin_BilgisayarGrafiklerProje2)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)
add_subdirectory(lib/glfw)

add_executable(Tuana_Aydin_BilgisayarGrafiklerProje2 src/main.cpp)
target_link_libraries(KediMacera glfw)