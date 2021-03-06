#ifndef BACKEND_H
#define BACKEND_H

#include "theoreticalmodelparameters.h"
#include <QObject>
#include <QString>
#include <QVector>
#include <QPointF>
#include <QRectF>
#include <QTemporaryDir>

namespace QtCharts {
    class QAbstractSeries;
    class QXYSeries;
}

class GradientDescent;

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
    void operationCanceled();

public slots:
    QString lastError() const
    {
        return m_lastError;
    }

    void setTheoreticalSeries(QtCharts::QAbstractSeries *series);
    void setExperimentalSeries(QtCharts::QAbstractSeries *series);
    void setErrorSeries(QtCharts::QAbstractSeries *series);

    void generateTheoreticalProfile(double b, double c, int type, double precision);
    bool loadExperimentalFromTextFile(QString fileUrl);
    bool loadExperimentalFromImageFile(QString fileUrl);
    bool minimizeError(int dropType, double step);

    void cancelOperation();

    QString getTempDir();
    void setPhoto(QString path, QRectF crop);

private slots:
    void updateErrorSeries();
    void updateCutoffMoment();

private:
    QtCharts::QXYSeries *m_theoretical;
    QtCharts::QXYSeries *m_experimental;
    QtCharts::QXYSeries *m_error;

    QString m_lastError;
    QVector<QPointF> m_theoreticalProfile;
    QVector<QPointF> m_experimentalProfile;
    int m_cutoffMoment;

    QThread *m_operationThread;
    GradientDescent *m_worker;

    QTemporaryDir m_tempDir;
};

#endif // BACKEND_H
