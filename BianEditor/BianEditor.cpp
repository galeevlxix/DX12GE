#include "BianEditor.h"
#include "qstylefactory.h"
#include <QHBoxLayout>
#include "EngineController.h"

#include <QTreeWidget>

const int MinPanelSize = 240;

BianEditor::BianEditor(QWidget *parent) : QMainWindow(parent)
{
    ui.setupUi(this);

    qApp->setStyle(QStyleFactory::create("Fusion"));
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(35, 35, 35));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(130, 130, 130));
    darkPalette.setColor(QPalette::Highlight, QColor(130, 130, 130));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    darkPalette.setColor(QPalette::PlaceholderText, QColor(130, 130, 130));
    qApp->setPalette(darkPalette);
    qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
    
    QWidget* central = ui.centralWidget;
    
    // MENU BAR 

    m_ProjectMenu = menuBar()->addMenu("Project");

    m_SceneMenu = menuBar()->addMenu("Scene");

    m_CreateNewScene = m_SceneMenu->addAction("Create new");
    m_OpenScene = m_SceneMenu->addAction("Open");
    m_SaveScene = m_SceneMenu->addAction("Save");
    m_SaveAsNewScene = m_SceneMenu->addAction("Save as");
    m_Exit = m_SceneMenu->addAction("Exit");
    connect(m_Exit, &QAction::triggered, this, &QWidget::close);

    m_NodeMenu = menuBar()->addMenu("Node");
    m_AddNode = m_NodeMenu->addAction("Add");
    m_DeleteNode = m_NodeMenu->addAction("Delete");
    
    QHBoxLayout* mainHLayout = new QHBoxLayout(central);
    mainHLayout->setContentsMargins(0, 0, 0, 0);
    mainHLayout->setSpacing(0);

    m_LeftSplitter = new QSplitter(Qt::Horizontal, central);
    m_LeftSplitter->setStyleSheet(R"(
        QSplitter::handle {
            background: #2b2b2b;
        }
        QSplitter::handle:horizontal {
            width: 4px;
        }
    )");

    m_SceneTree = new SceneTreeWidget(m_LeftSplitter);
    m_SceneTree->setFrameShape(QFrame::StyledPanel);
    m_SceneTree->setMinimumWidth(MinPanelSize);

    QWidget* rightWidget = new QWidget(m_LeftSplitter);
    
    {
        QHBoxLayout* rightLayout = new QHBoxLayout(rightWidget);
        rightLayout->setContentsMargins(0, 0, 0, 0);
        rightLayout->setSpacing(0);

        m_RightSplitter = new QSplitter(Qt::Horizontal, rightWidget);

        m_RightPanel = new QFrame(m_RightSplitter);
        m_RightPanel->setFrameShape(QFrame::StyledPanel);
        m_RightPanel->setMinimumWidth(MinPanelSize);

        QWidget* centralWidget = new QWidget(m_RightSplitter);

        {
            QVBoxLayout* centralLayout = new QVBoxLayout(centralWidget);
            centralLayout->setContentsMargins(0, 0, 0, 0);
            centralLayout->setSpacing(0);

            m_BottomSplitter = new QSplitter(Qt::Vertical, centralWidget);

            m_BottomPanel = new QFrame(m_BottomSplitter);
            m_BottomPanel->setFrameShape(QFrame::StyledPanel);
            m_BottomPanel->setMinimumHeight(MinPanelSize);

            m_Viewport = new DirectX12Viewport(m_BottomSplitter);

            m_BottomSplitter->addWidget(m_Viewport);
            m_BottomSplitter->addWidget(m_BottomPanel);

            m_BottomSplitter->setChildrenCollapsible(false);
            m_BottomSplitter->setStretchFactor(0, 0);
            m_BottomSplitter->setStretchFactor(1, 1);

            centralLayout->addWidget(m_BottomSplitter);
        }

        m_RightSplitter->addWidget(centralWidget);
        m_RightSplitter->addWidget(m_RightPanel);

        m_RightSplitter->setChildrenCollapsible(false);
        m_RightSplitter->setStretchFactor(0, 0);
        m_RightSplitter->setStretchFactor(1, 1);

        rightLayout->addWidget(m_RightSplitter);
    }

    m_LeftSplitter->addWidget(m_SceneTree);
    m_LeftSplitter->addWidget(rightWidget);

    m_LeftSplitter->setChildrenCollapsible(false);
    m_LeftSplitter->setStretchFactor(0, 0); 
    m_LeftSplitter->setStretchFactor(1, 1); 

    mainHLayout->addWidget(m_LeftSplitter);
}

BianEditor::~BianEditor()
{}

void BianEditor::resizeEvent(QResizeEvent * event)
{
    QMainWindow::resizeEvent(event);
    updatePanelConstraints();
}

void BianEditor::closeEvent(QCloseEvent* event)
{
    EngineController::Quit();
}

void BianEditor::updatePanelConstraints()
{
    if (!centralWidget() || !m_SceneTree || !m_LeftSplitter || !m_RightPanel || !m_RightSplitter || !m_BottomPanel || !m_BottomSplitter)
        return;

    const int maxPanelWidth = max(1, centralWidget()->width() * 30 / 100);
    const int maxPanelHeight = max(1, centralWidget()->height() * 50 / 100);

    m_SceneTree->setMaximumWidth(maxPanelWidth);
    m_RightPanel->setMaximumWidth(maxPanelWidth);
    m_BottomPanel->setMaximumHeight(maxPanelHeight);

    {
        QList<int> sizes = m_LeftSplitter->sizes();
        const int total = sizes[0] + sizes[1];
        sizes[0] = MinPanelSize;
        sizes[1] = max(0, total - MinPanelSize);
        m_LeftSplitter->setSizes(sizes);
    }

    {
        QList<int> sizes = m_RightSplitter->sizes();
        const int total = sizes[0] + sizes[1];
        sizes[1] = MinPanelSize;
        sizes[0] = max(0, total - MinPanelSize);
        m_RightSplitter->setSizes(sizes);
    }

    {
        QList<int> sizes = m_BottomSplitter->sizes();
        const int total = sizes[0] + sizes[1];
        sizes[1] = MinPanelSize;
        sizes[0] = max(0, total - MinPanelSize);
        m_BottomSplitter->setSizes(sizes);
    }
}

