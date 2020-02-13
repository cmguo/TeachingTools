#include "pageboxtoolbar.h"

#include <views/qsshelper.h>

#include <QHBoxLayout>

static QssHelper QSS(":/teachingtools/qss/pageboxtoolbar.qss");
static QssHelper QSS2(":/teachingtools/qss/pageboxpopbar.qss");

PageBoxToolBar::PageBoxToolBar(QWidget *parent)
    : ToolbarWidget(parent)
{
    setObjectName(("pageboxtoolbar"));
    setStyleSheet(QSS);
    layout()->setContentsMargins(6, 6, 6, 6);
    setPopupPosition(TopRight);
}

PageBoxToolBar::~PageBoxToolBar()
{
}

QWidget *PageBoxToolBar::createPopupWidget()
{
    QWidget * widget = ToolbarWidget::createPopupWidget();
    widget->setStyleSheet(QSS2);
    return widget;
}
