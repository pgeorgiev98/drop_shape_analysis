#include "worker.h"

#include <QCoreApplication>
#include <QQueue>
#include <QPair>
#include <QThread>
#include <QMutex>
#include <QEventLoop>

void Worker::doWork(const QVector<QPointF> &experimental, TheoreticalModelParameters::DropType dropType, double precision, int cutoffMoment)
{
    m_cancelled = false;

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

    const double gdPrecision = 1e-3;

    TheoreticalModelParameters bestParameters(dropType, 0, 0, precision, cutoffMoment);
    double bestError = qInf();

    QMutex m1, m2, m3;
    auto threadWork = [&]()
    {
        bool queueIsEmpty = false;
        double b, c;
        while(!queueIsEmpty)
        {
            m1.lock();
            queueIsEmpty = queue.isEmpty();
            if (!queueIsEmpty) {
                auto p = queue.dequeue();
                b = p.first;
                c = p.second;
                int threadNum = 0;
                if (threadNum == 0) {
                    QCoreApplication::processEvents();
                    double progress = tableSizeB * tableSizeC - queue.size();
                    progress /= tableSizeB * tableSizeC;
                    m2.lock();
                    emit progressChanged(progress);
                    m2.unlock();
                }
                if(m_cancelled)
                {
                    m1.unlock();
                    break;
                }
            }
            m1.unlock();

            if(queueIsEmpty)
                break;
            auto f = [b, dropType, precision, cutoffMoment, &experimental](double c){
                return DropGenerator::calculateError(DropGenerator::generateTheoreticalModel(TheoreticalModelParameters(dropType, b, c, precision, cutoffMoment)), experimental);
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
                } while(fc <= fcn && qAbs(cNext - c) > gdPrecision && !(qIsInf(fc) && qIsInf(fcn)) && steps < maxSteps);

                if(qAbs(cNext - c) <= gdPrecision || (qIsInf(fc) && qIsInf(fcn)))
                    break;

                c = cNext;
            }
            c = cNext;

            double error = f(c);

            m3.lock();
            if (error < bestError) {
                bestParameters.c = c;
                bestParameters.b = b;
                bestError = error;
            }
            m3.unlock();
        }
    };

    unsigned threadCount = 4;
    QThread* threads[threadCount];

    for(QThread*& t : threads)
    {
        t = QThread::create(threadWork);
        t->start();
    }

    QEventLoop eventLoop;

    unsigned threadsJoined = 0;

    auto countJoinedThreads = [threadCount, &eventLoop, &threadsJoined]()
    {
        if(++threadsJoined == threadCount)
            eventLoop.quit();
    };

    for(QThread* t : threads)
    {
        connect(t, &QThread::finished, countJoinedThreads);
    }

    eventLoop.exec();


    for(QThread*& t : threads)
    {
        t->wait();
        t->deleteLater();
    }

    if (m_cancelled)
        emit cancelled();
    else
        emit finished(bestParameters);
}

void Worker::cancel()
{
    m_cancelled = true;
}
