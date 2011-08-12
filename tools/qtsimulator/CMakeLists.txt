
project(QTSIMULATOR)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_BUILD_TYPE release)

set(FOO_SRCS
    ../../font.cpp
    ../../misc_inst.cpp
    ../../display.cpp
    ../../resourcefs.cpp
    ../../resource_image.cpp
    ../../tga_image.cpp
    ../../drivers/display_qt.cpp
    ../../drivers/event_qt.cpp
    ../../level2/input.cpp
    qtbackend.cpp
    window.cpp
    qtsimulator.cpp
    ../../../main.cpp)
set(FOO_HEADERS window.h)

# ../../.. is the main project directory
include_directories(../../..)
add_definitions(-DMXGUI_LIBRARY)

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
qt4_wrap_cpp(FOO_HEADERS_MOC ${FOO_HEADERS})
if(APPLE)
	add_executable(qtsimulator MACOSX_BUNDLE ${FOO_SRCS} ${FOO_HEADERS_MOC})
else()
	add_executable(qtsimulator ${FOO_SRCS} ${FOO_HEADERS_MOC})
endif()
target_link_libraries(qtsimulator ${QT_LIBRARIES})
set(BOOST_LIBS thread system)
find_package(Boost COMPONENTS ${BOOST_LIBS} REQUIRED)
target_link_libraries(qtsimulator ${Boost_LIBRARIES})
find_package(Threads REQUIRED)
target_link_libraries(qtsimulator ${CMAKE_THREAD_LIBS_INIT})
