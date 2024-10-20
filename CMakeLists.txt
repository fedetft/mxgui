# Copyright (C) 2024 by Skyward
#
# This program is free software; you can redistribute it and/or
# it under the terms of the GNU General Public License as published
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# As a special exception, if other files instantiate templates or use
# macros or inline functions from this file, or you compile this file
# and link it with other works to produce a work based on this file,
# this file does not by itself cause the resulting work to be covered
# by the GNU General Public License. However the source code for this
# file must still be made available in accordance with the GNU
# Public License. This exception does not invalidate any other
# why a work based on this file might be covered by the GNU General
# Public License.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>

cmake_minimum_required(VERSION 3.16)

project(MxGui
    DESCRIPTION "GUI library for the Miosix kernel"
    LANGUAGES C CXX
)

set(MXGUI_PATH ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to mxgui")

set(MXGUI_SRC
    ${MXGUI_PATH}/display.cpp
    ${MXGUI_PATH}/font.cpp
    ${MXGUI_PATH}/misc_inst.cpp
    ${MXGUI_PATH}/tga_image.cpp
    ${MXGUI_PATH}/resource_image.cpp
    ${MXGUI_PATH}/textbox.cpp
    ${MXGUI_PATH}/level2/input.cpp
    ${MXGUI_PATH}/level2/application.cpp
    ${MXGUI_PATH}/level2/drawing_context_proxy.cpp
    ${MXGUI_PATH}/level2/label.cpp
    ${MXGUI_PATH}/level2/button.cpp
    ${MXGUI_PATH}/level2/checkbox.cpp
    ${MXGUI_PATH}/level2/radio_button.cpp
    ${MXGUI_PATH}/level2/scrolling_list.cpp
    ${MXGUI_PATH}/level2/simple_plot.cpp
    ${MXGUI_PATH}/drivers/display_stm3210e-eval.cpp
    ${MXGUI_PATH}/drivers/event_stm3210e-eval.cpp
    ${MXGUI_PATH}/drivers/display_mp3v2.cpp
    ${MXGUI_PATH}/drivers/event_mp3v2.cpp
    ${MXGUI_PATH}/drivers/resfs_mp3v2.cpp
    ${MXGUI_PATH}/drivers/display_strive.cpp
    ${MXGUI_PATH}/drivers/display_st7735.cpp
    ${MXGUI_PATH}/drivers/display_oledboard2.cpp
    ${MXGUI_PATH}/drivers/event_strive.cpp
    ${MXGUI_PATH}/drivers/display_redbull_v2.cpp
    ${MXGUI_PATH}/drivers/event_redbull_v2.cpp
    ${MXGUI_PATH}/drivers/display_bitsboard.cpp
    ${MXGUI_PATH}/drivers/display_sony-newman.cpp
    ${MXGUI_PATH}/drivers/event_sony-newman.cpp
    ${MXGUI_PATH}/drivers/display_stm32f4discovery.cpp
    ${MXGUI_PATH}/drivers/event_stm32f4discovery.cpp
    ${MXGUI_PATH}/drivers/display_generic_1bpp.cpp
    ${MXGUI_PATH}/drivers/display_generic_4bpp.cpp
    ${MXGUI_PATH}/drivers/display_st7735.cpp
    ${MXGUI_PATH}/drivers/display_st25dvdiscovery.cpp
    ${MXGUI_PATH}/drivers/display_stm3220g-eval.cpp
    ${MXGUI_PATH}/drivers/event_stm3220g-eval.cpp
    ${MXGUI_PATH}/drivers/event_st25dvdiscovery.cpp
)

add_library(mxgui STATIC ${MXGUI_SRC})

target_include_directories(mxgui PUBLIC ${MXGUI_PATH})

# The user can set a custom path for mxgui_settings.h
set(MXGUI_SETTINGS_PATH ${MIOSIX_KPATH}/default CACHE PATH "Include directory for miosix_settings.h")
mark_as_advanced(MXGUI_SETTINGS_PATH)
if(NOT MXGUI_SETTINGS_PATH STREQUAL ${MIOSIX_KPATH}/default)
    message(NOTICE "You have set a custom path for mxgui_settings.h")
endif()

# Define "MXGUI_LIBRARY" for private headers
target_compile_definitions(mxgui PRIVATE MXGUI_LIBRARY)

# Link against Miosix
target_link_libraries(mxgui PRIVATE miosix)
