cmake_minimum_required(VERSION 2.8)

add_definitions(-std=c++11)

project(ledctrl)

find_package(Threads)

add_executable(${PROJECT_NAME}
    main.cpp
    LedControl.h
    Server.cpp
    Server.h
    )

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
