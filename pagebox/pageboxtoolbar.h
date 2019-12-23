#ifndef PAGEBOXTOOLBAR_H
#define PAGEBOXTOOLBAR_H

#include <views/toolbarwidget.h>

class PageNumberWidget;

class PageBoxToolBar : public ToolbarWidget
{
    Q_OBJECT
public:
    explicit PageBoxToolBar(QWidget *parent = nullptr);

    virtual ~PageBoxToolBar() override;
};

#endif // PAGEBOXTOOLBAR_H
