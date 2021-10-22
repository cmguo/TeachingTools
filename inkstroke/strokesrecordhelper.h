#ifndef STROKESRECORDHELPER_H
#define STROKESRECORDHELPER_H

#include <Windows/routedeventargs.h>
#include <Windows/Ink/events.h>
#include <Windows/Controls/inkcanvas.h>

#include <QObject>

INKCANVAS_USE_NAMESPACE

class ResourceRecord;
class MergeRecord;

class StrokesRecordHelper : public QObject
{
public:
    StrokesRecordHelper(InkCanvas *ink);
private:
    void handle(RoutedEventArgs & args);
    void strokesChanged(StrokeCollectionChangedEventArgs & args);
    void strokesReplaced(InkCanvasStrokesReplacedEventArgs &e);
    void selectionChanging(InkCanvasSelectionEditingEventArgs &e);
    void selectionChanged(EventArgs &e);
private:
    InkCanvas *ink_ = nullptr;
    ResourceRecord * record_ = nullptr;
    MergeRecord * mergeRecord_ = nullptr;
    InkCanvasSelectionEditingEventArgs * change_ = nullptr;
};

#endif // STROKESRECORDHELPER_H
