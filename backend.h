#ifndef BACKEND_H
#define BACKEND_H

#include "theoreticalmodelparameters.h"
#include <QObject>
#include <QString>
#include <QVector>
#include <QPointF>

namespace QtCharts {
    class QAbstractSeries;
    class QXYSeries;
}

class Backend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QtCharts::QAbstractSeries * theoreticalSeries WRITE setTheoreticalSeries)
    Q_PROPERTY(QtCharts::QAbstractSeries * experimentalSeries WRITE setExperimentalSeries)
    Q_PROPERTY(QtCharts::QAbstractSeries * errorSeries WRITE setErrorSeries)
public:
    explicit Backend(QObject *parent = nullptr);

signals:
    void progressChanged(double progress);
    void operationCompleted(double b, double c);

public slots:
    QString lastError() const
    {
        return m_lastError;
    }

    void setTheoreticalSeries(QtCharts::QAbstractSeries *series);
    void setExperimentalSeries(QtCharts::QAbstractSeries *series);
    void setErrorSeries(QtCharts::QAbstractSeries *series);

    void generateTheoreticalProfile(double b, double c, int type, double precision, int cutoffMoment);
    bool loadExperimentalFromTextFile(QString fileUrl);
    bool loadExperimentalFromImageFile(QString fileUrl);
    bool minimizeError(int dropType, double step);

private slots:
    void updateErrorSeries();

private:
    QtCharts::QXYSeries *m_theoretical;
    QtCharts::QXYSeries *m_experimental;
    QtCharts::QXYSeries *m_error;

    QString m_lastError;
    QVector<QPointF> m_theoreticalProfile;
    QVector<QPointF> m_experimentalProfile;
};

#endif // BACKEND_H
