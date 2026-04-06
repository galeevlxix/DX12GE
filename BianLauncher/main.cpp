#include "BianLauncher.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    BianLauncher window;
    window.show();
    return app.exec();
}
