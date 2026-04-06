#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_BianLauncher.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include "ItemWidget.h"
#include <QPushButton>

#include "CreateProjectWindow.h"

class BianLauncher : public QMainWindow
{
    Q_OBJECT

public:
    BianLauncher(QWidget *parent = nullptr);
    ~BianLauncher();

private:
    Ui::BianLauncherClass ui;

    QVBoxLayout* m_VLayout;
    QHBoxLayout* m_HLayout;

    QFrame* m_TopButtonPanel;
    QHBoxLayout* m_TopButtonLayout;
    QPushButton* m_CreateButton;
    QPushButton* m_ImportButton;
    QPushButton* m_SettingsButton;
    QPushButton* m_AboutButton;

    QListWidget* m_ProjectList;

    QFrame* m_RightButtonPanel;
	QVBoxLayout* m_RightButtonLayout;
    QPushButton* m_OpenButton;
    QPushButton* m_DeleteButton;
    QPushButton* m_RenameButton;

    CreateProjectWindow* m_CreateProjectWindow;

    void ReadSettingsFile();
    void RestoreSettingsFile();

    void AddProjectItem(const QString& name, const QString& path);
    void RemoveProjectItem(QListWidgetItem* item);
	void ClearProjectList();
	void UpdateProjectList();


    void OnCreateProjectButtonPressed();
};

