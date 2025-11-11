cmake_minimum_required(VERSION 3.10)

# Project name - change this when renaming the project
project(minPath)

# Specify the C++ standard - adjust if needed for newer standards
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directories - customize these paths if needed
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Source files - add new source files here
set(SOURCES
    main.cpp
    Bellman-Ford/BellmanFord.cpp    # Uncomment when adding these files
    Dijkstra/dijkstra_algorithm.cpp
    dataGenerator/dataGenerator.cpp
    aStar/aStar.cpp
)

# Include directories - add new include directories here
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Bellman-Ford
    ${CMAKE_SOURCE_DIR}/Dijkstra
    ${CMAKE_SOURCE_DIR}/dataGenerator
    ${CMAKE_SOURCE_DIR}/aStar
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Add compiler options - customize based on your needs
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>
)

# Optional: Add external dependencies here using find_package()
# Example:
# find_package(Boost REQUIRED)
# target_link_libraries(${PROJECT_NAME} PRIVATE Boost::boost)

# Optional: Define compilation flags based on build type
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")