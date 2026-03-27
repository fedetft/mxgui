
cmake_minimum_required(VERSION 3.19)
project(GUIUTILS)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 14)
# Force static linking on Windows to avoid having to copy
# the dlls to the executable's directory
if(WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
    list(APPEND CMAKE_EXE_LINKER_FLAGS -static)
endif()

# Note: disabling this option adds a requirement for pkg-config
option(BUILD_FREETYPE "Build freetype from source" ON)

## Find dependencies
if(BUILD_FREETYPE)
    execute_process(COMMAND
        bash ${CMAKE_CURRENT_SOURCE_DIR}/compile_freetype.sh
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND_ERROR_IS_FATAL ANY)
    find_library(FREETYPE
        NAMES freetype
        HINTS ${CMAKE_CURRENT_BINARY_DIR}/freetype-2.4.3/objs/.libs
        REQUIRED)
    # Freetype 2.4.3 depends on ZLIB
    find_package(ZLIB REQUIRED)
else()
    # Find Freetype using pkg-config because CMake's FindFreetype module is
    # completely incompetent at figuring out transitive dependencies
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(Freetype REQUIRED freetype2)
    find_package(Freetype REQUIRED)
endif()
find_package(PNG REQUIRED)
set(BOOST_LIBS program_options filesystem locale)
find_package(Boost COMPONENTS ${BOOST_LIBS} REQUIRED)

## Targets
# fontrendering
set(FONTRENDERING_SRCS bdfparser.cpp ttfparser.cpp fixed_width_generator.cpp
                  variable_width_generator.cpp fixes_parser.cpp font_core.cpp
                  unicode_blocks.cpp fontrendering.cpp)
add_executable(fontrendering ${FONTRENDERING_SRCS})
if(BUILD_FREETYPE)
    target_include_directories(fontrendering PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}/freetype-2.4.3/include)
    target_link_libraries(fontrendering ${FREETYPE} ZLIB::ZLIB)
else()
    target_include_directories(fontrendering PUBLIC ${Freetype_INCLUDE_DIRS})
    if(WIN32)
        target_link_libraries(fontrendering ${Freetype_STATIC_LDFLAGS})
    else()
        target_link_libraries(fontrendering ${Freetype_LDFLAGS})
    endif()
endif()
target_link_libraries(fontrendering PNG::PNG ${Boost_LIBRARIES})

# pngconverter
set(PNGCONV_SRCS pngconverter.cpp)
add_executable(pngconverter ${PNGCONV_SRCS})
target_link_libraries(pngconverter PNG::PNG ${Boost_LIBRARIES})
