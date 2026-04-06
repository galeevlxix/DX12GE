#include "BianLauncher.h"
#include "qstylefactory.h"

#include "Helpers.h"

BianLauncher::BianLauncher(QWidget *parent) : QMainWindow(parent)
{
    ui.setupUi(this);

    qApp->setStyle(QStyleFactory::create("Fusion"));
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
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

    ReadSettingsFile();

	m_VLayout = new QVBoxLayout(ui.centralWidget);
    m_VLayout->setContentsMargins(12, 12, 12, 12);
    m_VLayout->setSpacing(12);
    
    m_HLayout = new QHBoxLayout(ui.centralWidget);
    m_HLayout->setContentsMargins(0, 0, 0, 0);
    m_HLayout->setSpacing(12);

    m_TopButtonPanel = new QFrame(this);
    m_TopButtonPanel->setFixedHeight(70);
    m_TopButtonPanel->setFrameShape(QFrame::NoFrame);
    m_TopButtonPanel->setStyleSheet(R"(
            QFrame {
                background: #202020;
                border: 1px solid #303030;
                border-radius: 10px;
            }
            QPushButton {
                min-height: 42px;
                min-width: 140px;
                font-size: 14px;
                border: 1px solid #404040;
                border-radius: 8px;
                background: #2c2c2c;
            }
            QPushButton:hover {
                background: #383838;
            }
            QPushButton:pressed {
                background: #242424;
            }
            QToolTip {
                color: #ffffff;
                background-color: #202020;
                border: 1px solid #303030;
                padding: 6px;
                border-radius: 4px;
            }
        )");

    m_TopButtonLayout = new QHBoxLayout(m_TopButtonPanel);
    m_TopButtonLayout->setContentsMargins(12, 12, 12, 12);
    m_TopButtonLayout->setSpacing(10);

    m_CreateButton = new QPushButton("Create", m_TopButtonPanel);
	m_CreateButton->setToolTip("Create a new project");

    m_ImportButton = new QPushButton("Import", m_TopButtonPanel);
    m_ImportButton->setToolTip("Import a project from another folder");

    m_SettingsButton = new QPushButton("Settings", m_TopButtonPanel);
    m_SettingsButton->setToolTip("Project management settings");

    m_AboutButton = new QPushButton("About", m_TopButtonPanel);
    m_AboutButton->setToolTip("About Bian Engine");

    m_TopButtonLayout->addWidget(m_CreateButton);
    m_TopButtonLayout->addWidget(m_ImportButton);
    m_TopButtonLayout->addWidget(m_SettingsButton);
    m_TopButtonLayout->addWidget(m_AboutButton);
    m_TopButtonLayout->addStretch();

    m_ProjectList = new QListWidget(this);
    m_ProjectList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_ProjectList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_ProjectList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_ProjectList->setSpacing(0);
    m_ProjectList->setFrameShape(QFrame::NoFrame);
    m_ProjectList->setStyleSheet(R"(
            QListWidget {
                background: #181818;
                border: 1px solid #303030;
                border-radius: 10px;
                outline: none;
            }
            QListWidget::item {
                border: none;
                padding: 0px;
                margin: 0px;
            }
            QListWidget::item:hover {
                background: #252525;
            }
            QListWidget::item:selected {
                background: #303030;
            }
        )");

    m_RightButtonPanel = new QFrame(this);
    m_RightButtonPanel->setFixedWidth(220);
    m_RightButtonPanel->setFrameShape(QFrame::NoFrame);
    m_RightButtonPanel->setStyleSheet(R"(
            QFrame {
                background: #202020;
                border: 1px solid #303030;
                border-radius: 10px;
            }
            QPushButton {
                min-height: 42px;
                font-size: 14px;
                border: 1px solid #404040;
                border-radius: 8px;
                background: #2c2c2c;
            }
            QPushButton:hover {
                background: #383838;
            }
            QPushButton:pressed {
                background: #242424;
            }
            QToolTip {
                color: #ffffff;
                background-color: #202020;
                border: 1px solid #303030;
                padding: 6px;
                border-radius: 4px;
            }
        )");

    m_RightButtonLayout = new QVBoxLayout(m_RightButtonPanel);
    m_RightButtonLayout->setContentsMargins(12, 12, 12, 12);
    m_RightButtonLayout->setSpacing(10);

    m_OpenButton = new QPushButton("Open in editor", m_RightButtonPanel);
    m_OpenButton->setToolTip("Open the selected project in the engine editor");

    m_DeleteButton = new QPushButton("Delete", m_RightButtonPanel);
    m_DeleteButton->setToolTip("Delete the selected project and its folder with data");

    m_RenameButton = new QPushButton("Rename", m_RightButtonPanel);
    m_RenameButton->setToolTip("Rename the selected project and its folder");

    m_RightButtonLayout->addWidget(m_OpenButton);
    m_RightButtonLayout->addWidget(m_DeleteButton);
    m_RightButtonLayout->addWidget(m_RenameButton);
    m_RightButtonLayout->addStretch();

    m_VLayout->addWidget(m_TopButtonPanel);
    m_VLayout->addLayout(m_HLayout);
    m_HLayout->addWidget(m_ProjectList, 1);
    m_HLayout->addWidget(m_RightButtonPanel, 0);

    UpdateProjectList();

    connect(m_OpenButton, &QPushButton::clicked, this, [this]() {
        auto* item = m_ProjectList->currentItem();
        if (!item) return;
		// open logic here
        });

    connect(m_DeleteButton, &QPushButton::clicked, this, [this]() {
        auto* item = m_ProjectList->currentItem();
		RemoveProjectItem(item);
        m_ProjectList->setCurrentItem(nullptr);
        });

    connect(m_RenameButton, &QPushButton::clicked, this, [this]() {
        auto* item = m_ProjectList->currentItem();
        if (!item) return;
		// rename logic here
        });

    connect(m_CreateButton, &QPushButton::clicked, this, [this]() { OnCreateProjectButtonPressed(); });
}

BianLauncher::~BianLauncher()
{}

void BianLauncher::ReadSettingsFile()
{
    

}

void BianLauncher::RestoreSettingsFile()
{
    
}

void BianLauncher::AddProjectItem(const QString& name, const QString& path)
{
    QListWidgetItem* item = new QListWidgetItem(m_ProjectList);
    item->setData(Qt::UserRole, path);

    ItemWidget* widget = new ItemWidget(name, path, m_ProjectList);

    item->setSizeHint(widget->sizeHint());
    m_ProjectList->addItem(item);
    m_ProjectList->setItemWidget(item, widget);
}

void BianLauncher::RemoveProjectItem(QListWidgetItem* item)
{
    if (!item) return;
	delete m_ProjectList->takeItem(m_ProjectList->row(item));
}

void BianLauncher::ClearProjectList()
{
    for (int i = m_ProjectList->count() - 1; i >= 0; --i)
    {
        RemoveProjectItem(m_ProjectList->item(i));
	}
}

void BianLauncher::UpdateProjectList()
{
	ClearProjectList();

    // TEST
    AddProjectItem("Sandbox", "D:/GameProjects/Sandbox");
    AddProjectItem("RacingGame", "D:/GameProjects/RacingGame");
    AddProjectItem("RTSDemo", "D:/GameProjects/RTSDemo");
    AddProjectItem("VoxelWorld", "D:/GameProjects/VoxelWorld");
    AddProjectItem("DX12Test", "D:/GameProjects/DX12Test");
    AddProjectItem("EditorLab", "D:/GameProjects/EditorLab");
    AddProjectItem("PhysicsPlayground", "D:/GameProjects/PhysicsPlayground");
    AddProjectItem("OpenWorldPrototype", "D:/GameProjects/OpenWorldPrototype");
}

void BianLauncher::OnCreateProjectButtonPressed()
{
    if (m_CreateProjectWindow)
    {
        m_CreateProjectWindow->close();
        delete m_CreateProjectWindow;
        m_CreateProjectWindow = nullptr;
    }

    m_CreateProjectWindow = new CreateProjectWindow(this);
    m_CreateProjectWindow->show();
}
