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
    void cancel();

signals:
    void finished(TheoreticalModelParameters parameters);
    void progressChanged(double progress);
    void cancelled();

private:
    bool m_cancelled;
};

#endif // WORKER_H
