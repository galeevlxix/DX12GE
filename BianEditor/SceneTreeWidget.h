#pragma once
#include <QTreeWidget>
#include <QTimer>
#include <Engine/Base/Singleton.h>

class SceneTreeWidget : public QTreeWidget
{
	QTimer* m_Timer;

public:
	SceneTreeWidget(QWidget* parent = nullptr);
	~SceneTreeWidget();

	void UpdateTree();

private:
	void UpdateNode(Node3D* node, QTreeWidgetItem* item);
};

