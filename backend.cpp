#include "backend.h"
#include "dropgenerator.h"
#include "gradientdescent.h"
#include <QXYSeries>
#include <QChart>
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <QImage>

using namespace QtCharts;

Backend::Backend(QObject *parent)
    : QObject(parent)
    , m_operationThread(nullptr)
    , m_worker(nullptr)
{
}

void Backend::setTheoreticalSeries(QtCharts::QAbstractSeries *series)
{
    m_theoretical = static_cast<QXYSeries *>(series);
}

void Backend::setExperimentalSeries(QtCharts::QAbstractSeries *series)
{
    m_experimental = static_cast<QXYSeries *>(series);
}

void Backend::setErrorSeries(QtCharts::QAbstractSeries *series)
{
    m_error = static_cast<QXYSeries *>(series);
}

static void adjustAxes(QChart *chart)
{
    chart->createDefaultAxes();

    QList<QAbstractAxis*> axesX= chart->axes(Qt::Horizontal);
    QList<QAbstractAxis*> axesY= chart->axes(Qt::Vertical);

    double maxX = 0.0;
    double maxY = 0.0;
    for (QAbstractSeries *series : chart->series()) {
        QXYSeries *s = static_cast<QXYSeries *>(series);
        const auto &points = s->points();
        for (auto p : points) {
            if (p.x() > maxX)
                maxX = p.x();
            if (p.y() > maxY)
                maxY = p.y();
        }
    }

    if (maxX == 0.0)
        maxX = 1.0;
    if (maxY == 0.0)
        maxY = 1.0;

    for(auto axisX : axesX)
        axisX->setRange(0, maxX);
    for(auto axisY : axesY)
        axisY->setRange(0, maxY);
}

void Backend::generateTheoreticalProfile(double b, double c,
                                         int type,
                                         double precision, int cutoffMoment)
{
    TheoreticalModelParameters params(TheoreticalModelParameters::DropType(type), b, c, precision, cutoffMoment);
    auto v = DropGenerator::generateTheoreticalModel(params);
    m_theoreticalProfile = v;
    m_theoretical->replace(v);

    QChart *chart = m_theoretical->chart();
    adjustAxes(chart);

    updateErrorSeries();
}

bool Backend::loadExperimentalFromTextFile(QString fileUrl)
{
    QString filePath = QUrl(fileUrl).isLocalFile() ? QUrl(fileUrl).toLocalFile() : fileUrl;
    static auto expectChar = [](QTextStream &in, char c) -> bool {
        char ch;
        in >> ch;
        return c == ch;
    };

    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) {
        m_lastError = QString("Failed to open file %1: %2").arg(filePath).arg(f.errorString());
        return false;
    }
    QTextStream in(&f);
    QVector<QPointF> points;
    try {
        if (!expectChar(in, '{'))
            throw "";
        while (!in.atEnd()) {
            if (!expectChar(in, '{'))
                throw "";
            double x, y;
            in >> x;
            if (!expectChar(in, ','))
                throw "";
            in >> y;
            if (!expectChar(in, '}'))
                throw "";
            points.append(QPointF(x, y));
            if (!expectChar(in, ','))
                break;
        }
    } catch(...) {
        m_lastError = "File format error";
        return false;
    }

    m_experimentalProfile = points;

    m_experimental->replace(points);
    // TODO: update cutoff

    QChart *chart = m_experimental->chart();
    adjustAxes(chart);

    updateErrorSeries();

    return true;
}

bool Backend::loadExperimentalFromImageFile(QString fileUrl)
{
    QString filePath = QUrl(fileUrl).isLocalFile() ? QUrl(fileUrl).toLocalFile() : fileUrl;
    auto drop = DropGenerator::generateModelFromImage(filePath);
    m_experimentalProfile = drop;

    m_experimental->replace(drop);
    // TODO: update cutoff

    QChart *chart = m_experimental->chart();
    adjustAxes(chart);

    updateErrorSeries();

    return true;
}

void Backend::updateErrorSeries()
{
    auto error = DropGenerator::generateError(m_theoreticalProfile, m_experimentalProfile);

    m_error->replace(error);

    QChart *chart = m_error->chart();
    adjustAxes(chart);
}

bool Backend::minimizeError(int dropType, double step)
{
    if (m_experimentalProfile.isEmpty()) {
        m_lastError = "An experimental profile must be loaded first";
        return false;
    }

    m_operationThread = new QThread(this);
    m_worker = new GradientDescent;
    m_worker->moveToThread(m_operationThread);

    connect(m_worker, &GradientDescent::progressChanged, this, &Backend::progressChanged, Qt::QueuedConnection);

    QTimer *singleShotTimer = new QTimer(this);
    singleShotTimer->setSingleShot(true);
    QObject::connect(
                singleShotTimer,
                &QTimer::timeout,
                m_worker,
                [this, dropType, step]() {
        m_worker->doWork(m_experimentalProfile, TheoreticalModelParameters::DropType(dropType), step, 0); // TODO: cutoffMoment
    }, Qt::QueuedConnection);
    m_operationThread->start();
    singleShotTimer->start(0);

    TheoreticalModelParameters bestParameters;
    connect(m_worker, &GradientDescent::finished, this, [this](TheoreticalModelParameters params) {
        emit operationCompleted(params.b, params.c);
    }, Qt::QueuedConnection);

    auto destroyWorker = [this]() {
        m_operationThread->quit();
        m_operationThread->wait();
        m_worker->deleteLater();
        m_operationThread->deleteLater();
        m_worker = nullptr;
        m_operationThread = nullptr;
    };

    connect(m_worker, &GradientDescent::finished, this, destroyWorker, Qt::QueuedConnection);

    connect(m_worker, &GradientDescent::cancelled, this, destroyWorker, Qt::QueuedConnection);
    connect(m_worker, &GradientDescent::cancelled, this, &Backend::operationCanceled, Qt::QueuedConnection);

    return true;
}

void Backend::cancelOperation()
{
    if (m_worker && m_operationThread) {
        QTimer *timer = new QTimer(this);
        timer->setSingleShot(true);
        connect(timer, &QTimer::timeout, m_worker, &GradientDescent::cancel, Qt::QueuedConnection);
        timer->start(0);
    }
}

QString Backend::getTempDir()
{
    return m_tempDir.path();
}

void Backend::setPhoto(QString path, QRectF crop)
{
    QImage image(path);
    double w = image.width();
    double h = image.height();
    QRect cropI;
    cropI.setX(crop.x() * w);
    cropI.setY(crop.y() * h);
    cropI.setWidth(crop.width() * w);
    cropI.setHeight(crop.height() * h);
    QImage cropped(cropI.width(), cropI.height(), image.format());
    for (int y = 0; y < cropI.height(); ++y) {
        for (int x = 0; x < cropI.width(); ++x) {
            cropped.setPixel(x, y, image.pixel(cropI.x() + x, cropI.y() + y));
        }
    }
    cropped.save(path);

    loadExperimentalFromImageFile(path);
}
