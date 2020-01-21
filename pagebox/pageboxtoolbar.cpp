#include "pageboxtoolbar.h"

#include <QHBoxLayout>

PageBoxToolBar::PageBoxToolBar(QWidget *parent)
    : ToolbarWidget(parent)
{
    setObjectName(("pageboxtoolbar"));
    setAttribute(Qt::WA_StyledBackground);
    //setAttribute(Qt::WA_TranslucentBackground);
    this->setStyleSheet("QPushButton,.QLabel{color:white;background-color:#00000000;border:none;font-size:16pt;} "
                        "QPushButton{qproperty-iconSize: 30px 30px;border-radius:8px;font-family:'微软雅黑';background-color:#00000000} "
                        "QPushButton:checked{background-color:#FF008FFF;}"
                        "#pageboxtoolbar{background-color:#F22B3034;border-radius:6px;}");
    layout()->setMargin(0);
    layout()->setContentsMargins(5, 0, 5, 0);
    setPopupPosition(TopRight);
    hide();
}

PageBoxToolBar::~PageBoxToolBar()
{
}

QWidget *PageBoxToolBar::createPopupWidget()
{
    QWidget * widget = ToolbarWidget::createPopupWidget();
    //widget->setStyleSheet("#popupwidget{background-color:#F22B3034;border-radius:6px;}");
    //widget->setAttribute(Qt::WA_TranslucentBackground);
    return widget;
}
