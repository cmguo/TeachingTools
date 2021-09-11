#ifndef WORDSCONTROL_H
#define WORDSCONTROL_H

#include "core/control.h"

class QPainterPath;

class HanziControl : public Control
{
    Q_OBJECT

public:
    Q_INVOKABLE HanziControl(ResourceView *res);

public slots:
    void next();

    void anim();

    void radical();

    void pinyin();

public:

protected:
    virtual ControlView *create(ControlView *parent) override;

    virtual void timerEvent(QTimerEvent * event) override;

private:
    int timer_;
};

#endif // WORDSCONTROL_H
