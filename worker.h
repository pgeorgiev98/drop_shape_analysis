#ifndef WORKER_H
#define WORKER_H

#include "theoreticalmodelparameters.h"
#include "dropgenerator.h"
#include <QObject>

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr)
        : QObject(parent)
    {}

public slots:
    void doWork(const QVector<QPointF> &experimental, TheoreticalModelParameters::DropType dropType, double precision, int cutoffMoment);

signals:
    void finished(TheoreticalModelParameters parameters);
    void progressChanged(double progress);
};

#endif // WORKER_H
