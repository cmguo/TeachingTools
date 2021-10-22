#include "strokesrecordhelper.h"

#include <Windows/Input/mousedevice.h>
#include <Windows/Input/stylusdevice.h>

#include <core/resourcerecord.h>

#include <Windows/Controls/inkevents.h>

StrokesRecordHelper::StrokesRecordHelper(InkCanvas *ink)
    : QObject(ink)
    , ink_(ink)
{
    ink->AddHandler(Mouse::MouseDownEvent, RoutedEventHandlerT<
                    StrokesRecordHelper, RoutedEventArgs, &StrokesRecordHelper::handle>(this));
    ink->AddHandler(Mouse::MouseUpEvent, RoutedEventHandlerT<
                    StrokesRecordHelper, RoutedEventArgs, &StrokesRecordHelper::handle>(this));
    ink->AddHandler(Stylus::StylusDownEvent, RoutedEventHandlerT<
                    StrokesRecordHelper, RoutedEventArgs, &StrokesRecordHelper::handle>(this));
    ink->AddHandler(Stylus::StylusUpEvent, RoutedEventHandlerT<
                    StrokesRecordHelper, RoutedEventArgs, &StrokesRecordHelper::handle>(this));
    QObject::connect(ink, &InkCanvas::StrokesReplaced,
                     this, &StrokesRecordHelper::strokesReplaced);
    QObject::connect(ink, &InkCanvas::SelectionMoving,
                     this, &StrokesRecordHelper::selectionChanging);
    QObject::connect(ink, &InkCanvas::SelectionResizing,
                     this, &StrokesRecordHelper::selectionChanging);
    QObject::connect(ink, &InkCanvas::SelectionMoved,
                     this, &StrokesRecordHelper::selectionChanged);
    QObject::connect(ink, &InkCanvas::SelectionResized,
                     this, &StrokesRecordHelper::selectionChanged);
    ink_->Strokes()->setProperty("StrokesRecordHelper", QVariant::fromValue<QObject*>(this));
    QObject::connect(ink_->Strokes().get(), &StrokeCollection::StrokesChanged,
                     this, &StrokesRecordHelper::strokesChanged);
}

void StrokesRecordHelper::handle(RoutedEventArgs &args)
{
    if (&args.GetRoutedEvent() == &Mouse::MouseUpEvent
            || &args.GetRoutedEvent() == &Stylus::StylusUpEvent) {
        if (record_) {
            RecordMergeScope rs(ink_);
            rs.add(record_);
        }
    } else {
        delete record_;
    }
    record_ = nullptr;
    mergeRecord_ = nullptr;
}

class StrokesChangedRecord : public ResourceRecord
{
public:
    StrokesChangedRecord(SharedPointer<StrokeCollection> strokes, StrokeCollectionChangedEventArgs &args)
        : strokes_(strokes)
        , args_(args)
    {
        setInfo("StrokesChangedRecord");
    }
public:
    virtual void redo() override
    {
        remove(args_.Index(), args_.Removed());
        insert(args_.Index(), args_.Added());
    }
    virtual void undo() override
    {
        remove(args_.Index(), args_.Added());
        insert(args_.Index(), args_.Removed());
    }
private:
    void insert(int index, SharedPointer<StrokeCollection> strokes)
    {
        for (auto s : *strokes)
            strokes_->Insert(index++, s);
    }
    void remove(int index, SharedPointer<StrokeCollection> strokes)
    {
        for (auto s : *strokes.get()) {
            assert((*strokes_)[index] == s);
            strokes_->RemoveAt(index);
        }
    }
private:
    SharedPointer<StrokeCollection> strokes_;
    StrokeCollectionChangedEventArgs args_;
};

void StrokesRecordHelper::strokesChanged(StrokeCollectionChangedEventArgs &args)
{
    RecordMergeScope rs(ink_);
    if (rs) {
        ResourceRecord * r = new StrokesChangedRecord(ink_->Strokes(), args);
        if (sender()->parent()) {
            QObject * real = sender()->parent()->property("StrokesRecordHelper")
                    .value<QObject*>();
            StrokesRecordHelper * helper = static_cast<StrokesRecordHelper*>(real);
            if (real)
                helper->record_ = ResourceRecordSet::merge(helper->record_, helper->mergeRecord_, r);
        } else {
            record_ = ResourceRecordSet::merge(record_, mergeRecord_, r);
        }
    }
}

void StrokesRecordHelper::strokesReplaced(InkCanvasStrokesReplacedEventArgs &e)
{
    if (e.PreviousStrokes())
        e.PreviousStrokes().get()->disconnect(this);
    e.NewStrokes()->setProperty("StrokesRecordHelper", QVariant::fromValue<QObject*>(this));
    QObject::connect(e.NewStrokes().get(), &StrokeCollection::StrokesChanged,
                     this, &StrokesRecordHelper::strokesChanged);
}

void StrokesRecordHelper::selectionChanging(InkCanvasSelectionEditingEventArgs &e)
{
    change_ = &e;
}

class StrokesEditRecord : public ResourceRecord
{
public:
    StrokesEditRecord(SharedPointer<StrokeCollection> strokes, InkCanvasSelectionEditingEventArgs &args)
        : strokes_(strokes)
        , args_(args)
    {
        setInfo("StrokesEditRecord");
    }
public:
    virtual void redo() override
    {
        strokes_->Transform(MapRectToRect(args_.NewRectangle(), args_.OldRectangle()), false);
    }
    virtual void undo() override
    {
        strokes_->Transform(MapRectToRect(args_.OldRectangle(), args_.NewRectangle()), false);
    }
private:
    Matrix MapRectToRect(Rect const & target, Rect const & source)
    {
        double m11 = target.Width() / source.Width();
        double dx = target.Left() - m11 * source.Left();
        double m22 = target.Height() / source.Height();
        double dy = target.Top() - m22 * source.Top();
        return Matrix(m11, 0, 0, m22, dx, dy);
    }
private:
    SharedPointer<StrokeCollection> strokes_;
    InkCanvasSelectionEditingEventArgs args_;
};

void StrokesRecordHelper::selectionChanged(EventArgs &)
{
    RecordMergeScope rs(ink_);
    if (rs)
        rs.add(new StrokesEditRecord(ink_->GetSelectedStrokes(), *change_));
    change_ = nullptr;
}
