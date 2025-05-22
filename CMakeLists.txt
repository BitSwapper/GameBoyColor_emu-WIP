cmake_minimum_required(VERSION 3.15)

project(GBC LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor)

set(GLAD_SOURCES
    ${VENDOR_DIR}/glad/src/glad.c
)

set(IMGUI_SOURCES
    ${VENDOR_DIR}/imgui/imgui.cpp
    ${VENDOR_DIR}/imgui/imgui_draw.cpp
    ${VENDOR_DIR}/imgui/imgui_tables.cpp
    ${VENDOR_DIR}/imgui/imgui_widgets.cpp
    ${VENDOR_DIR}/imgui/backends/imgui_impl_sdl2.cpp
    ${VENDOR_DIR}/imgui/backends/imgui_impl_opengl3.cpp
)

set(EMULATOR_SOURCES
    main.cpp
    src/Bus.cpp
    src/Cartridge.cpp          
    src/Cpu.cpp
    src/Emulator.cpp
    src/EmulatorUI.cpp
    src/TestSuite.cpp
    src/Opcodes.cpp
    src/InvalidInstruction.cpp
    ${IMGUI_SOURCES}
    ${GLAD_SOURCES}
)

add_executable(gbc_emu ${EMULATOR_SOURCES})

# --- Include Directories ---
target_include_directories(gbc_emu PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${VENDOR_DIR}/SDL2/include
    ${VENDOR_DIR}/imgui
    ${VENDOR_DIR}/imgui/backends
    ${VENDOR_DIR}/glad/include
)

# --- Link Libraries ---
find_package(OpenGL REQUIRED)

if(MSVC)
    target_link_libraries(gbc_emu PRIVATE
        ${VENDOR_DIR}/SDL2/lib/x64/SDL2.lib
        ${VENDOR_DIR}/SDL2/lib/x64/SDL2main.lib
        OpenGL::GL
    )
else()
    find_package(SDL2 REQUIRED)
    if(SDL2_FOUND AND OpenGL_FOUND)
        target_link_libraries(gbc_emu PRIVATE SDL2::SDL2 SDL2::SDL2main OpenGL::GL)
    else()
        message(FATAL_ERROR "SDL2 or OpenGL not found for non-MSVC build.")
    endif()
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(MSVC)
    add_custom_command(TARGET gbc_emu POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${VENDOR_DIR}/SDL2/lib/x64/SDL2.dll"
        $<TARGET_FILE_DIR:gbc_emu>
        COMMENT "Copying SDL2.dll to output directory"
    )
endif()