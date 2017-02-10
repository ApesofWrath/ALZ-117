cmake_minimum_required(VERSION 2.8)

project(vision)

set(CMAKE_CXX_FLAGS "-std=c++0x")

find_package(OpenCV REQUIRED)

include_directories(include)

add_library(libntcore STATIC IMPORTED)
add_library(libwpiutil STATIC IMPORTED)


set_property(TARGET libntcore PROPERTY IMPORTED_LOCATION /home/ubuntu/ALZ-117/Linux/arm/libntcore.a)
set_property(TARGET libwpiutil PROPERTY IMPORTED_LOCATION /home/ubuntu/ALZ-117/Linux/arm/libwpiutil.a)

add_executable(cv_vision vision.cpp)

target_link_libraries(cv_vision ${OpenCV_LIBS})
target_link_libraries(cv_vision libntcore)
target_link_libraries(cv_vision libwpiutil)

