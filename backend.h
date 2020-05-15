#ifndef BACKEND_H
#define BACKEND_H

#include "theoreticalmodelparameters.h"
#include <QObject>

namespace QtCharts {
    class QAbstractSeries;
}

class Backend : public QObject
{
    Q_OBJECT
public:
    explicit Backend(QObject *parent = nullptr);

public slots:
    void generateTheoreticalProfile(double b, double c, int type, double precision, int cutoffMoment, QtCharts::QAbstractSeries *series);
};

#endif // BACKEND_H
