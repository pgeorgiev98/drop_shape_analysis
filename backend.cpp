#include "backend.h"
#include "dropgenerator.h"
#include <QXYSeries>
#include <QChart>
#include <QFile>
#include <QTextStream>
#include <QVector>
#include <QPointF>
#include <QDebug>

using namespace QtCharts;

Backend::Backend(QObject *parent)
    : QObject(parent)
{
}

static void adjustAxes(QChart *chart)
{
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

void Backend::generateTheoreticalProfile(double b, double c,
                                         int type,
                                         double precision, int cutoffMoment,
                                         QAbstractSeries *series)
{
    QXYSeries *s = static_cast<QXYSeries *>(series);
    TheoreticalModelParameters params(TheoreticalModelParameters::DropType(type), b, c, precision, cutoffMoment);
    auto v = DropGenerator::generateTheoreticalModel(params);
    s->replace(v);

    QChart *chart = s->chart();
    adjustAxes(chart);
}

bool Backend::loadExperimentalFromTextFile(QString fileUrl, QtCharts::QAbstractSeries *series)
{
    QString filePath = QUrl(fileUrl).isLocalFile() ? QUrl(fileUrl).toLocalFile() : fileUrl;
    static auto expectChar = [](QTextStream &in, char c) -> bool {
        char ch;
        in >> ch;
        return c == ch;
    };

    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) {
        m_lastError = QString("Failed to open file %1: %2").arg(filePath).arg(f.errorString());
        return false;
    }
    QTextStream in(&f);
    QVector<QPointF> points;
    try {
        if (!expectChar(in, '{'))
            throw "";
        while (!in.atEnd()) {
            if (!expectChar(in, '{'))
                throw "";
            double x, y;
            in >> x;
            if (!expectChar(in, ','))
                throw "";
            in >> y;
            if (!expectChar(in, '}'))
                throw "";
            points.append(QPointF(x, y));
            if (!expectChar(in, ','))
                break;
        }
    } catch(...) {
        m_lastError = "File format error";
        return false;
    }

    QXYSeries *s = static_cast<QXYSeries *>(series);
    s->replace(points);
    // TODO: update cutoff

    QChart *chart = s->chart();
    adjustAxes(chart);

    return true;
}

bool Backend::loadExperimentalFromImageFile(QString fileUrl, QtCharts::QAbstractSeries *series)
{
    QString filePath = QUrl(fileUrl).isLocalFile() ? QUrl(fileUrl).toLocalFile() : fileUrl;
    auto drop = DropGenerator::generateModelFromImage(filePath);

    QXYSeries *s = static_cast<QXYSeries *>(series);
    s->replace(drop);
    // TODO: update cutoff

    QChart *chart = s->chart();
    adjustAxes(chart);

    return true;
}
