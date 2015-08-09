
#include "simple_plot.h"
#include "display_state_saver.h"
#include <cmath>
#include <limits>
#include <sstream>

using namespace std;

namespace mxgui {

//
// class SimplePlot
//

SimplePlot::SimplePlot(Point upperLeft, Point lowerRight) : upperLeft(upperLeft), lowerRight(lowerRight),
    font(droid11)
{
    foreground=white;
    background=black;
    
    ymin=0.f;
    ymax=1.f;
    
    first=true;
}

void SimplePlot::draw(DrawingContext& dc, const vector<float>& data, Color color, bool fullRedraw)
{
    vector<Dataset> dataset;
    dataset.push_back(Dataset(data,color));
    draw(dc,dataset,fullRedraw);
}

void SimplePlot::draw(DrawingContext& dc, const vector<Dataset>& dataset, bool fullRedraw)
{
    if(first) fullRedraw=true;
    
    int numElem=0;
    if(dataset.empty()==false)
    {
        numElem=dataset.front().data->size();
        for(vector<Dataset>::const_iterator it=dataset.begin();it!=dataset.end();++it)
        {
            if(it->data->size()!=numElem) return; //For now inconsistent size unsupported
            for(vector<float>::const_iterator it2=it->data->begin();it2!=it->data->end();++it2)
            {
                ymin=min(ymin,*it2);
                ymax=max(ymax,*it2);
            }
        }
    }
    
    const int fh=font.getHeight();
    const int whitespaceBeforeTicks=1;
    const int ticksLength=4;
    const int ticksYspace=font.calculateLength("-8.8e+88"); //max space occupied by number()
    const int ticksXspace=fh;
    
    //20 is a guess, it represents some pixels for the actual graph, not just axes
    if(lowerRight.x()-upperLeft.x()<ticksYspace+whitespaceBeforeTicks+ticksLength+20) return;
    if(lowerRight.y()-upperLeft.y()<ticksXspace+whitespaceBeforeTicks+ticksLength+20) return;
    
    StateSaver dcState(dc);
    
    dc.setFont(font);
    dc.setTextColor(make_pair(foreground,background));
    
    if(fullRedraw) dc.clear(upperLeft,lowerRight,background);
    
    //Draw ticks
    if(fullRedraw || ymin!=prevYmin)
    {
        prevYmin=ymin;
        //TODO: avoid clearing
        dc.clear(Point(upperLeft.x(),lowerRight.y()-ticksXspace-fh),
                 Point(upperLeft.x()+ticksYspace,lowerRight.y()+ticksXspace),background);
        string xt=number(ymin);
        dc.write(Point(upperLeft.x()+ticksYspace-font.calculateLength(xt.c_str()),
                       lowerRight.y()-ticksXspace-fh),xt.c_str());
    }
    if(fullRedraw || ymax!=prevYmax)
    {
        prevYmax=ymax;
        //TODO: avoid clearing
        dc.clear(upperLeft,
                 Point(upperLeft.x()+ticksYspace,upperLeft.y()+fh),background);
        string xt=number(ymax);
        dc.write(Point(upperLeft.x()+ticksYspace-font.calculateLength(xt.c_str()),
                       upperLeft.y()),xt.c_str());
    }
    //TODO: For now always redraw X ticks max
    //TODO: avoid clearing
    dc.clear(Point(lowerRight.x()-ticksYspace,lowerRight.y()-fh),
             lowerRight,background);
    string xt=number(numElem>2 ? numElem-1 : 1);
    dc.write(Point(lowerRight.x()-font.calculateLength(xt.c_str()),lowerRight.y()-fh),
             xt.c_str());
    
    //Plot drawing area
    const int x1=upperLeft.x()+ticksYspace+whitespaceBeforeTicks+ticksLength+2;
    const int y1=upperLeft.y();
    const int x2=lowerRight.x();
    const int y2=lowerRight.y()-ticksXspace-whitespaceBeforeTicks-ticksLength-2;
    const int h=y2-y1+1;
    
    //Draw axes
    if(fullRedraw)
    {
        dc.line(Point(x1-1,y1),Point(x1-1,y2+ticksLength),foreground);
        dc.line(Point(x1-1,y1),Point(x1-1-ticksLength,y1),foreground);
        dc.line(Point(x1-ticksLength,y2+1),Point(x2,y2+1),foreground);
        dc.line(Point(x2,y2+1),Point(x2,y2+1+ticksLength),foreground);
        dc.write(Point(upperLeft.x()+ticksYspace+whitespaceBeforeTicks+ticksLength,
                       lowerRight.y()-fh),"0");
    }
    
    if(numElem<2)
    {
        //Can't plot a single value (or zero values!)
        dc.clear(Point(x1,y1),Point(x2,y2),background);
        return;
    }
    
    if(numElem<x2-x1)
    {
        //More points on screen than data points
        
        //TODO: avoid clearing
        dc.clear(Point(x1,y1),Point(x2,y2),background);
        
        const float incr=static_cast<float>(x2-x1+1)/static_cast<float>(numElem-1);
        for(vector<Dataset>::const_iterator it=dataset.begin();it!=dataset.end();++it)
        {
            float xAcc=x1;
            int xPrev=x1;
            int yPrev=-1;
            vector<float>::const_iterator it2=it->data->begin();
            if(!isnan(*it2))
                yPrev=min(y2,max<int>(y1,y2-((*it2-ymin)/(ymax-ymin)*static_cast<float>(h))));
            for(++it2;it2!=it->data->end();++it2)
            {
                float xAccNext=xAcc+incr;
                int x=min(x2,static_cast<int>(xAccNext+0.5f));
                int y;
                if(!isnan(*it2))
                {
                    y=min(y2,max<int>(y1,y2-((*it2-ymin)/(ymax-ymin)*static_cast<float>(h))));
                    if(yPrev>=0) dc.line(Point(xPrev,yPrev),Point(x,y),it->color);
                } else y=-1;
                xAcc=xAccNext;
                yPrev=y;
                xPrev=x;
            }
        }
    } else {
        //More data points than points on screen
        
        const float incr=static_cast<float>(numElem)/static_cast<float>(x2-x1+1);
        float xAcc=0.f;
        
        vector<Color> buffer;
        buffer.resize(h);
        vector<pair<int,int> > prevY;
        prevY.resize(dataset.size(),make_pair(-1,-1));
        
        for(int x=x1;x<=x2;x++)
        {
            fill(buffer.begin(),buffer.end(),background);
            
            float xAccNext=xAcc+incr;
            int range1=min(static_cast<int>(xAcc),numElem-1);
            int range2=min(static_cast<int>(xAccNext),numElem);
            xAcc=xAccNext;
            
            for(int i=0;i<dataset.size();i++)
            {
                int yMin=numeric_limits<int>::max();
                int yMax=numeric_limits<int>::min();
                for(int r=range1;r<range2;r++)
                {
                    float num=dataset.at(i).data->at(r);
                    if(isnan(num)) continue;
                    int y=min(h-1,max<int>(0,((num-ymin)/(ymax-ymin)*static_cast<float>(h))));
                    yMin=min(yMin,y);
                    yMax=max(yMax,y);
                }
                if(yMin!=numeric_limits<int>::max())
                {
                    int oldMin=prevY.at(i).first;
                    int oldMax=prevY.at(i).second;
                    if(oldMin>0)
                    {
                        if(yMin>oldMax) yMin=oldMax+1;
                        if(yMax<oldMin) yMax=oldMin-1;
                    }
                    for(int y=yMin;y<=yMax;y++) buffer.at(y)=buffer.at(y) | dataset.at(i).color;
                } else {
                    yMin=yMax=-1;
                }
                prevY.at(i)=make_pair(yMin,yMax);
            }
            
            //TODO: we need a vertical scanline primitive to optimize this
            dc.beginPixel();
            for(int i=0;i<h;i++) dc.setPixel(Point(x,y2-i),buffer.at(i));
        }
    }
    
    first=false;
}

string SimplePlot::number(float num)
{
    ostringstream ss;
    ss.precision(2);
    ss<<num;
    return ss.str();
}

} //namespace mxgui
