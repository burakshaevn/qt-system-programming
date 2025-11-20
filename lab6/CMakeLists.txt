cmake_minimum_required(VERSION 3.16)
project(Assembler VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt6_add_executable(Assembler
    main.cpp
    MainWindow.cpp
    MainWindow.h
    MainWindow.ui
    assembler/Assembler.cpp
    assembler/Assembler.h
    assembler/Command.cpp
    assembler/Command.h
    assembler/CodeLine.cpp
    assembler/CodeLine.h
    assembler/SymbolicName.cpp
    assembler/SymbolicName.h
    assembler/Section.cpp
    assembler/Section.h
    assembler/TNLine.cpp
    assembler/TNLine.h
    assembler/CommandDto.cpp
    assembler/CommandDto.h
    assembler/AssemblerException.cpp
    assembler/AssemblerException.h
    helpers/Parser.cpp
    helpers/Parser.h
    helpers/Comparer.cpp
    helpers/Comparer.h
)

target_include_directories(Assembler
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Assembler
    PRIVATE
    Qt6::Core
    Qt6::Widgets
)

