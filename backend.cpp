#include "backend.h"
#include "dropgenerator.h"
#include <QXYSeries>
#include <QChart>
#include <QDebug>

using namespace QtCharts;

Backend::Backend(QObject *parent)
    : QObject(parent)
{
}

void Backend::generateTheoreticalProfile(double b, double c,
                                         int type,
                                         double precision, int cutoffMoment,
                                         QAbstractSeries *series)
{
    QXYSeries *s = static_cast<QXYSeries *>(series);
    TheoreticalModelParameters params(TheoreticalModelParameters::DropType(type), b, c, precision, cutoffMoment);
    auto v = DropGenerator::generateTheoreticalModel(params);
    s->replace(v);

    // Adjust axes to fit

    QChart *chart = s->chart();
    chart->createDefaultAxes();

    QList<QAbstractAxis*> axesX= chart->axes(Qt::Horizontal);
    QList<QAbstractAxis*> axesY= chart->axes(Qt::Vertical);

    double maxX = 0.0;
    double maxY = 0.0;
    for (QAbstractSeries *series : chart->series()) {
        QXYSeries *s = static_cast<QXYSeries *>(series);
        const auto &points = s->points();
        for (auto p : points) {
            if (p.x() > maxX)
                maxX = p.x();
            if (p.y() > maxY)
                maxY = p.y();
        }
    }

    if (maxX == 0.0)
        maxX = 1.0;
    if (maxY == 0.0)
        maxY = 1.0;

    for(auto axisX : axesX)
        axisX->setRange(0, maxX);
    for(auto axisY : axesY)
        axisY->setRange(0, maxY);
}
