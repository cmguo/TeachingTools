#include "pageboxtoolbar.h"

#include <QHBoxLayout>

PageBoxToolBar::PageBoxToolBar(QWidget *parent): QWidget(parent)
{
    this->setObjectName(("pageboxtoolbar"));
    this->setAttribute(Qt::WA_StyledBackground,true);
    this->setStyleSheet("QPushButton,.QLabel{color:white;background-color:#00000000;border:none;font-size:16pt} "
                        "#pageboxtoolbar{background-color:#6A000000;border-radius:3px;}");
    horizontalLayout = new QHBoxLayout(this);
    horizontalLayout->setMargin(0);
    horizontalLayout->setContentsMargins(5,0,5,0);
    fullBtn = new QPushButton(this);
    fullBtn->setIconSize(QSize(20,20));
    fullBtn->setIcon(QIcon(":/icon/icon/enter_full_screen_btn_icon.svg"));
    fullBtn->setText(("全屏"));

    QLabel *spliterLabel4 = new QLabel(this);
    spliterLabel4->setText("|");
    scaleUpBtn = new QPushButton(this);
    scaleUpBtn->setIconSize(QSize(20,20));
    scaleUpBtn->setText(("+"));
    scaleUpBtn->setObjectName(("scaleUpBtn"));

    QLabel *spliterLabel3 = new QLabel(this);
    spliterLabel3->setText("|");
    scaleDownBtn = new QPushButton(this);
    scaleDownBtn->setIconSize(QSize(20,20));
    scaleDownBtn->setText(("-"));
    scaleDownBtn->setObjectName(("scaleDownBtn"));

    QLabel *spliterLabel = new QLabel(this);
    spliterLabel->setText("|");
    preBtn = new QPushButton(this);
    preBtn->setObjectName(("preBtn"));
    preBtn->setText((" < "));
    progressLabel = new QLabel(this);
    progressLabel->setObjectName(("progressLabel"));
    progressLabel->setText(("1/3"));
    nextBtn = new QPushButton(this);
    nextBtn->setObjectName(("nextBtn"));
    nextBtn->setText((" > "));
    QLabel *spliterLabel1 = new QLabel(this);
    spliterLabel1->setText("|");
    exitBtn = new QPushButton(this);
    exitBtn->setObjectName(("exitBtn"));
    exitBtn->setText(("结束讲评"));
    horizontalLayout->addWidget(fullBtn);
    horizontalLayout->addStretch(1);
    horizontalLayout->addWidget(spliterLabel);
    horizontalLayout->addWidget(scaleUpBtn);
    horizontalLayout->addStretch(1);
    horizontalLayout->addWidget(spliterLabel3);
    horizontalLayout->addWidget(scaleDownBtn);
    horizontalLayout->addStretch(1);
    horizontalLayout->addWidget(spliterLabel4);
    horizontalLayout->addStretch(1);
    horizontalLayout->addWidget(preBtn);
    horizontalLayout->addStretch(1);
    horizontalLayout->addWidget(progressLabel);
    horizontalLayout->addStretch(1);
    horizontalLayout->addWidget(nextBtn);
    horizontalLayout->addStretch(1);
    horizontalLayout->addWidget(spliterLabel1);
    horizontalLayout->addStretch(1);
    horizontalLayout->addWidget(exitBtn);
    setLayout(horizontalLayout);
    QObject::connect(fullBtn, SIGNAL(clicked()), this, SLOT(on_clicked_fullBtn()));
}

PageBoxToolBar::~PageBoxToolBar()
{
}

void PageBoxToolBar::updateProgressText(QString progress)
{
    this->progressLabel->setText(progress);
}

void PageBoxToolBar::setIsFull(bool isfull)
{
    this->isFullState = isfull;
    if(!isfull){
       fullBtn->setIcon(QIcon(":/icon/icon/enter_full_screen_btn_icon.svg"));
       fullBtn->setText(("全屏"));
    } else {
       fullBtn->setIcon(QIcon(":/icon/icon/exit_full_screen_btn_icon.svg"));
       fullBtn->setText(("缩小"));
    }
}

void PageBoxToolBar::on_clicked_fullBtn()
{
    emit fullStateChanged(!isFullState);
    setIsFull(!isFullState);
}
