#ifndef PAGEMODELCONTROLWIDGET_H
#define PAGEMODELCONTROLWIDGET_H

#include <QWidget>

#include <core/toolbutton.h>


class PageModelControlWidget:public QWidget
{

public:
    explicit PageModelControlWidget(QWidget *parent = nullptr);

    virtual ~PageModelControlWidget() override;
    ToolButton* toolButton();

private:
    ToolButton *toolButton_ = nullptr;
};

#endif // PAGEMODELCONTROLWIDGET_H
