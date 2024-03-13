#include <entry.h>
#include <memory>
#include <level2/scrolling_list.h>
#include <iostream>
using namespace std;
using namespace mxgui;


ENTRY()
{
    shared_ptr<Window> w= make_shared<Window>();
    
    ScrollingList sl=ScrollingList(w.get(),Point(10,10),10,100);
    Label l1 = Label(w.get(),Point(10,220),100,20,"Select a number");
    for( int i=0;i<50;i++)
    {
         sl.addItem(to_string(i));
    }

    sl.setCallback([&l1,&sl](){
        l1.setText("Selected: "+sl.getSelected());
    });

    ScrollingList sl2=ScrollingList(w.get(),Point(120,30),1,60,20,40);
    Label l2 = Label(w.get(),Point(120,5),100,20,"Select your Beatle");

    sl2.addItem("John");
    sl2.addItem("Paul");
    sl2.addItem("George");
    sl2.addItem("Ringo");

    Label l3 = Label(w.get(),Point(120,220),100,20,"");
    sl2.setCallback([&l3,&sl2](){
        l3.setText("Selected: "+sl2.getSelected());
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