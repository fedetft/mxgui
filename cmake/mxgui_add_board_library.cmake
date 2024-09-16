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

# Add a library target called mxgui-${BOARD_NAME}
#
# Links the interface library Miosix::interface-${BOARD_NAME} to derive all the
# kernel's include directories, compile definitions and compile options.
#
# Arguments:
# - BOARD_NAME: Board name identifier
#
# Required variables:
# - MXGUI_PATH: Path to the mxgui directory
function(mxgui_add_board_library BOARD_NAME)
    if(NOT MXGUI_PATH)
        message(FATAL_ERROR "MXGUI_PATH must be defined")
    endif()

    include(cmake/sources.cmake)

    set(MXGUI_LIB mxgui-${BOARD_NAME})
    add_library(${MXGUI_LIB} STATIC ${MXGUI_SRC})
    add_library(MxGui::${MXGUI_LIB} ALIAS ${MXGUI_LIB})

    target_include_directories(${MXGUI_LIB} PUBLIC ${MXGUI_PATH})

    # The user can set a custom path for mxgui_settings.h
    if(DEFINED CUSTOM_MXGUI_SETTINGS_PATH)
        target_include_directories(${MXGUI_LIB} PRIVATE ${CUSTOM_MXGUI_SETTINGS_PATH})
    else()
        # By default default/config/mxgui_settings.h is used
        target_include_directories(${MXGUI_LIB} PRIVATE ${MXGUI_PATH}/default)
    endif()

    # Define MXGUI_LIB for private headers
    target_compile_definitions(${MXGUI_LIB} PRIVATE MXGUI_LIBRARY)

    target_link_libraries(mxgui-${BOARD_NAME} PRIVATE Miosix::interface-${BOARD_NAME})
endfunction()
