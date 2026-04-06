#include "ItemWidget.h"
#include <QStyle>

ItemWidget::ItemWidget(const QString& projectName, const QString& projectPath, QWidget* parent) : QFrame(parent)
{
    setObjectName("ProjectItem");
    setFrameShape(QFrame::NoFrame);

    m_Layout = new QHBoxLayout(this);
    m_Layout->setContentsMargins(12, 12, 12, 12);
    m_Layout->setSpacing(12);

    m_IconLabel = new QLabel(this);
    m_IconLabel->setFixedSize(48, 48);
    QIcon icon = style()->standardIcon(QStyle::SP_DirIcon);
    m_IconLabel->setPixmap(icon.pixmap(48, 48));
    m_IconLabel->setAlignment(Qt::AlignCenter);

    m_TextLayout = new QVBoxLayout();
    m_TextLayout->setContentsMargins(0, 0, 0, 0);
    m_TextLayout->setSpacing(4);

    m_NameLabel = new QLabel(projectName, this);
    QFont nameFont;
    nameFont.setPointSize(12);
    nameFont.setBold(true);
    m_NameLabel->setFont(nameFont);

    m_PathLabel = new QLabel(projectPath, this);
    QFont pathFont;
    pathFont.setPointSize(9);
    m_PathLabel->setFont(pathFont);
    m_PathLabel->setWordWrap(true);
    m_PathLabel->setStyleSheet("color: gray;");

    m_TextLayout->addWidget(m_NameLabel);
    m_TextLayout->addWidget(m_PathLabel);

    m_Layout->addWidget(m_IconLabel, 0, Qt::AlignTop);
    m_Layout->addLayout(m_TextLayout, 1);

    setStyleSheet(R"(
            QFrame#ProjectItemWidget {
                background: #232323;
                border: 1px solid #353535;
                border-radius: 8px;
            }
        )");
}
