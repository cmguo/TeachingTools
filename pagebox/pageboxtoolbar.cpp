#include "pageboxtoolbar.h"

#include <QHBoxLayout>

static constexpr char const * STYLE =
        "QPushButton,.QLabel{color:white;background-color:#00000000;border:none;font-size:16pt;spacing:30px;}"
        "QPushButton{qproperty-iconSize:30px 30px;border-radius:8px;font-family:'微软雅黑';background-color:#00000000} "
        "QPushButton:checked{background-color:#FF008FFF;}"
        "#pageboxtoolbar{background-color:#C8000000;border-radius:6px;}";

static constexpr char const * POPUP_STYLE =
        "QPushButton,.QLabel{color:white;background-color:#00000000;border:none;font-size:16pt;}"
        "QPushButton{qproperty-iconSize:30px 30px;background-color:#00000000} "
        "QPushButton:checked{background-color:#00000000;}"
        "#popupwidget{background-color:#80000000;border-radius:6px;}";

PageBoxToolBar::PageBoxToolBar(QWidget *parent)
    : ToolbarWidget(parent)
{
    setObjectName(("pageboxtoolbar"));
    setStyleSheet(STYLE);
    setPopupPosition(TopRight);
}

PageBoxToolBar::~PageBoxToolBar()
{
}

QWidget *PageBoxToolBar::createPopupWidget()
{
    QWidget * widget = ToolbarWidget::createPopupWidget();
    widget->setStyleSheet(POPUP_STYLE);
    return widget;
}
