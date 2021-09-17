#include "tesseractocr.h"

#ifdef HAS_TESSERACT
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#endif

#include <QDir>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QDebug>

TesseractOcr::TesseractOcr()
{
#ifdef HAS_TESSERACT
    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    // Initialize tesseract-ocr with English, without specifying tessdata path
    if (api->Init(QDir::currentPath().toUtf8() + "/tessdata_best", "chi_sim")) {
        fprintf(stderr, "Could not initialize tesseract.\n");
        exit(1);
    }
    api->SetPageSegMode(tesseract::PSM_SINGLE_CHAR);
    api_ = api;
#endif
}

TesseractOcr::~TesseractOcr()
{
#ifdef HAS_TESSERACT
    api_->End();
    delete api_;
#endif
}

QString TesseractOcr::detect(QString const & file)
{
    QString out;
#ifdef HAS_TESSERACT
    Pix *image = pixRead(file.toUtf8());
    api_->SetImage(image);
    // Get OCR result
    char * outText = api_->GetUTF8Text();
    out = outText;
    qDebug() << "OCR output:" << out;

//    delete [] outText; // TODO: CRT
    pixDestroy(&image);
#endif
    return out;
}

static QImage toImage(QGraphicsItem * item, QSize size);

QString TesseractOcr::detect(QGraphicsItem *item)
{
    QBuffer imageData;
    imageData.open(QBuffer::ReadWrite);
    toImage(item, {}).save(&imageData, "png");
    imageData.seek(0);

//    QFile file("ocr2.png");
//    file.open(QFile::WriteOnly);
//    toImage(item, {80, 80}).save(&file, "png");
//    file.close();

//    return detect("ocr2.png");

    QString out;

#ifdef HAS_TESSERACT
    // Open input image with leptonica library
    Pix *image = pixReadMem(reinterpret_cast<l_uint8 const*>(imageData.data().data()), imageData.size());
    api_->SetImage(image);
    // Get OCR result
    char * outText = api_->GetUTF8Text();
    out = outText;
    qDebug() << "OCR output:" << out;

//    api_->SetVariable("save_blob_choices", "T");
//    api_->Recognize(NULL);

//    tesseract::ResultIterator* ri = api_->GetIterator();
//    tesseract::PageIteratorLevel level = tesseract::RIL_SYMBOL;
//    if(ri != 0) {
//        do {
//            const char* symbol = ri->GetUTF8Text(level);
//            float conf = ri->Confidence(level);
//            if(symbol != 0) {
//                qDebug() << "symbol" << symbol << "conf:" << conf;
//                tesseract::ChoiceIterator ci(*ri);
//                do {
//                    const char* choice = ci.GetUTF8Text();
//                    qDebug() << choice << "conf:" << ci.Confidence();
//                } while(ci.Next());
//            }
////            delete[] symbol;
//        } while((ri->Next(level)));
//    }

//    api_->SetVariable("lstm_choice_mode", "2");
//    api_->Recognize(0);
//    tesseract::PageIteratorLevel level = tesseract::RIL_WORD;
//    tesseract::ResultIterator* res_it = api_->GetIterator();
//    // Get confidence level for alternative symbol choices. Code is based on
//    // https://github.com/tesseract-ocr/tesseract/blob/master/src/api/hocrrenderer.cpp#L325-L344
//    std::vector<std::vector<std::pair<const char*, float>>>* choiceMap = nullptr;
//    if (res_it != 0) {
//        do {
//            const char* word;
//            float conf;
//            int x1, y1, x2, y2, tcnt = 1, gcnt = 1, wcnt = 0;
//            res_it->BoundingBox(level, &x1, &y1, &x2, &y2);
//            choiceMap = res_it->GetBestLSTMSymbolChoices();
//            for (auto timestep : *choiceMap) {
//                if (timestep.size() > 0) {
//                    for (auto & j : timestep) {
//                        conf = int(j.second * 100);
//                        word =  j.first;
//                        qDebug() << wcnt << "symbol: '" << word << "';  conf: " << conf << "; BoundingBox:";
//                        gcnt++;
//                    }
//                    tcnt++;
//                }
//                wcnt++;
//            }
//        } while (res_it->Next(level));
//    }

    //    delete [] outText; // TODO: CRT
    pixDestroy(&image);
#endif

    return out;

}

static void paintItem(QPainter & painter, QGraphicsItem * item, QStyleOptionGraphicsItem & option)
{
    item->paint(&painter, &option, nullptr);
    for (QGraphicsItem * c : item->childItems()) {
        paintItem(painter, c, option);
    }
}

static QImage toImage(QGraphicsItem * item, QSize size)
{
    QPointF c = item->boundingRect().center() * 2;
    if (size.isEmpty()) {
        size = QSizeF(c.x(), c.y()).toSize();
    }
    QPointF scale(size.width() / c.x(), size.height() / c.y());
    QImage image(size, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setBrush(Qt::black);
    painter.setTransform(QTransform::fromScale(scale.x(), scale.y()));
    QStyleOptionGraphicsItem option;
    paintItem(painter, item, option);
    painter.end();
    return image;
}
