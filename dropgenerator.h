#ifndef DROPGENERATOR_H
#define DROPGENERATOR_H

#include <QMetaType>
#include <QVector>
#include <QPointF>

class DropGenerator
{
public:
    DropGenerator() = delete;

    enum class DropType {PENDANT, SPINNING};

    struct TheoreticalModelParameters
    {
        DropType dropType;
        double b, c;
        double precision;
        int cutoffMoment;

        TheoreticalModelParameters(DropType dropType, double b, double c, double precision, int cutoffMoment)
            : dropType(dropType), b(b), c(c), precision(precision), cutoffMoment(cutoffMoment)
        {}
        TheoreticalModelParameters()
        {}
    };

    static QVector<QPointF> generateTheoreticalModel(double b, double c, DropType type, double precision, int cutoffMoment);
    static QVector<QPointF> generateError(const QVector<QPointF> &theoretical, const QVector<QPointF> &experimental);
    static double calculateError(const QVector<QPointF> &error);
    static double calculateError(const QVector<QPointF> &theoretical, const QVector<QPointF> &experimental);
    static QVector<QPointF> generateModelFromImage(const QString fileName);
};

Q_DECLARE_METATYPE(DropGenerator::TheoreticalModelParameters)

#endif // DROPGENERATOR_H
