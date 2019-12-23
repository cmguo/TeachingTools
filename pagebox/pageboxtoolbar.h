#ifndef PAGEBOXTOOLBAR_H
#define PAGEBOXTOOLBAR_H

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>


class PageBoxToolBar : public QWidget
{
    Q_OBJECT
public:
    explicit PageBoxToolBar(QWidget *parent = nullptr);
    virtual ~PageBoxToolBar();

    void updateProgressText(QString progress);
    void setIsFull(bool isfull);
Q_SIGNALS:
    void fullStateChanged(bool isFull);
public Q_SLOTS:
    void on_clicked_fullBtn();

public:
    QPushButton *fullBtn;
    QPushButton *scaleUpBtn;
    QPushButton *scaleDownBtn;
    QPushButton *preBtn;
    QLabel *progressLabel;
    QPushButton *nextBtn;
    QPushButton *exitBtn;
    QHBoxLayout *horizontalLayout;

private:
    bool isFullState = false;
};

#endif // PAGEBOXTOOLBAR_H
