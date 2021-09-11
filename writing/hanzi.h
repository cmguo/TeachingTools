#ifndef HANZI_H
#define HANZI_H

#include <QObject>
#include <QPainterPath>
#include <QTransform>
#include <QVector>

class Hanzi : public QObject
{
    Q_OBJECT
public:
    static QRect VIEW_BOX;

    static QTransform TRANSFORM;

    enum EtymologyType {
        ideographic, pictographic, pictophonetic
    };

    Q_ENUM(EtymologyType)

    struct Etymology {
        EtymologyType type;
        QString hint;
    };

    struct DictionaryItem {
        /*
         *  The Unicode character for this glyph.
         * Required.
         */
        QChar character;
        /*
         * A String definition targeted towards second-language learners.
         * Optional.
         */
        QString definition;
        /*
         * A comma-separated list of String pronunciations of this character.
         * Required, but may be empty.
         */
        QVector<QString> pinyin;
        /*
         * An [Ideograph Description Sequence]
         * decomposition of the character.
         * Required, but invalid if it starts with a full-width question mark '？'.
         */
        QString decomposition;
        /*
         * An etymology for the character.
         * This field may be null.
         * If present, it will always have a "type" field,
         *   which will be one of "ideographic", "pictographic", or "pictophonetic".
         * If the type is one of the first two options, then the etymology will always
         *   include a string "hint" field explaining its formation.
         */
        Etymology etymology;
        /*
         * Unicode primary radical for this character.
         * Required.
         */
        QString radical;
        /*
         * A list of mappings from strokes of this character to strokes of its
         *   components, as indexed in its decomposition tree.
         * Any given entry in this list may be null.
         * If an entry is not null, it will be a list of indices corresponding
         *   to a path down the decomposition tree.
         */
        QVector<QVector<int>> matches;
    };

    struct Graphic {
        /*
         *  The Unicode character for this glyph.
         * Required.
         */
        QChar character;
        /*
         * List of SVG path data for each stroke of this character, ordered by
         *   proper stroke order.
         * Each stroke is laid out on a 1024x1024 size coordinate system where:
         *   - The upper-left corner is at position (0, 900).
         *   - The lower-right corner is at position (1024, -124).
         */
        QVector<QByteArray> strokes;
        /*
         * A list of stroke medians, in the same coordinate system as the SVG
         *   paths above.
         * These medians can be used to produce a rough stroke-order animation,
         *   although it is a bit tricky.
         * Each median is a list of pairs of integers.
         * This list will be as long as the strokes list.
         */
        QVector<QVector<QPair<int, int>>> medians;
    };

public:
    explicit Hanzi(QChar character, QObject *parent = nullptr);

public:
    QVector<QPainterPath> strokes() const;

    QPainterPath medians(int index);

    QVector<int> radical() const;

    QVector<QByteArray> pinyin() const;

private:
    static void parseDict(DictionaryItem & dict, QByteArray const & json);

    static void parseGraphic(Graphic & graphic, QByteArray const & json);

    static QPainterPath parsePath(QByteArray const & storke);

    static QByteArray json(QByteArray type, QChar c);

private:
    DictionaryItem dict_;
    Graphic graphic_;
};

#endif // HANZI_H
