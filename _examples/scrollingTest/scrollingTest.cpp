#include <entry.h>
#include <memory>
#include <level2/scrolling_list.h>
//#include <iostream>
using namespace std;
using namespace mxgui;


ENTRY()
{
    shared_ptr<Window> w= make_shared<Window>();
    
    ScrollingList sl=ScrollingList(w.get(),Point(10,10),10,100);
    Label l1 = Label(w.get(),Point(10,170),100,20,"Select a number");
    for( int i=0;i<20;i++)
    {
         sl.addItem(to_string(i));
    }

    sl.setCallback([&l1,&sl](){
        l1.setText("Selected: "+sl.getSelected());
    });


    
    InputHandler::instance().registerEventCallback([w](){
        Event e = InputHandler::instance().popEvent();
        //cout<<"Event: "<<e.getEvent()<<endl;
        w.get()->postEvent(e);
    });
    WindowManager::instance().start(w);
    w.get()->eventLoop();

    return 0;
}