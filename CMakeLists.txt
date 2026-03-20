cmake_minimum_required(VERSION 3.28...4.0)
project(Bookworm VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ─── Tell CMake 4.x to be lenient with older third-party CMakeLists ──────────
set(CMAKE_POLICY_VERSION_MINIMUM "3.5" CACHE STRING "" FORCE)

# ─── Download SFML 3.0.0 automatically ───────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG        3.0.0
    GIT_SHALLOW    TRUE
)
# We only need Graphics, Window, and System
set(SFML_BUILD_AUDIO   ON CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SFML)

# ─── Source files ─────────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/Tile.cpp
    src/Grid.cpp
    src/Dictionary.cpp
    src/Player.cpp
    src/GameEngine.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

# ─── Include directories ──────────────────────────────────────────────────────
target_include_directories(${PROJECT_NAME} PRIVATE include)

# ─── Link SFML ────────────────────────────────────────────────────────────────
target_link_libraries(${PROJECT_NAME} PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
)

# ─── Copy assets folder next to the executable ────────────────────────────────
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMENT "Copying assets to build directory..."
)