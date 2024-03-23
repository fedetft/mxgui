/***************************************************************************
 *   Copyright (C) 2022,2024 by Daniele Cattaneo                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or inline functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#pragma once

#include "display.h"

/** 
 * A class providing methods for drawing multiple lines of text within a given
 * bounding box.
 */
struct TextBox
{
    enum Options
    {
        WrapMask           = 1<<0,
        CharWrap           = 0<<0, ///< Lines can wrap between any character
        WordWrap           = 1<<0, ///< Lines wrap only at word boundaries

        AlignmentMask      = 3<<1,
        LeftAlignment      = 0<<1, ///< Align text lines to the left
        CenterAlignment    = 1<<1, ///< Align text lines to the center
        RightAlignment     = 2<<1, ///< Align text lines to the right

        BackgroundMask     = 1<<3,
        BoxBackground      = 0<<3, ///< Draws the background color within the entire area of the text bounding box.
        TextOnlyBackground = 1<<3, ///< Draws the background color only over the text.

        PartialLinesMask   = 1<<4,
        RemovePartialLines = 0<<4, ///< If the a line of text doesn't fully fit in the box, do not render it.
        ClipPartialLines   = 1<<4  ///< If the a line of text doesn't fully fit in the box, render it partially.
    };

    /**
     * Draws multiple lines of text within a given bounding box with margins.
     * \param dc The drawing context where to draw the text.
     * \param p0 The top-left corner of the bounding box.
     * \param p1 The bottom-right corner of the bounding box.
     * \param str The string of text to draw.
     * \param options A set of options for configuring the drawing process.
     * \param topMargin Size in pixel of the top margin of the box.
     * \param leftMargin Size in pixel of the left margin of the box.
     * \param rightMargin Size in pixel of the right margin of the box.
     * \param rightMargin Size in pixel of the bottom margin of the box.
     * \returns The number of characters effectively drawn from the string.
     */
    static int draw(mxgui::DrawingContext& dc, mxgui::Point p0,
        mxgui::Point p1, const char *str, unsigned int options,
        unsigned short topMargin, unsigned short leftMargin, 
        unsigned short rightMargin, unsigned short bottomMargin,
        int scrollY=0);

    /**
     * Draws multiple lines of text within a given bounding box.
     * \param dc The drawing context where to draw the text.
     * \param p0 The top-left corner of the bounding box.
     * \param p1 The bottom-right corner of the bounding box.
     * \param str The string of text to draw.
     * \param options A set of options for configuring the drawing process.
     * \returns The number of characters effectively drawn from the string.
     */
    static int draw(mxgui::DrawingContext& dc, mxgui::Point p0,
        mxgui::Point p1, const char *str, unsigned int options=0,
        int scrollY=0);
};
