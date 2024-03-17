#include "mxgui/entry.h"
#include "mxgui/display.h"
#include <cmath>

using namespace mxgui;

void drawGradientBar(DrawingContext& dc, int y0, int y1, int r, int g, int b)
{
    for(int x=0; x<dc.getWidth(); x++)
    {
        unsigned int rr = x * r / (dc.getWidth()-1);
        unsigned int gg = x * g / (dc.getWidth()-1);
        unsigned int bb = x * b / (dc.getWidth()-1);
        Color col = bb + (gg << 5) + (rr << 11);
        dc.line(Point(x,y0), Point(x,y1-1), col);
    }
}

void drawDitherBar(DrawingContext& dc, int y0, int y1, int r, int g, int b)
{
    // Floyd-Steinberg dithering
    // (rotated by 90 degrees and flipped around a vertical axis)
    const int one = 0x10000;
    dc.beginPixel();
    // The error matrix has dummy elements in the top and bottom row
    // to avoid handling border conditions. Column 0 is always the one currently
    // being scanned, and column 1 is the one at its left.
    int (*error)[2] = new int[y1-y0+2][2];
    for(int y=0; y<y1-y0+2; y++)
    {
        error[y][0] = 0;
        error[y][1] = 0;
    }
    for(int x=0; x<dc.getWidth(); x++)
    {
        float vg = (float)x / (float)(dc.getWidth()-1);
        vg = std::pow(vg, 2.2f); // sRGB gamma 2.2
        int v = (int)(vg * one);
        int dy = (x%2) * 2 - 1; // Switch scanning direction at each line
        for(int y = (x%2)?0:y1-y0-1; 0<=y && y<y1-y0; y+=dy)
        {
            int accum = v + error[y+1][0];
            int cap = accum>=one/2 ? one : 0;
            int err = accum - cap;
            error[y+1+dy][0] += (err * 7 + 8) / 16;
            error[y+1-dy][1] += (err * 3 + 8) / 16;
            error[y+1   ][1] += (err * 5 + 8) / 16;
            error[y+1+dy][1] += (err * 1 + 8) / 16;
            unsigned int rr = cap ? r : 0;
            unsigned int gg = cap ? g : 0;
            unsigned int bb = cap ? b : 0;
            Color col = bb + (gg << 5) + (rr << 11);
            dc.setPixel(Point(x, y0+y), col);
        }
        // Shift columns to the left by 1 in the error matrix
        for(int y=0; y<y1-y0+2; y++)
        {
            error[y][0] = error[y][1];
            error[y][1] = 0;
        }
    }
    delete[] error;
}

ENTRY()
{
    #ifndef MXGUI_COLOR_DEPTH_16_BIT
    #error "selected color depth not yet handled by this program"
    #endif
    // Display four gradients, one for gray and one for each primary color,
    // to help calibrating gamma curves.
	{
		DrawingContext dc(DisplayManager::instance().getDisplay());
        int height = dc.getHeight();
        drawGradientBar(dc,          0, height*1/8, 31, 63, 31);
        drawDitherBar  (dc, height*1/8, height*2/8, 31, 63, 31);
        drawGradientBar(dc, height*2/8, height*3/8, 31,  0,  0);
        drawDitherBar  (dc, height*3/8, height*4/8, 31,  0,  0);
        drawGradientBar(dc, height*4/8, height*5/8,  0, 63,  0);
        drawDitherBar  (dc, height*5/8, height*6/8,  0, 63,  0);
        drawGradientBar(dc, height*6/8, height*7/8,  0,  0, 31);
        drawDitherBar  (dc, height*7/8, height    ,  0,  0, 31);
	}
	for(;;) ;
}
