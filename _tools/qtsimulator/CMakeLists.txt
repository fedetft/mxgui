
project(QTSIMULATOR)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_BUILD_TYPE release)

# Function to handle embedding images in the executable binary
# It basically calls the pngconverter tool and generates the corresponding
# .cpp and .h file in the build directory
# Adapted from http://www.cmake.org/pipermail/cmake/2010-June/037733.html
find_program(PNGCONV_EXECUTABLE pngconverter PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../code_generators/build)
function(preprocess_png out_var)
  set(result)
  foreach(file ${ARGN})
    get_filename_component(basename ${file} NAME_WE)
    get_filename_component(path ${file} PATH)
    set(png "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
    set(cpp "${CMAKE_CURRENT_SOURCE_DIR}/${path}/${basename}.cpp")
    set(h   "${CMAKE_CURRENT_SOURCE_DIR}/${path}/${basename}.h")
    add_custom_command(OUTPUT ${cpp} ${h}
      COMMAND ${PNGCONV_EXECUTABLE} --in ${png} --depth 16
      DEPENDS ${png}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Preprocessing ${file}"
      VERBATIM
      )
    set_source_files_properties(${cpp} PROPERTIES GENERATED 1)
    set_source_files_properties(${h}   PROPERTIES GENERATED 1)
    list(APPEND result ${cpp} ${h})
  endforeach()
  set(${out_var} "${result}" PARENT_SCOPE)
endfunction()

# List here .png files to be embedded in the executable
set(FOO_IMG
   #../../_examples/clipped-image/dis.png
   )
preprocess_png(IMG_OUT ${FOO_IMG})

# List here your application files
set(FOO_SRCS
    ../../_examples/teapot/fps_counter.cpp
    ../../_examples/teapot/bresenham_fsm.cpp
    ../../_examples/teapot/triangle_fsm.cpp
    ../../_examples/teapot/rendering_engine.cpp
    ../../_examples/teapot/teapot.cpp)

# These are the sources of the mxgui library and the simulator
set(LIB_SRCS
    ../../font.cpp
    ../../misc_inst.cpp
    ../../display.cpp
    ../../resourcefs.cpp
    ../../resource_image.cpp
    ../../tga_image.cpp
    ../../drivers/display_qt.cpp
    ../../drivers/event_qt.cpp
    ../../level2/input.cpp
    ../../level2/application.cpp
    ../../level2/drawing_context_proxy.cpp
    ../../level2/label.cpp
    ../../level2/simple_plot.cpp
    qtbackend.cpp
    window.cpp
    qtsimulator.cpp)

set(FOO_HEADERS window.h)

# ../.. is the mxgui directory
include_directories(../..)
# ../../.. is the main project directory
include_directories(../../..)
add_definitions(-DMXGUI_LIBRARY)

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
qt4_wrap_cpp(FOO_HEADERS_MOC ${FOO_HEADERS})
if(APPLE)
	add_executable(qtsimulator MACOSX_BUNDLE ${LIB_SRCS} ${FOO_SRCS} ${IMG_OUT} ${FOO_HEADERS_MOC})
else()
	add_executable(qtsimulator ${LIB_SRCS} ${FOO_SRCS} ${IMG_OUT} ${FOO_HEADERS_MOC})
endif()
target_link_libraries(qtsimulator ${QT_LIBRARIES})
set(BOOST_LIBS thread filesystem system)
find_package(Boost COMPONENTS ${BOOST_LIBS} REQUIRED)
target_link_libraries(qtsimulator ${Boost_LIBRARIES})
find_package(Threads REQUIRED)
target_link_libraries(qtsimulator ${CMAKE_THREAD_LIBS_INIT})
