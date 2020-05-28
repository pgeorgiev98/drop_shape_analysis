#include "gradientdescent.h"

#include <QCoreApplication>
#include <QQueue>
#include <QPair>
#include <QThread>
#include <QMutex>
#include <QEventLoop>

void GDWorker::run()
{
    bool queueIsEmpty = false;
    double b, c;
    while(!queueIsEmpty)
    {
        m_ctx.m1.lock();
        queueIsEmpty = m_ctx.queue.isEmpty();
        if (!queueIsEmpty) {
            auto p = m_ctx.queue.dequeue();
            b = p.first;
            c = p.second;
            int threadNum = 0;
            if (threadNum == 0) {
                QCoreApplication::processEvents();
                double progress = m_ctx.tableSizeB * m_ctx.tableSizeC - m_ctx.queue.size();
                progress /= m_ctx.tableSizeB * m_ctx.tableSizeC;
                m_ctx.m2.lock();
                emit progressChanged(progress);
                m_ctx.m2.unlock();
            }
            if(m_ctx.m_cancelled)
            {
                m_ctx.m1.unlock();
                break;
            }
        }
        m_ctx.m1.unlock();

        if(queueIsEmpty)
            break;
        auto f = [b, this](double c){
            return DropGenerator::calculateError(DropGenerator::generateTheoreticalModel(TheoreticalModelParameters(m_ctx.dropType, b, c, m_ctx.precision, m_ctx.cutoffMoment)), m_ctx.experimental);
        };

        auto der = [f](double c){
            const double h = 0.001;
            return (f(c + h) - f(c)) / h;
        };


        double cNext;
        int steps = 0;
        const int maxSteps = 75;
        while(steps < maxSteps)
        {
            double alpha = 0.1;

            const double fc = f(c);
            double fcn;
            do
            {
                cNext = c -alpha*der(c);
                alpha /= 2;

                fcn = f(cNext);

                ++steps;
            } while(fc <= fcn && qAbs(cNext - c) > m_ctx.gdPrecision && !(qIsInf(fc) && qIsInf(fcn)) && steps < maxSteps);

            if(qAbs(cNext - c) <= m_ctx.gdPrecision || (qIsInf(fc) && qIsInf(fcn)))
                break;

            c = cNext;
        }
        c = cNext;

        double error = f(c);

        m_ctx.m3.lock();
        if (error < m_ctx.bestError) {
            m_ctx.bestParameters.c = c;
            m_ctx.bestParameters.b = b;
            m_ctx.bestError = error;
        }
        m_ctx.m3.unlock();
    }
}

void GradientDescent::doWork(const QVector<QPointF> &experimental, TheoreticalModelParameters::DropType dropType, double precision, int cutoffMoment)
{
    const double minB =  0.1, maxB =  3.0;
    const double minC = -6.0, maxC = 0.0;

    QQueue<QPair<double, double>> queue;

    const int tableSizeB = 100, tableSizeC = 15;
    for (int bi = 0; bi < tableSizeB; ++bi) {
        for (int ci = 0; ci < tableSizeC; ++ci) {
            double b = minB + bi * (maxB - minB) / tableSizeB;
            double c = minC + ci * (maxC - minC) / tableSizeC;
            queue.enqueue({b, c});
        }
    }

    TheoreticalModelParameters bestParameters(dropType, 0, 0, precision, cutoffMoment);

    unsigned threadCount = QThread::idealThreadCount();
    GDWorker *threads[threadCount];
    GDWorker::context context{QMutex(), QMutex(), QMutex(), queue, false, experimental,
                             dropType, precision, cutoffMoment, qInf(), bestParameters};
    m_ctx = &context;

    QEventLoop eventLoop;

    unsigned threadsJoined = 0;

    auto countJoinedThreads = [threadCount, &eventLoop, &threadsJoined]()
    {
        if(++threadsJoined == threadCount)
            eventLoop.quit();
    };

    for(GDWorker*& t : threads)
    {
        t = new GDWorker(context);
        connect(t, &GDWorker::progressChanged, this, &GradientDescent::progressChanged);
        connect(t, &QThread::finished, countJoinedThreads);
        t->start();
    }

    eventLoop.exec();

    for(GDWorker* t : threads)
    {
        t->wait();
        t->deleteLater();
    }

    if (context.m_cancelled)
        emit cancelled();
    else
        emit finished(context.bestParameters);
}

void GradientDescent::cancel()
{
    m_ctx->m_cancelled = true;
}
