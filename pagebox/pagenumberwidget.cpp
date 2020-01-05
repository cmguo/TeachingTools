#include "pagenumberwidget.h"

#include <core/toolbutton.h>

#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>

PageNumberWidget::PageNumberWidget(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    preBtn = new QPushButton(this);
    preBtn->setObjectName(("preBtn"));
    preBtn->setText((" < "));
    progressLabel = new QLabel(this);
    progressLabel->setObjectName(("progressLabel"));
    progressLabel->setText(("0/0"));
    nextBtn = new QPushButton(this);
    nextBtn->setObjectName(("nextBtn"));
    nextBtn->setText((" > "));
    layout->addWidget(preBtn);
    layout->addWidget(progressLabel);
    layout->addWidget(nextBtn);
    setLayout(layout);
    QObject::connect(preBtn, &QPushButton::clicked, this, &PageNumberWidget::buttonClicked);
    QObject::connect(nextBtn, &QPushButton::clicked, this, &PageNumberWidget::buttonClicked);
}

PageNumberWidget::~PageNumberWidget()
{
    if (toolButton_)
        delete toolButton_;
}

void PageNumberWidget::setTotal(int n)
{
    total_ = n;
    setNumber(total_ == 0 ? -1 : 0);
}

void PageNumberWidget::setNumber(int n)
{
    if (no_ == n)
        return;
    no_ = n;
    progressLabel->setText(QString("%1/%2").arg(no_ + 1).arg(total_));
}

ToolButton *PageNumberWidget::toolButton()
{
    if (toolButton_ == nullptr)
        toolButton_ = new ToolButton({"pages", "",
                                      ToolButton::CustomWidget, QVariant::fromValue(this)});
    return toolButton_;
}

void PageNumberWidget::buttonClicked()
{
    if (sender() == preBtn) {
        if (no_ > 0) {
            setNumber(no_ - 1);
        }
    } else if (sender() == nextBtn) {
        if (no_ + 1 < total_) {
            setNumber(no_ + 1);
        }
    }
    emit pageNumberChanged(no_);
}
