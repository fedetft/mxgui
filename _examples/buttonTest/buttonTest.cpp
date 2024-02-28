#include <entry.h>
#include <memory>
#include <level2/checkbox.h>
#include <level2/button.h>
#include <level2/radio_button.h>
#include <level2/label.h>
//#include <iostream>
using namespace std;
using namespace mxgui;


ENTRY()
{
    int counter=0;
    RadioGroup rg=RadioGroup();
    RadioGroup rg2=RadioGroup();
    shared_ptr<Window> w= make_shared<Window>();
    Button b1=Button(w.get(),DrawArea(Point(10,10),Point(70,40)),"Button 1");
    Label l1=Label(w.get(),Point(110,10),5,20,"0");

    CheckBox c1=CheckBox(w.get(),Point(10,60),15,"Check 1");
    Label l2=Label(w.get(),Point(110,60),80,20,"false");
    CheckBox c2=CheckBox(w.get(),Point(10,80),15,"Check 2",true);
    Label l5=Label(w.get(),Point(110,80),80,20,"true");

    c1.setCallback([&l2,&c1](){
        string s="false";
        if(c1.isChecked())
            s="true";
        l2.setText(s);
    });

    c2.setCallback([&l5,&c1](){
        string s="false";
        if(c1.isChecked())
            s="true";
        l5.setText(s);
    });

    RadioButton r00=RadioButton(w.get(),&rg2,Point(10,110),15,"Mario");
    RadioButton r01=RadioButton(w.get(),&rg2,Point(10,130),15,"Luigi");
    Label l4=Label(w.get(),Point(110,110),70,20,"None");
    auto cb2 = [&rg2,&l4]() { l4.setText(rg2.getChecked()->getLabel()); };

    r00.setCallback(cb2);
    r01.setCallback(cb2);
    
    RadioButton r1=RadioButton(w.get(),&rg,Point(10,160),15,"Radio 1");
    RadioButton r2=RadioButton(w.get(),&rg,Point(10,200),15,"Radio 2");
    RadioButton r3=RadioButton(w.get(),&rg,Point(10,240),15,"Radio 3");
    
    Label l3=Label(w.get(),Point(110,160),70,20,"Radio 1");
    auto cb = [&rg,&l3]() { l3.setText(rg.getChecked()->getLabel()); };

    r1.setCallback(cb);
    r2.setCallback(cb);
    r3.setCallback(cb);
    rg.setChecked(&r1);
    b1.setCallback([&l1,&counter](){
        counter=++counter%10;
        l1.setText(to_string(counter));
    });

    Button b2=Button(w.get(),Point(10,280),25,35,"Exit");
    b2.setCallback([w](){
        //brutal way to do it, but it's just an example
        w.get()->~Window();
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