#ifndef OCR_H
#define OCR_H

#include <QBuffer>
#include <QGraphicsItem>

namespace tesseract {
    class TessBaseAPI;
}

class TesseractOcr
{
public:
    TesseractOcr();

    ~TesseractOcr();

public:
    QString detect(QString const & file);

    QString detect(QGraphicsItem * item);

private:
    tesseract::TessBaseAPI * api_;
};

#endif // OCR_H
