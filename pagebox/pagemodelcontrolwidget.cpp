#include "pagemodelcontrolwidget.h"

#include <views/qsshelper.h>

#include <QHBoxLayout>
#include <QPushButton>
#include <QDebug>

#include <core/toolbutton.h>

PageModelControlWidget::PageModelControlWidget(QWidget *parent)
    :QWidget(parent, Qt::FramelessWindowHint)
{
    static QssHelper qss(":/teachingtools/qss/pagenumber.qss");
    QSize iconSize = qss.value("QPushButton", "qproperty-iconSize").toSize();
    setStyleSheet(qss);

    QHBoxLayout* layout = new QHBoxLayout(this);
    QPushButton *preBtn_ = new QPushButton(this);
    preBtn_->setObjectName(("preBtn"));
    preBtn_->setIconSize(iconSize);
    preBtn_->setText("单页");
    preBtn_->setIcon(ToolButton::makeIcon(":teachingtools/icon/single_page.svg,default", iconSize));
    QPushButton *nextBtn_ = new QPushButton(this);
    nextBtn_->setObjectName(("nextBtn"));
    nextBtn_->setIconSize(iconSize);
    nextBtn_->setText("双页");
    nextBtn_->setIcon(ToolButton::makeIcon(":teachingtools/icon/double_page.svg,default", iconSize));
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(preBtn_);
    layout->addWidget(nextBtn_);
    setLayout(layout);
    hide();
    QObject::connect(preBtn_, &QPushButton::clicked, this, [](){
        qDebug()<<"single";
    });
    QObject::connect(nextBtn_, &QPushButton::clicked, this,[](){
        qDebug()<<"dup";
    });
}

PageModelControlWidget::~PageModelControlWidget()
{

}

ToolButton *PageModelControlWidget::toolButton()
{
    if (toolButton_ == nullptr)
        toolButton_ = new ToolButton({"switchPage", "",
                                      ToolButton::CustomWidget, QVariant::fromValue(this)});
    return toolButton_;
}


