#include "dropgenerator.h"

#include <QtMath>
#include <QVector2D>
#include <QQueue>
#include <QPair>
#include <QDebug>
#include <QDateTime>
#include <QImage>
#include <QVector>
#include <QColor>

QVector<QPointF> DropGenerator::generateTheoreticalModel(double b, double c, DropType type, double precision)
{
    const int maximumXDirectionChanges = (type == DropType::PENDANT ? 2 : 1);
    const int maximumYDirectionChanges = (type == DropType::PENDANT ? 0 : 1);
    int xDirectionChanges = 0, yDirectionChanges = 0;
    double currentXDirection = 1, currentYDirection = 1; // Moving upwards to the right

    QVector<QPointF> drop;
    const double h = precision;
    auto f1=[](double phi){return qCos(phi);};
    auto f2=[](double phi){return qSin(phi);};
    auto f3p=[b, c](double x, double z, double phi){return x == 0.0 ? b : 2*b + c*z - (qSin(phi) / x);};
    auto f3r=[b, c](double x, double z, double phi){return x == 0.0 ? b : 2*b + c*x*x - (qSin(phi) / x);};

    double currx = 0, nextx, currz = 0, nextz, currphi = 0, nextphi;
    int steps = 0, maxSteps = 5000;

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

            if ((nextx - currx) * currentXDirection < 0.0) {
                if (++xDirectionChanges > maximumXDirectionChanges)
                    break;
                currentXDirection = nextx - currx;
            }
            if ((nextz - currz) * currentYDirection < 0.0) {
                if (++yDirectionChanges > maximumYDirectionChanges)
                    break;
                currentYDirection = nextz - currz;
            }

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

            if ((nextx - currx) * currentXDirection < 0.0) {
                if (++xDirectionChanges > maximumXDirectionChanges)
                    break;
                currentXDirection = nextx - currx;
            }
            if ((nextz - currz) * currentYDirection < 0.0) {
                if (++yDirectionChanges > maximumYDirectionChanges)
                    break;
                currentYDirection = nextz - currz;
            }

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

    QVector<QPointF> totalError;
    QVector<QVector<QPointF>> errors;

    typedef const QVector<QPointF> &ModelRef;
    QVector<QPair<ModelRef, ModelRef>> modelsArr = {{theoretical, experimental}, {experimental, theoretical}};

    for (QPair<ModelRef, ModelRef> models : modelsArr) {
        ModelRef modelA = models.first;
        ModelRef modelB = models.second;
        QVector<QPointF> error;
        for (int j = 0; j < modelA.size(); ++j) {
            const QPointF &a = modelA[j];
            double minDist = qInf();
            for (int i = 0; i < modelB.size() - 1; ++i) {
                const QPointF &b0 = modelB[i], &b1 = modelB[i + 1];
                const QPointF d = b1 - b0;
                const QPointF dp(-d.y(), d.x());
                double l = dp.x() * (b0.y() - a.y()) - dp.y() * (b0.x() - a.x()) / (d.x() * dp.y() - d.y() * dp.x());
                QPointF target = b0 + qBound(0.0, l, 1.0) * d;
                double dist = qMin(QVector2D(a - target).length(),
                                   qMin(QVector2D(a - b0).length(),
                                        QVector2D(a - b1).length()));
                if (dist < minDist)
                    minDist = dist;
            }
            double vx = double(j) / modelA.size();
            error.append({vx, minDist});
        }
        errors.append(std::move(error));
    }

    // Merge errors into totalError
    int i0 = 0, i1 = 0;
    while (i0 < errors[0].size() && i1 < errors[1].size())
        if (errors[0][i0].x() < errors[1][i1].x())
            totalError.append(errors[0][i0++]);
        else
            totalError.append(errors[1][i1++]);
    for (; i0 < errors[0].size(); ++i0)
        totalError.append(errors[0][i0]);
    for (; i1 < errors[1].size(); ++i1)
        totalError.append(errors[1][i1]);

    return totalError;
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

    double totalError = 0.0;

    typedef const QVector<QPointF> &ModelRef;
    QVector<QPair<ModelRef, ModelRef>> modelsArr = {{theoretical, experimental}, {experimental, theoretical}};

    for (QPair<ModelRef, ModelRef> models : modelsArr) {
        ModelRef modelA = models.first;
        ModelRef modelB = models.second;
        QVector<QPointF> error;
        for (int j = 0; j < modelA.size(); ++j) {
            const QPointF &a = modelA[j];
            double minDist = qInf();
            for (int i = 0; i < modelB.size() - 1; ++i) {
                const QPointF &b0 = modelB[i], &b1 = modelB[i + 1];
                const QPointF d = b1 - b0;
                const QPointF dp(-d.y(), d.x());
                double l = dp.x() * (b0.y() - a.y()) - dp.y() * (b0.x() - a.x()) / (d.x() * dp.y() - d.y() * dp.x());
                QPointF target = b0 + qBound(0.0, l, 1.0) * d;
                double dist = qMin(QVector2D(a - target).lengthSquared(),
                                   qMin(QVector2D(a - b0).lengthSquared(),
                                        QVector2D(a - b1).lengthSquared()));
                if (dist < minDist)
                    minDist = dist;
            }
            totalError += minDist;
        }
    }

    return totalError;
}

QVector<QPointF> DropGenerator::generateModelFromImage(const QString fileName)
{
    QImage image;
    if(!image.load(fileName))
    {
        qDebug() << "no";
        return {};
    }

    double averageValue = 0;
    for(int i = 0; i < image.height(); ++i)
    {
        for(int j = 0; j < image.width(); ++j)
        {
            int r, g, b;
            image.pixelColor(j, i).getRgb(&r, &g, &b);
            double newColor;
            newColor = 0.2126*r + 0.7152*g + 0.0722*b;
            image.setPixelColor(j, i, QColor(newColor, newColor, newColor));
            averageValue += image.pixelColor(j, i).valueF();
        }
    }

    averageValue /= image.height()*image.width();

    auto white = QColorConstants::White;
    auto black = QColorConstants::Black;

    for(int i = 0; i < image.height(); ++i)
    {
        double rowColorValue = image.pixelColor(0, i).valueF();
        for(int j = 0; j < image.width(); ++j)
        {
            double value = image.pixelColor(j, i).valueF();
            if(qAbs(value - rowColorValue) > qAbs(averageValue - rowColorValue))
            {
                image.setPixelColor(j, i, black);
            }
            else
            {
                image.setPixelColor(j, i, white);
            }
        }
    }

    QVector<QPointF> drop;
    int apex = 0, apexY = 0;
    for(int i = image.height() - 1; i >= 0; --i)
    {
        for(int j = 0; j < image.width(); ++j)
        {
            if(image.pixelColor(j, i) == black)
            {
                drop.append(QPointF(j, i));
                if(j > apex)
                {
                    apex = j;
                    apexY = i;
                }
                break;
            }
        }
    }
    double pivot = drop.last().x();
    double scaleFactor = apex - pivot;

    for(auto &point : drop)
    {
        point.setX(point.x() + 2*(apex - point.x()) - apex);
        point.setY(apexY - point.y());
        point /= scaleFactor;
    }
    return drop;
}
