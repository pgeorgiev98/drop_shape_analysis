#ifndef WORKER_H
#define WORKER_H

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
    void doWork(const QVector<QPointF> &experimental, DropGenerator::DropType dropType, double precision, int cutoffMoment);

signals:
    void finished(DropGenerator::TheoreticalModelParameters parameters);
    void progressChanged(double progress);
};

#endif // WORKER_H
