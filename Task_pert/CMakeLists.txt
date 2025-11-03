cmake_minimum_required(VERSION 3.15)

project(BOiOD LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRC_FILES
    main.cpp
    CPM/CPMCalculator.cpp
    CPM/Task.cpp
    PERT/PERTCalculator.cpp
    PERT/PertProblemGenerator.cpp
    PERT/Task_pert.cpp
    Result_printer/ResultPrinter.cpp
    Data_Loader/DataLoader.cpp
    Data_Loader/DataLoader_pert.cpp
)

add_executable(cpm_test ${SRC_FILES})

target_include_directories(cpm_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/CPM
        ${CMAKE_CURRENT_SOURCE_DIR}/PERT
        ${CMAKE_CURRENT_SOURCE_DIR}/Data_Loader
        ${CMAKE_CURRENT_SOURCE_DIR}/Result_printer
)

target_compile_options(cpm_test
    PRIVATE
        -Wall
        -Wextra
        -O3
        -march=native
        -flto
)
