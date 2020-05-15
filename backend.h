#ifndef BACKEND_H
#define BACKEND_H

#include "theoreticalmodelparameters.h"
#include <QObject>
#include <QString>
#include <QVector>
#include <QPointF>

namespace QtCharts {
    class QAbstractSeries;
}

class Backend : public QObject
{
    Q_OBJECT
public:
    explicit Backend(QObject *parent = nullptr);

public slots:
    QString lastError() const
    {
        return m_lastError;
    }

    void generateTheoreticalProfile(double b, double c, int type, double precision, int cutoffMoment, QtCharts::QAbstractSeries *series);
    bool loadExperimentalFromTextFile(QString fileUrl, QtCharts::QAbstractSeries *series);
    bool loadExperimentalFromImageFile(QString fileUrl, QtCharts::QAbstractSeries *series);
    void updateErrorSeries(QtCharts::QAbstractSeries *errorSeries);

private:
    QString m_lastError;
    QVector<QPointF> m_theoreticalProfile;
    QVector<QPointF> m_experimentalProfile;
};

#endif // BACKEND_H
