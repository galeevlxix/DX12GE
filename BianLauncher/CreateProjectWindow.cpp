#include "CreateProjectWindow.h"

CreateProjectWindow::CreateProjectWindow(QWidget* parent) : QMainWindow(parent)
{
    if (objectName().isEmpty())
    {
        setObjectName("BianCreateProjectClass");
    }
    resize(621, 380);

    setWindowTitle("Bian Engine - Create new project");
}

CreateProjectWindow::~CreateProjectWindow()
{

}


