cmake_minimum_required(VERSION 3.16)

project(VirtualMachine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES
    main.cpp
    Assembler.cpp
    Lexer.cpp
    Parser.cpp
    Virtual_Machine.cpp
)

set(HEADERS
    Assembler.h
    Lexer.h
    OpCode.h
    Parser.h
    Token.h
    Virtual_Machine.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/code.txt ${CMAKE_CURRENT_BINARY_DIR}/code.txt COPYONLY)