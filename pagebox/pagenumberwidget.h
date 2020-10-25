#ifndef PAGENOWIDGET_H
#define PAGENOWIDGET_H

#include <QWidget>
#include "TeachingTools_global.h"

class ToolButton;
class ResourcePage;

class QPushButton;
class QLabel;

class TEACHINGTOOLS_EXPORT PageNumberWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PageNumberWidget(QWidget *parent = nullptr);

    virtual ~PageNumberWidget() override;

signals:
    void pageNumberChanged(int no);

public:
    int total() const { return total_; }

    void setTotal(int n);

    int number() const { return no_; }

    // no signal
    void setNumber(int n);

    bool gotoPrev();

    bool gotoNext();

    bool gotoPage(int n);

    bool isFirstPage() const;

    bool isLastPage() const;

    void notify();

    ToolButton* toolButton();

    void attachResourcePage(ResourcePage * page);

private:
    void buttonClicked();

private:
    QPushButton *preBtn_;
    QLabel *progressLabel_;
    QPushButton *nextBtn_;

private:
    ToolButton* toolButton_ = nullptr;
    ResourcePage * page_ = nullptr;

private:
    int total_ = 0;
    int no_ = -1;
};

#endif // PAGENOWIDGET_H
