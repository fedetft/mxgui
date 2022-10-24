
#include <QtWidgets/QApplication>
#include <boost/filesystem.hpp>
#include "window.h"

using namespace boost::filesystem;

int main(int argc, char *argv[])
{
    current_path(path(argv[0]).parent_path()); //chdir() to executable's path
    QApplication a(argc,argv);
    Window window;
    return a.exec();
}
