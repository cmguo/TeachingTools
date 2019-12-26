#ifndef CONTROLS_H
#define CONTROLS_H

#include "writing/writinggridcontrol.h"
#include "pagebox/pageboxcontrol.h"
#include "inkstroke/inkstrokecontrol.h"

REGISTER_CONTROL(PageBoxControl, "pages")
REGISTER_CONTROL(WritingGridControl, "writinggrid");
REGISTER_CONTROL(InkStrokeControl, "inkstroke");

#include "inkstroke/inkstrokes.h"

REGISTER_RESOURCE_VIEW(InkStrokes, "inkstroke");

#endif // CONTROLS_H
