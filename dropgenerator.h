#ifndef DROPGENERATOR_H
#define DROPGENERATOR_H

#include "theoreticalmodelparameters.h"
#include <QMetaType>
#include <QVector>
#include <QPointF>

class DropGenerator
{
public:
    DropGenerator() = delete;

    static QVector<QPointF> generateTheoreticalModel(TheoreticalModelParameters parameters);
    static QVector<QPointF> generateError(const QVector<QPointF> &theoretical, const QVector<QPointF> &experimental);
    static double calculateError(const QVector<QPointF> &error);
    static double calculateError(const QVector<QPointF> &theoretical, const QVector<QPointF> &experimental);
    static QVector<QPointF> generateModelFromImage(const QString fileName);
};

#endif // DROPGENERATOR_H
