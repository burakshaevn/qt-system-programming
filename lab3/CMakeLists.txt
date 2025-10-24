cmake_minimum_required(VERSION 3.16)
project(TwoPassAssembler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Enable Qt MOC and UIC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
    src/ui/mainwindow.cpp
    src/assembler/assembler.cpp
    src/parser/parser.cpp
    src/structures/command.cpp
    src/structures/operand.cpp
    src/structures/symbolicname.cpp
    src/structures/codeline.cpp
    src/structures/section.cpp
    src/structures/tnline.cpp
    src/exceptions/assemblerexception.cpp
)

# Header files
set(HEADERS
    include/ui/mainwindow.h
    include/assembler/assembler.h
    include/parser/parser.h
    include/structures/command.h
    include/structures/operand.h
    include/structures/symbolicname.h
    include/structures/codeline.h
    include/structures/section.h
    include/structures/tnline.h
    include/exceptions/assemblerexception.h
)

# UI files
set(UI_FILES
    src/ui/mainwindow.ui
)

# Create executable
add_executable(QtAssembler ${SOURCES} ${HEADERS} ${UI_FILES})

# Link Qt libraries
target_link_libraries(QtAssembler Qt6::Core Qt6::Widgets)
