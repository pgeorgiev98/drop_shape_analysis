#include "worker.h"

#include <QQueue>
#include <QPair>
#include <QDebug>

#include <omp.h>

void Worker::doWork(const QVector<QPointF> &experimental, DropGenerator::DropType dropType, double precision)
{
    const double minB =  0.5, maxB =  3.0;
    const double minC = -6.0, maxC = -0.5;

    QQueue<QPair<double, double>> queue;

    const int tableSizeB = 30, tableSizeC = 5;
    for (int bi = 0; bi < tableSizeB; ++bi) {
        for (int ci = 0; ci < tableSizeC; ++ci) {
            double b = minB + bi * (maxB - minB) / tableSizeB;
            double c = minC + ci * (maxC - minC) / tableSizeC;
            queue.enqueue({b, c});
        }
    }

    const double gdPrecision = 1e-3;

    DropGenerator::TheoreticalModelParameters bestParameters(dropType, 0, 0, precision);
    double bestError = qInf();

#pragma omp parallel
    for (;;) {
        double b, c;
        bool queueIsEmpty;
#pragma omp critical
        {
            queueIsEmpty = queue.isEmpty();
            if (!queueIsEmpty) {
                auto p = queue.dequeue();
                b = p.first;
                c = p.second;
                int threadNum = omp_get_thread_num();
                if (threadNum == 0) {
                    double progress = tableSizeB * tableSizeC - queue.size();
                    progress /= tableSizeB * tableSizeC;
                    emit progressChanged(progress);
                }
            }
        }
        if (queueIsEmpty)
            break;

        auto f = [b, dropType, precision, &experimental](double c){
            return DropGenerator::calculateError(DropGenerator::generateTheoreticalModel(b, c, dropType, precision), experimental);
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
#pragma omp critical
        {
            if (error < bestError) {
                bestParameters.c = c;
                bestParameters.b = b;
                bestError = error;
            }
        }
    }

    emit finished(bestParameters);
}
