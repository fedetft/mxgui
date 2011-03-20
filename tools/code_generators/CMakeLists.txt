
cmake_minimum_required(VERSION 2.6)
project(GUIUTILS)

## This script has to run before CMake looks for libraries, since it builds
## freetype 2.4.3
set(CMAKE_BUILD_TYPE Release)
execute_process(COMMAND ../compile_freetype.sh)

## Targets
set(FONTCORE_SRCS bdfparser.cpp ttfparser.cpp fixed_width_generator.cpp
                  variable_width_generator.cpp fixes_parser.cpp font_core.cpp)
add_library(fontcore STATIC ${FONTCORE_SRCS})

set(FONTRENDERING_SRCS fontrendering.cpp)
add_executable(fontrendering ${FONTRENDERING_SRCS})

set(PNGCONV_SRCS pngconverter.cpp)
add_executable(pngconverter ${PNGCONV_SRCS})

## Link libraries
find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})
target_link_libraries(fontrendering fontcore ${PNG_LIBRARY})
target_link_libraries(pngconverter ${PNG_LIBRARY})

set(BOOST_LIBS program_options system)
find_package(Boost COMPONENTS ${BOOST_LIBS} REQUIRED)
target_link_libraries(fontrendering ${Boost_LIBRARIES})
target_link_libraries(pngconverter ${Boost_LIBRARIES})

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIR})
target_link_libraries(fontrendering fontcore ${ZLIB_LIBRARY})

## Uncomment this if you want to use the system's freetype library
#find_package(Freetype REQUIRED)
#include_directories(${FREETYPE_INCLUDE_DIRS})
#target_link_libraries(fontrendering ${FREETYPE_LIBRARIES})
## Uncomment this if you want to build and use freetype 2.4.3
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/build/freetype-2.4.3/include)
find_library(FREETYPE NAMES freetype HINTS ${CMAKE_CURRENT_SOURCE_DIR}/build/freetype-2.4.3/objs/.libs)
target_link_libraries(fontrendering ${FREETYPE})
