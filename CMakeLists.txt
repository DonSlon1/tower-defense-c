cmake_minimum_required(VERSION 3.20)
project(projekt C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_net REQUIRED)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/sources/**/*.c")
set(PROJECT_INCLUDE
    "${CMAKE_CURRENT_LIST_DIR}/sources/core"
    "${CMAKE_CURRENT_LIST_DIR}/sources/objects"
    "${CMAKE_CURRENT_LIST_DIR}/sources/ui"
    "${CMAKE_CURRENT_LIST_DIR}/sources/network"
    "${CMAKE_CURRENT_LIST_DIR}/sources/utils"
)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2_image::SDL2_image SDL2_ttf::SDL2_ttf SDL2_net::SDL2_net m)

target_compile_definitions(${PROJECT_NAME} PRIVATE ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/")

target_compile_options(${PROJECT_NAME} PRIVATE
    # Enable all warnings and treat them as errors
    -Wall
    -Wextra
    -Wpedantic
    -Werror

    # Code quality warnings (common to GCC and Clang)
    -Wshadow                      # Variable shadowing
    -Wuninitialized              # Uninitialized variables
    -Wunused-label               # Unused labels
    -Wconversion                 # Implicit type conversions
    -Wdouble-promotion          # float -> double promotion
    -Wformat=2                   # Format string security
    -Wnull-dereference          # NULL pointer dereference
    -Wvla                        # Variable length arrays (avoid)
    -Wcast-align                 # Alignment issues
    -Wcast-qual                  # const/volatile casting
    -Wwrite-strings             # String literal constness
    -Wmissing-prototypes        # Missing function prototypes
    -Wstrict-prototypes         # Old-style function declarations
    -Wfloat-equal               # Floating point equality comparison
    -Wundef                      # Undefined macros in #if
    -Wpointer-arith             # Pointer arithmetic
    -Wredundant-decls           # Redundant declarations
    -Wmissing-declarations      # Missing declarations
    -Wmissing-field-initializers # Missing struct field initializers
    -Wswitch-default            # Switch without default case
    -Wswitch-enum               # Switch on enum missing cases

    # GCC-specific warnings
    $<$<C_COMPILER_ID:GNU>:-Wlogical-op>
    $<$<C_COMPILER_ID:GNU>:-Wduplicated-cond>
    $<$<C_COMPILER_ID:GNU>:-Wduplicated-branches>
    $<$<C_COMPILER_ID:GNU>:-Wrestrict>
    $<$<C_COMPILER_ID:GNU>:-Wjump-misses-init>
    $<$<C_COMPILER_ID:GNU>:-Wstrict-overflow=5>
    $<$<C_COMPILER_ID:GNU>:-Warray-bounds=2>
    $<$<C_COMPILER_ID:GNU>:-Wformat-overflow=2>
    $<$<C_COMPILER_ID:GNU>:-Wstringop-overflow=4>
    $<$<C_COMPILER_ID:GNU>:-Wtrampolines>
    $<$<C_COMPILER_ID:GNU>:-Winline>
    $<$<C_COMPILER_ID:GNU>:-Wdisabled-optimization>
    $<$<C_COMPILER_ID:GNU>:-Wstack-protector>

    # Clang-specific warnings
    $<$<C_COMPILER_ID:Clang>:-Weverything>
    $<$<C_COMPILER_ID:Clang>:-Wno-padded>
    $<$<C_COMPILER_ID:Clang>:-Wno-declaration-after-statement>
    $<$<C_COMPILER_ID:Clang>:-Wno-unsafe-buffer-usage>
    $<$<C_COMPILER_ID:Clang>:-Wno-disabled-macro-expansion>
    $<$<C_COMPILER_ID:Clang>:-Wno-c++98-compat>
    $<$<C_COMPILER_ID:Clang>:-Wno-pre-c23-compat>
    $<$<C_COMPILER_ID:Clang>:-Wno-pre-c11-compat>
    $<$<C_COMPILER_ID:Clang>:-Wno-covered-switch-default>
    $<$<C_COMPILER_ID:Clang>:-Wno-unreachable-code-break>
    $<$<C_COMPILER_ID:Clang>:-Wno-unreachable-code>

    # Security hardening flags
    -fstack-protector-strong    # Stack canary protection
    $<$<C_COMPILER_ID:GNU>:-fstack-clash-protection>
    $<$<C_COMPILER_ID:GNU>:-fcf-protection=full>
    -fPIE                        # Position independent executable
    $<$<CONFIG:Release>:-D_FORTIFY_SOURCE=2>  # Buffer overflow detection (Release only, needs optimization)

    # Disable specific warnings we handle differently
    -Wno-unused-variable        # Allow unused variables
    -Wno-sign-conversion        # Too noisy for practical use
    $<$<C_COMPILER_ID:GNU>:-Wno-old-style-declaration>  # Allow 'const auto' instead of 'auto const' (GCC only)

    # Build type specific flags
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Debug>:-fsanitize=address>
    $<$<CONFIG:Debug>:-fsanitize=undefined>
    $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
    $<$<AND:$<CONFIG:Debug>,$<C_COMPILER_ID:GNU>>:-fsanitize=leak>
    $<$<AND:$<CONFIG:Debug>,$<C_COMPILER_ID:GNU>>:-fno-optimize-sibling-calls>
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Release>:-DNDEBUG>
    $<$<CONFIG:Release>:-march=native>
    $<$<CONFIG:Release>:-flto>
)

target_link_options(${PROJECT_NAME} PRIVATE
    # Security hardening
    -Wl,-z,relro                # Read-only relocations
    -Wl,-z,now                  # Immediate binding
    -Wl,-z,noexecstack          # Non-executable stack
    -pie                         # Position independent executable

    # Sanitizers for debug builds
    $<$<CONFIG:Debug>:-fsanitize=address>
    $<$<CONFIG:Debug>:-fsanitize=undefined>
    $<$<AND:$<CONFIG:Debug>,$<C_COMPILER_ID:GNU>>:-fsanitize=leak>

    # Link-time optimization for release
    $<$<CONFIG:Release>:-flto>
)
