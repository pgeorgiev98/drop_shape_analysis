#include "dropgenerator.h"

#include <QtMath>
#include <QVector2D>
#include <QQueue>
#include <QPair>
#include <QDebug>
#include <QDateTime>

DropGenerator::DropGenerator(QObject *parent) : QObject(parent)
{

}

QVector<QPointF> DropGenerator::generateTheoreticalModel(double b, double c, DropType type, double precision)
{
    QVector<QPointF> drop;
    const double h = precision;
    auto f1=[](double phi){return qCos(phi);};
    auto f2=[](double phi){return qSin(phi);};
    auto f3p=[b, c](double x, double z, double phi){return x == 0.0 ? b : 2*b + c*z - (qSin(phi) / x);};
    auto f3r=[b, c](double x, double z, double phi){return x == 0.0 ? b : 2*b + c*x*x - (qSin(phi) / x);};

    double currx = 0, nextx, currz = 0, nextz, currphi = 0, nextphi;
    int steps = 0, maxSteps = 500;

    if(type == DropType::PENDANT)
    {
        while(steps <= maxSteps)
        {
            drop.append({currx, currz});

            if(currx > 1.0)
                break;

            double k1, k2, k3, k4;
            double l1, l2, l3, l4;
            double m1, m2, m3, m4;

            k1 = h*f1(currphi);
            l1 = h*f2(currphi);
            m1 = h*f3p(currx, currz, currphi);
            k2 = h*f1(currphi + m1/2);
            l2 = h*f2(currphi + m1/2);
            m2 = h*f3p(currx + k1/2, currz + l1/2, currphi + m1/2);
            k3 = h*f1(currphi + m2/2);
            l3 = h*f2(currphi + m2/2);
            m3 = h*f3p(currx + k2/2, currz + l2/2, currphi + m2/2);
            k4 = h*f1(currphi + m3);
            l4 = h*f2(currphi + m3);
            m4 = h*f3p(currx + k3, currz + l3, currphi + m3);

            nextx = currx + 1.0/6 * (k1 + 2*k2 + 2*k3 + k4);
            nextz = currz + 1.0/6 * (l1 + 2*l2 + 2*l3 + l4);
            nextphi = currphi + 1.0/6 * (m1 + 2*m2 + 2*m3 + m4);

            currx = nextx;
            currz = nextz;
            currphi = nextphi;

            ++steps;
        }
    }
    else{
        while(steps <= maxSteps)
        {
            drop.append({currx, currz});

            double k1, k2, k3, k4;
            double l1, l2, l3, l4;
            double m1, m2, m3, m4;
            k1 = h*f1(currphi);
            l1 = h*f2(currphi);
            m1 = h*f3r(currx, currz, currphi);
            k2 = h*f1(currphi + m1/2);
            l2 = h*f2(currphi + m1/2);
            m2 = h*f3r(currx + k1/2, currz + l1/2, currphi + m1/2);
            k3 = h*f1(currphi + m2/2);
            l3 = h*f2(currphi + m2/2);
            m3 = h*f3r(currx + k2/2, currz + l2/2, currphi + m2/2);
            k4 = h*f1(currphi + m3);
            l4 = h*f2(currphi + m3);
            m4 = h*f3r(currx + k3, currz + l3, currphi + m3);

            nextx = currx + 1.0/6 * (k1 + 2*k2 + 2*k3 + k4);
            nextz = currz + 1.0/6 * (l1 + 2*l2 + 2*l3 + l4);
            nextphi = currphi + 1.0/6 * (m1 + 2*m2 + 2*m3 + m4);

            currx = nextx;
            currz = nextz;
            currphi = nextphi;

            if(currx <= std::numeric_limits<double>::epsilon())
                break;

            ++steps;
        }
    }

    if(steps >= maxSteps)
    {
        return {};
    }

    return drop;
}

QVector<QPointF> DropGenerator::generateError(const QVector<QPointF> &theoretical, const QVector<QPointF> &experimental)
{
    if (theoretical.size() < 2 || experimental.isEmpty()) {
        return {};
    }

    QVector<QPointF> error;

    for (int j = 0; j < experimental.size(); ++j) {
        const QPointF &e = experimental[j];
        double minDist = qInf();
        for (int i = 0; i < theoretical.size() - 1; ++i) {
            const QPointF &t0 = theoretical[i], &t1 = theoretical[i + 1];
            const QPointF d = t1 - t0;
            const QPointF dp(-d.y(), d.x());
            double l = dp.x() * (t0.y() - e.y()) - dp.y() * (t0.x() - e.x()) / (d.x() * dp.y() - d.y() * dp.x());
            QPointF target = t0 + qBound(0.0, l, 1.0) * d;
            double dist = qMin(QVector2D(e - target).length(),
                               qMin(QVector2D(e - t0).length(),
                                    QVector2D(e - t1).length()));
            if (dist < minDist)
                minDist = dist;
        }
        double vx = double(j) / experimental.size();
        error.append({vx, minDist});
    }

    return error;
}

double DropGenerator::calculateError(const QVector<QPointF> &error)
{
    if(error.isEmpty())
        return qInf();

    double errorAcc = 0;
    for(auto point : error)
    {
        errorAcc += point.y() * point.y();
    }

    return errorAcc;
}

double DropGenerator::calculateError(const QVector<QPointF> &theoretical, const QVector<QPointF> &experimental)
{
    if (theoretical.size() < 2 || experimental.isEmpty()) {
        return qInf();
    }

    double error = 0.0;

    for (int j = 0; j < experimental.size(); ++j) {
        const QPointF &e = experimental[j];
        double minDist = qInf();
        for (int i = 0; i < theoretical.size() - 1; ++i) {
            const QPointF &t0 = theoretical[i], &t1 = theoretical[i + 1];
            const QPointF d = t1 - t0;
            const QPointF dp(-d.y(), d.x());
            double l = dp.x() * (t0.y() - e.y()) - dp.y() * (t0.x() - e.x()) / (d.x() * dp.y() - d.y() * dp.x());
            QPointF target = t0 + qBound(0.0, l, 1.0) * d;
            double dist = qMin(QVector2D(e - target).lengthSquared(),
                               qMin(QVector2D(e - t0).lengthSquared(),
                                    QVector2D(e - t1).lengthSquared()));
            if (dist < minDist)
                minDist = dist;
        }
        error += minDist;
    }

    return error;
}

DropGenerator::TheoreticalModelParameters DropGenerator::minimizeError(const QVector<QPointF> &experimental, DropType dropType, double precision)
{
    qDebug() << "In minimizeError()";
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

    TheoreticalModelParameters bestParameters(dropType, 0, 0, precision);
    double bestError = qInf();

    QDateTime dt = QDateTime::currentDateTime();
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
                qDebug().nospace() << "Solving with b = " << b << " c = " << c << ") (" << tableSizeB * tableSizeC - queue.size() << "/" << tableSizeB * tableSizeC << ")";
            }
        }
        if (queueIsEmpty)
            break;

        auto f = [b, dropType, precision, &experimental](double c){
            return calculateError(generateTheoreticalModel(b, c, dropType, precision), experimental);
        };

        auto der = [f](double c){
            const double h = 0.001;
            return (f(c + h) - f(c)) / h;
        };


        double cNext;
        int steps = 0;
        const int maxSteps = 1000;
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
            qDebug() << "Completed after" << steps << "steps; b =" << b << "c =" << c << "error =" << error;
            if (error < bestError) {
                bestParameters.c = c;
                bestParameters.b = b;
                bestError = error;
            }
        }
    }

    qDebug() << "Done. (took" << dt.msecsTo(QDateTime::currentDateTime()) << "ms)";

    return bestParameters;
}
