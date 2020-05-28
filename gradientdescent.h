#ifndef WORKER_H
#define WORKER_H

#include "theoreticalmodelparameters.h"
#include "dropgenerator.h"
#include <QObject>
#include <QThread>
#include <QMutex>
#include <QQueue>

class GDWorker : public QThread
{
    Q_OBJECT

public:
    struct context
    {
        static constexpr double gdPrecision = 1e-3;
        static const int tableSizeB = 100, tableSizeC = 15;
        QMutex m1, m2, m3;
        QQueue<QPair<double, double>> queue;
        bool m_cancelled;
        const QVector<QPointF> &experimental;
        TheoreticalModelParameters::DropType dropType;
        double precision;
        int cutoffMoment;
        double bestError;
        TheoreticalModelParameters bestParameters;
    };

    GDWorker(context &context)
        :m_ctx(context)
    {}

    void run() override;

signals:
    void progressChanged(double progress);

private:
    context &m_ctx;
};

class GradientDescent : public QObject
{
    Q_OBJECT
public:
    explicit GradientDescent(QObject *parent = nullptr)
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
    GDWorker::context *m_ctx;
};

#endif // WORKER_H
