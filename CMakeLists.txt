cmake_minimum_required(VERSION 3.28)
project(AsciiRenderer)

set(CMAKE_CXX_STANDARD 26)

find_package (Eigen3 3.4 REQUIRED)

include(CPack)
set(CPACK_GENERATOR "DragNDrop")

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} Eigen3::Eigen)


install(TARGETS ${PROJECT_NAME} DESTINATION .)