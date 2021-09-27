#include "mindnodeview.h"
#include "mindnode.h"
#include "mindtextedititem.h"
#include "mindviewstyle.h"

#include <QTextCursor>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

MindTextEditItem::MindTextEditItem(QGraphicsItem *parent)
    : QGraphicsTextItem(parent)
{
    hide();
    setTextInteractionFlags(Qt::TextEditorInteraction);
}

void MindTextEditItem::attachTo(MindNodeView *nodeView, QPointF const & pos)
{
    nodeView_ = nodeView;
    setFont(nodeView->style()->textFont());
    setDefaultTextColor(nodeView->style()->textColor());
    setPlainText(nodeView->node()->title);
    setPos(nodeView->pos() + nodeView->style()->textOffset());
    QTextCursor textCursor = this->textCursor();
    if (pos.isNull()) {
        textCursor.select(QTextCursor::Document);
    } else {
        textCursor.setPosition(document()->documentLayout()->hitTest(pos - this->pos(), Qt::FuzzyHit));
    }
    setTextCursor(textCursor);
    qreal m = document()->documentMargin();
    moveBy(-m, -m);
    nodeView_->setEditing(true);
    show();
    setFocus();
}

void MindTextEditItem::focusOutEvent(QFocusEvent *event)
{
    QGraphicsTextItem::focusOutEvent(event);
    finish();
}

void MindTextEditItem::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return) {
    } else {
        QGraphicsTextItem::keyPressEvent(event);
    }
}

void MindTextEditItem::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return) {
        finish();
    } else if (event->key() == Qt::Key_Escape) {
        finish(true);
    } else {
        QGraphicsTextItem::keyPressEvent(event);
    }
}

void MindTextEditItem::finish(bool cancel)
{
    hide();
    if (nodeView_ == nullptr)
        return;
    nodeView_->setEditing(false);
    nodeView_->setTitle(toPlainText());
    nodeView_ = nullptr;
    if (!cancel)
        emit editEnded();
}
