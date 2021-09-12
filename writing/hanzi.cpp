#include "hanzi.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMetaEnum>

QRect Hanzi::VIEW_BOX = {0, 0, 1024, 1024};
QTransform Hanzi::TRANSFORM(1, 0, 0, -1, 0, 900);

static QVector<int> makeNull()
{
    QVector<int> v(1, 0);
    v.clear();
    return v;
}

static QVector<int> null = makeNull();

Hanzi::Hanzi(QChar character, QObject *parent)
    : QObject(parent)
{
    parseDict(dict_, json("dictionary", character));
    parseGraphic(graphic_, json("graphics", character));
}

QVector<QPainterPath> Hanzi::strokes() const
{
    QVector<QPainterPath> phs;
    for (QByteArray const & stroke : graphic_.strokes) {
        phs.append(parsePath(stroke));
    }
    return phs;
}

QPainterPath Hanzi::medians(int index)
{
    QPainterPath ph;
    for (auto & pt : graphic_.medians[index]) {
        if (ph.elementCount() == 0)
            ph.moveTo(pt.first, pt.second);
        else
            ph.lineTo(pt.first, pt.second);
    }
    return ph;
}

QVector<int> Hanzi::radical() const
{
    static QString parts2 = "⿱⿰⿹⿸⿺⿵⿶⿷⿴⿻";
    static QString parts3 = "⿳⿲";
    QVector<int> stack(1, 1);
    QVector<int> position(1, 0);
    int i = 0;
    while (!stack.isEmpty()) {
        QChar c = dict_.decomposition[i++];
        if (parts2.contains(c)) {
            stack.append(2);
            position.append(0);
        } else if (parts3.contains(c)) {
            stack.append(3);
            position.append(0);
        } else if (c == dict_.radical) {
            break;
        } else {
            while (!stack.isEmpty() && ++position.back() == stack.back()) {
                stack.pop_back();
                position.pop_back();
            }
        }
    }
    QVector<int> indexes;
    if (position.isEmpty()) {
        if (dict_.radical == dict_.character) {
            for (int i = 0; i < dict_.matches.size(); ++i)
                indexes.append(i);
            return indexes;
        }
    } else {
        position.pop_front();
    }
    for (int i = 0; i < dict_.matches.size(); ++i) {
        if (!dict_.matches[i].isSharedWith(null) && dict_.matches[i] == position)
            indexes.append(i);
    }
    return indexes;
}

QVector<QByteArray> Hanzi::pinyin() const
{
    static QList<QByteArray> shengmu = // 声母
            QByteArray("b,p,m,f,d,t,n,l,g,k,h,j,q,x,zh,ch,sh,r,z,c,s,y,w")
            .split(',');
    static QByteArray jqx = "jqx";
    static QByteArray danyunmu = "aoeiuv"; // 单韵母
    static QString danyunmu2 = // 单韵母，带声调
            "aāáǎàoōóǒòeēéěèiīíǐìuūúǔùüǖǘǚǜ";
    static QList<QByteArray> fuyunmu = // 复韵母
            QByteArray("ai,ei,ui,ao,ou,iu,ie,ve,er,an,en,in,un,vn,ang,eng,ing,ong")
            .split(',');
    static QList<QByteArray> zhengti = // 整体认读音节
            QByteArray("zhi,chi,shi,ri,zi,ci,si,yi,wu,yu,ye,yue,yuan,yin,yun,ying")
            .split(',');

    QVector<QByteArray> results;
    for (auto & pinyin : dict_.pinyin) {
        QByteArray shengmu; // 声母
        QByteArray yinjie; // 音节
        char shengdiao = 0;
        for (auto c : pinyin) {
            int index = danyunmu2.indexOf(c);
            char s = 0;
            if (index < 0) {
                s = c.toLatin1();
                index = danyunmu.indexOf(s);
            } else {
                s = danyunmu.at(index / 5);
                if (index % 5)
                    shengdiao = index % 5;
            }
            if (shengmu.isEmpty() && index >= 0)
                shengmu = yinjie;
            yinjie.append(s);
        }
        bool zt = zhengti.contains(yinjie);
        QByteArray yunmu = yinjie.mid(shengmu.size());
        if (shengmu.size() == 1 && jqx.contains(shengmu.front())
                && yunmu.front() == 'u')
            yunmu.front() = 'v';
        bool single = (yunmu.size() == 1) || fuyunmu.contains(yunmu);
        if (shengdiao) {
            //yinjie = yinjie.replace('v', "uu");
            yinjie.append('0' + shengdiao);
            yunmu.append('0' + shengdiao);
        }
        if (!zt) {
            results.append(shengmu);
            if (!single) {
                results.append(yunmu.left(1));
                results.append(yunmu.mid(1));
            } else {
                results.append(yunmu);
            }
        }
        results.append(yinjie);
    }
    return results;
}

void Hanzi::parseDict(Hanzi::DictionaryItem &dict, const QByteArray &json)
{
    QJsonDocument d = QJsonDocument::fromJson(json);
    QJsonObject j = d.object();
    dict.character = j.value("character").toString().front();
    dict.definition = j.value("definition").toString();
    for (QJsonValue const v : j.value("pinyin").toArray())
        dict.pinyin.append(v.toString());
    dict.decomposition = j.value("decomposition").toString();
    QJsonObject e = j.value("etymology").toObject();
    dict.etymology.type = static_cast<EtymologyType>(QMetaEnum::fromType<EtymologyType>()
            .keyToValue(e.value("type").toString().toUtf8()));
    dict.etymology.hint = e.value("hint").toString();
    dict.definition = j.value("definition").toString();
    dict.radical = j.value("radical").toString();
    for (QJsonValue const v : j.value("matches").toArray()) {
        QVector<int> array;
        if (v.isNull()) {
            array = null;
        } else {
            for (QJsonValue const vv : v.toArray()) {
                array.append(vv.toInt());
            }
        }
        dict.matches.append(array);
    }
}

void Hanzi::parseGraphic(Hanzi::Graphic &graphic, const QByteArray &json)
{
    QJsonDocument d = QJsonDocument::fromJson(json);
    QJsonObject j = d.object();
    graphic.character = j.value("character").toString().front();
    for (QJsonValue const v : j.value("strokes").toArray()) {
        graphic.strokes.append(v.toString().toUtf8());
    }
    for (QJsonValue const v : j.value("medians").toArray()) {
        QVector<QPair<int, int>> array;
        for (QJsonValue const vv : v.toArray()) {
            QJsonArray pt = vv.toArray();
            array.append({pt[0].toInt(), pt[1].toInt()});
        }
        graphic.medians.append(array);
    }
}

QPainterPath Hanzi::parsePath(const QByteArray &stroke)
{
    QPainterPath ph;
    auto tokens = stroke.split(' ');
    int i = 0;
    while (i < tokens.size()) {
        char t = tokens[i++].front();
        if (t == 'Z') {
            ph.closeSubpath();
            break;
        }
        int x = tokens[i++].toInt();
        int y = tokens[i++].toInt();
        switch (t) {
        case 'M':
            ph.moveTo(x, y);
            break;
        case 'L':
            ph.lineTo(x, y);
            break;
        case 'C': {
            int x1 = tokens[i++].toInt();
            int y1 = tokens[i++].toInt();
            int x2 = tokens[i++].toInt();
            int y2 = tokens[i++].toInt();
            ph.cubicTo(x, y, x1, y1, x2, y2);
        }
            break;
        case 'Q': {
            int x1 = tokens[i++].toInt();
            int y1 = tokens[i++].toInt();
            ph.quadTo(x, y, x1, y1);
        }
            break;
        }
    }
    return ph;
}

QByteArray Hanzi::json(QByteArray type, QChar c)
{
    QFile file("makemeahanzi/" + type + ".txt");
    file.open(QFile::ReadOnly);
    QByteArray bytes = QString(c).toUtf8() + "\"";
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        if (line.mid(14).startsWith(bytes)) {
            return line;
        }
    }
    return nullptr;
}
