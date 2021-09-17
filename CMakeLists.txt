cmake_minimum_required(VERSION 3.19)
project(sokoban C)

set(CMAKE_C_STANDARD 99)

SET(SRC main.c sokoban.c)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

find_library(MENU_LIBRARY menu)

add_executable(${PROJECT_NAME} ${SRC})

target_link_libraries(${PROJECT_NAME} ${CURSES_LIBRARIES} ${MENU_LIBRARY})