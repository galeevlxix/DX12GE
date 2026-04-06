#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_BianEditor.h"

#include <QMenuBar>
#include <QFrame>
#include <QSplitter>
#include "DirectX12Viewport.h"

#include "SceneTreeWidget.h"

class BianEditor : public QMainWindow
{
    Q_OBJECT

public:
    BianEditor(QWidget *parent = nullptr);
    ~BianEditor();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    Ui::BianEditorClass ui;

    // MENU BAR

    QMenu* m_ProjectMenu;

    QMenu* m_SceneMenu;
    QAction* m_CreateNewScene;
    QAction* m_OpenScene;
    QAction* m_SaveScene;
    QAction* m_SaveAsNewScene;
    QAction* m_Exit;

    QMenu* m_NodeMenu;
    QAction* m_AddNode;
    QAction* m_DeleteNode;

    // LEFT

    QSplitter* m_LeftSplitter;
    SceneTreeWidget* m_SceneTree;

    // RIGHT

    QSplitter* m_RightSplitter;
    QFrame* m_RightPanel;

    // BOTTOM 

    QSplitter* m_BottomSplitter;
    QFrame* m_BottomPanel;

    // VIEWPORT

    DirectX12Viewport* m_Viewport;
    
    void updatePanelConstraints();

public:

    DirectX12Viewport* GetViewport() { return m_Viewport; }
    SceneTreeWidget* GetSceneTreeWidget() { return m_SceneTree; }
};

