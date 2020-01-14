#include "pageboxtoolbar.h"

#include <QHBoxLayout>

PageBoxToolBar::PageBoxToolBar(QWidget *parent)
    : ToolbarWidget(parent)
{
    this->setObjectName(("pageboxtoolbar"));
    this->setAttribute(Qt::WA_StyledBackground,true);
    this->setStyleSheet("QPushButton,.QLabel{color:white;background-color:#00000000;border:none;font-size:16pt;} "
                        "QPushButton{font-family: '微软雅黑';background-color:#00000000} "
                        "QPushButton:checked{background-color:black;}"
                        "#pageboxtoolbar{background-color:#6A000000;border-radius:3px;}");
    layout()->setMargin(0);
    layout()->setContentsMargins(5, 0, 5, 0);
    setPopupPosition(TopRight);
    hide();
}

PageBoxToolBar::~PageBoxToolBar()
{
}
