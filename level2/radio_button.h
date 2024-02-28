/***************************************************************************
 *   Copyright (C) 2014 by Terraneo Federico                               *
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

#ifndef RADIOBUTTON_H
#define	RADIOBUTTON_H
#include "checkbox.h"

#ifdef MXGUI_LEVEL_2

namespace mxgui {

//forward decls
class RadioGroup;
/**
 * RadioButton.
 */
class RadioButton : public CheckBox
{
public:
    
    /**
     * Constructor
     * The object will be immediately enqueued for redraw
     * \param w window to which this object belongs
     * \param group the group to which this radio button belongs
     * \param p upper left point of the CheckBox
     * \param dimension width of the CheckBox ( it's a square )
     * \param text label of the checkbox
     */
    RadioButton(Window *w,RadioGroup *group, Point p, short dimension=15, const std::string& text="");

    /**
     * \internal
     * Overridden this member function to draw the object.
     * \param dc drawing context used to draw the object
     */
    virtual void onDraw(DrawingContextProxy& dc);

    /**
     * Returns the string of the label
     */
    std::string getLabel();
    /**
     * Used by RadioGroup to set the checked state of the radio button
     * \param checked value to set
     */
    void setChecked(bool checked);
    
private:
    RadioGroup* group; ///< The group to which this radio button belongs
    void check();///< Overridden to call the RadioGroup::setChecked
};

class RadioGroup
{
public:
    RadioGroup();
    /**
     * Adds a radio button to the group
     * \param rb the radio button to add
     */
    void addRadioButton(RadioButton* rb);
    /**
     * Sets the checked radio button
     * \param rb the radio button to check
     */
    void setChecked(RadioButton* rb);
    RadioButton* getChecked();//< Returns the checked radio button or nullptr if none is checked
    std::list<RadioButton*> radioButtons;//< The list of radio buttons which belong to this group

private:
    RadioButton* checked;//< The checked radio button
};

} //namesapce mxgui

#endif //MXGUI_LEVEL_2

#endif //RADIOBUTTON_H
