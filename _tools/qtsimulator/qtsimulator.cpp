
#include <QtWidgets/QApplication>
#include <filesystem>
#include "window.h"

using namespace std::filesystem;

int main(int argc, char *argv[])
{
    current_path(path(argv[0]).parent_path()); //chdir() to executable's path
    QApplication a(argc,argv);
    Window window;
    return a.exec();
}
