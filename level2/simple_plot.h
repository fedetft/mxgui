
#ifndef SIMPLE_PLOT_H
#define SIMPLE_PLOT_H

#include <vector>
#include <string>
#include <display.h>
#include <misc_inst.h>

namespace mxgui {

class Dataset
{
public:
    Dataset() : color(white) {}
    Dataset(const std::vector<float>& data, Color color)
        : data(&data), color(color) {}
    
    const std::vector<float>* data;
    Color color;
};

class SimplePlot
{
public:
    SimplePlot(Point upperLeft, Point lowerRight);
    
    void draw(DrawingContext& dc, const std::vector<float>& data,
              Color color=white, bool fullRedraw=false);
    
    void draw(DrawingContext& dc, const std::vector<Dataset>& dataset,
              bool fullRedraw=false);

    void setFont(const Font& font) { this->font=font; }
    
    Point upperLeft;
    Point lowerRight;
    Font font;
    Color foreground;
    Color background;
    
    float ymin;
    float ymax;
    
private:
    std::string number(float num);
    
    bool first;
    float prevYmin,prevYmax;
};

} //namespace mxgui

#endif //SIMPLE_PLOT_H
