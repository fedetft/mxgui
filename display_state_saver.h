
#ifndef DISPLAY_STATE_SAVER_H
#define DISPLAY_STATE_SAVER_H

#include <utility>
#include <display.h>

namespace mxgui {

class StateSaver
{
public:
    StateSaver(DrawingContext& dc) : dc(dc), savedFont(dc.getFont()), savedColor(dc.getTextColor())
    {}
    
    ~StateSaver()
    {
        dc.setFont(savedFont);
        dc.setTextColor(savedColor);
    }
private:
    DrawingContext& dc;
    Font savedFont;
    std::pair<Color,Color> savedColor;
};

} //namespace mxgui

#endif //DISPLAY_STATE_SAVER_H
