#include "SceneTreeWidget.h"
#include <QHeaderView>

SceneTreeWidget::SceneTreeWidget(QWidget* parent) : QTreeWidget(parent), m_Timer(new QTimer(this))
{
	setColumnCount(1);
	setHeaderLabels({ "Scene Tree" });
}

SceneTreeWidget::~SceneTreeWidget()
{

}

void SceneTreeWidget::UpdateTree()
{
	if (!Singleton::IsInitialized()) return;
	if (!Singleton::GetNodeGraph()) return;

	clear();
	UpdateNode(Singleton::GetNodeGraph()->GetRoot(), nullptr);
	expandAll();
	header()->setSectionResizeMode(QHeaderView::Stretch);
}

void SceneTreeWidget::UpdateNode(Node3D* parentNode, QTreeWidgetItem* parentItem)
{
	if (!parentNode) return;

	const std::vector<Node3D*> children = parentNode->GetChildren();
	const bool hasChildren = children.size() > 0;

	for (Node3D* childNode : children)
	{
		QTreeWidgetItem* childItem = nullptr;

		if (parentItem)
		{
			childItem = new QTreeWidgetItem(parentItem);
		}
		else
		{
			childItem = new QTreeWidgetItem(this);
		}

		childItem->setText(0, childNode->GetName().c_str());

		UpdateNode(childNode, childItem);
	}
}