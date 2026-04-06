#pragma once

#include <QFrame>
#include <QWidget>
#include <QHBoxLayout>
#include <QString>
#include <qlabel.h>

class ItemWidget : public QFrame
{
public:
	ItemWidget(const QString& projectName, const QString& projectPath, QWidget* parent = nullptr);

private:
	QHBoxLayout* m_Layout = nullptr;
	QLabel* m_IconLabel = nullptr;
	QVBoxLayout* m_TextLayout = nullptr;
	QLabel* m_NameLabel = nullptr;
	QLabel* m_PathLabel = nullptr;
};

