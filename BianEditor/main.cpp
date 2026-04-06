#include "BianEditor.h"
#include <QtWidgets/QApplication>

#include "EngineController.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    BianEditor window;
    window.showMaximized();

    DirectX12Viewport* viewport = window.GetViewport();
    assert(EngineController::Run(viewport) == 0 && "Failed to run render engine.");
    window.GetSceneTreeWidget()->UpdateTree();

    return app.exec();
}
