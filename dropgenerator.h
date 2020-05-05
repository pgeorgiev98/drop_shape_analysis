#ifndef DROPGENERATOR_H
#define DROPGENERATOR_H

#include <QObject>
#include <QVector>
#include <QPointF>

class DropGenerator : public QObject
{
    Q_OBJECT
public:
    explicit DropGenerator(QObject *parent = nullptr);

    enum class DropType {PENDANT, SPINNING};

    struct TheoreticalModelParameters
    {
        DropType dropType;
        double b, c;
        double precision;

        TheoreticalModelParameters(DropType dropType, double b, double c, double precision)
            : dropType(dropType), b(b), c(c), precision(precision)
        {}
    };

    static QVector<QPointF> generateTheoreticalModel(double b, double c, DropType type, double precision);
    static QVector<QPointF> generateError(const QVector<QPointF> &theoretical, const QVector<QPointF> &experimental);
    static double calculateError(const QVector<QPointF> &error);
    static double calculateError(const QVector<QPointF> &theoretical, const QVector<QPointF> &experimental);
    static TheoreticalModelParameters minimizeError(const QVector<QPointF> &experimental, DropType dropType, double precision);
};

#endif // DROPGENERATOR_H
