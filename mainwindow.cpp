#include "mainwindow.h"

#include <QTextStream>
#include <QPainter>
#include <QMessageBox>
#include <QLineSeries>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QtMath>
#include <limits>
#include <QComboBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QDoubleValidator>
#include <QSettings>
#include <QList>
#include <QQueue>
#include <QDateTime>
#include <QDebug>

using namespace QtCharts;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_chartsLayout(new QHBoxLayout)
    , m_inputprecision(new QLineEdit)
    , m_inputb(new QLineEdit)
    , m_inputc(new QLineEdit)
    , m_modelErrorLabel(new QLabel)
    , m_modelChart(new QChartView)
    , m_errorChart(new QChartView)
    , m_dropType(new QComboBox)
    , m_theoreticalSeries(new QLineSeries)
    , m_experimentalSeries(new QLineSeries)
    , m_errorSeries(new QLineSeries)
{
    const double minDouble = std::numeric_limits<double>::lowest();
    const double maxDouble = std::numeric_limits<double>::max();
    m_dropType->addItems({"Pendant", "Rotating"});
    m_modelChart->setMinimumSize(500, 500);
    m_errorChart->setMinimumSize(500, 500);
    m_inputb->setValidator(new QDoubleValidator(minDouble, maxDouble, 1000));
    m_inputc->setValidator(new QDoubleValidator(minDouble, maxDouble, 1000));
    QWidget *w = new QWidget;
    w->setLayout(m_chartsLayout);
    setCentralWidget(w);
    m_inputprecision->setPlaceholderText("Default is 0.1");
    m_inputprecision->setValidator(new QDoubleValidator(minDouble, maxDouble, 1000));
    m_inputb->setAlignment(Qt::AlignRight);
    m_inputc->setAlignment(Qt::AlignRight);
    m_inputprecision->setAlignment(Qt::AlignRight);
    m_modelErrorLabel->setAlignment(Qt::AlignHCenter);

    QPushButton *visualiseTheoreticalModelButton = new QPushButton("Visualise theoretical model");
    QPushButton *loadExperimentalModelButton = new QPushButton("Load experimental model");
    QPushButton *generateClosestModelButton = new QPushButton("Generate closest theoretical model");

    QGroupBox *calculationSettingsBox = new QGroupBox("Calculation settings");
    QGroupBox *theoreticalModelBox = new QGroupBox("Theoretical model");

    {
        QGridLayout *l = new QGridLayout;
        calculationSettingsBox->setLayout(l);

        int row = 0;
        l->addWidget(new QLabel("Drop type: "), row, 0);
        l->addWidget(m_dropType, row, 1);
        ++row;

        l->addWidget(new QLabel("Precision: "), row, 0, Qt::AlignRight);
        l->addWidget(m_inputprecision, row, 1);
        ++row;
    }

    {
        QGridLayout *l = new QGridLayout;
        theoreticalModelBox->setLayout(l);

        int row = 0;
        l->addWidget(new QLabel("b: "), row, 0, Qt::AlignRight);
        l->addWidget(m_inputb, row, 1);
        ++row;

        l->addWidget(new QLabel("c: "), row, 0, Qt::AlignRight);
        l->addWidget(m_inputc, row, 1);
        ++row;

        l->addWidget(visualiseTheoreticalModelButton, row++, 0, 1, 2);
    }

    QGridLayout *grid = new QGridLayout;
    grid->setSpacing(16);
    int row = 0;

    grid->setRowStretch(row++, 1);

    grid->addWidget(calculationSettingsBox, row++, 0, 1, 2);
    grid->addWidget(theoreticalModelBox, row++, 0, 1, 2);

    grid->addWidget(loadExperimentalModelButton, row++, 0, 1, 2);
    grid->addWidget(generateClosestModelButton, row++, 0, 1, 2);

    grid->setRowStretch(row++, 1);

    grid->addWidget(m_modelErrorLabel, row++, 0, 1, 2);

    m_chartsLayout->addLayout(grid);
    m_chartsLayout->addWidget(m_modelChart, 1);
    m_chartsLayout->addWidget(m_errorChart, 1);

    m_theoreticalSeries->setName("Theoretical model");
    m_experimentalSeries->setName("Experimental model");
    m_modelChart->chart()->addSeries(m_theoreticalSeries);
    m_modelChart->chart()->addSeries(m_experimentalSeries);
    m_modelChart->chart()->createDefaultAxes();
    m_modelChart->setRenderHint(QPainter::Antialiasing);

    m_errorSeries->setName("Error");
    m_errorSeries->setPen(QPen(Qt::red));
    m_errorChart->chart()->addSeries(m_errorSeries);
    m_errorChart->chart()->createDefaultAxes();
    m_errorChart->setRenderHint(QPainter::Antialiasing);

    connect(m_inputb, &QLineEdit::returnPressed, this, &MainWindow::visualiseTheoreticalModel);
    connect(m_inputc, &QLineEdit::returnPressed, this, &MainWindow::visualiseTheoreticalModel);

    connect(visualiseTheoreticalModelButton, &QPushButton::clicked, this, &MainWindow::visualiseTheoreticalModel);
    connect(loadExperimentalModelButton, &QPushButton::clicked, this, &MainWindow::selectExperimentalModel);
    connect(generateClosestModelButton, &QPushButton::clicked, this, &MainWindow::visualiseClosestTheoreticalModel);
}

static bool expectChar(QTextStream &in, char c)
{
    char ch;
    in >> ch;
    return c == ch;
}

void MainWindow::setSeries(QLineSeries *series, const QVector<QPointF> &points)
{
    QChart *c = series->chart();
    c->removeSeries(series);
    series->clear();
    for (auto p : points)
        series->append(p.x(), p.y());
    c->addSeries(series);
    c->createDefaultAxes();
    QList<QAbstractAxis*> axesX= m_modelChart->chart()->axes(Qt::Horizontal);
    QList<QAbstractAxis*> axesY= m_modelChart->chart()->axes(Qt::Vertical);

    for(auto axisX : axesX)
        axisX->setRange(0, 1);

    for(auto axisY : axesY)
        axisY->setMin(0);
}

void MainWindow::visualiseTheoreticalModel()
{
    double b, c;
    b = m_inputb->text().replace(',', '.').toDouble();
    c = m_inputc->text().replace(',', '.').toDouble();
    bool precisionValid;
    double h = m_inputprecision->text().replace(',', '.').toDouble(&precisionValid);
    if (!precisionValid)
        h = 0.1;

    DropType dropType = m_dropType->currentIndex() == 0 ? DropType::PENDANT : DropType::SPINNING;

    auto drop = generateTheoreticalModel(b, c, dropType, h);
    if(drop.isEmpty())
    {
        QMessageBox::critical(this, "Error", "Maximum number of iterations reached");
        return;
    }
    setSeries(m_theoreticalSeries, drop);
    m_currentTheoreticalModel = drop;
    updateErrorSeries();
}

void MainWindow::visualiseClosestTheoreticalModel()
{
    if (m_currentExperimentalModel.isEmpty()) {
        QMessageBox::information(this, "Select experimental model", "An experimental model must be loaded first");
        return;
    }

    TheoreticalModelParameters parameters = minimizeError();
    m_dropType->setCurrentIndex(parameters.dropType == DropType::PENDANT ? 0 : 1);
    m_inputb->setText(QString::number(parameters.b));
    m_inputc->setText(QString::number(parameters.c));
    m_inputprecision->setText(QString::number(parameters.precision));
    visualiseTheoreticalModel();
}

void MainWindow::selectExperimentalModel()
{
    QSettings settings;
    static const char *dataDirKey = "experimental-data-dir";
    QString dir = settings.value(dataDirKey, QString()).toString();
    QString fileName = QFileDialog::getOpenFileName(this, "Select experimental model", dir);
    if (!fileName.isEmpty()) {
        settings.setValue(dataDirKey, QFileInfo(fileName).dir().path());
        setExperimentalModel(fileName);
    }
}

void MainWindow::setExperimentalModel(const QString &filePath)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", QString("Failed to open file %1: %2").arg(filePath).arg(f.errorString()));
        return;
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
        QMessageBox::critical(this, "Error", "File format error");
        return;
    }

    setSeries(m_experimentalSeries, points);
    m_currentExperimentalModel = points;
    updateErrorSeries();
}

void MainWindow::updateErrorSeries()
{
    auto theoretical = m_currentTheoreticalModel;
    auto experimental = m_currentExperimentalModel;

    auto error = generateError(theoretical, experimental);
    setSeries(m_errorSeries, error);
    if (!error.isEmpty()) {
        double errorValue = calculateError(error);
        m_modelErrorLabel->setText(QString("Squared error: %1").arg(errorValue));
    } else {
        m_modelErrorLabel->setText(QString());
    }
}

QVector<QPointF> MainWindow::generateTheoreticalModel(double b, double c, DropType type, double precision)
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
    qDebug() << "steps:" << steps;

    return drop;
}

QVector<QPointF> MainWindow::generateError(const QVector<QPointF> &theoretical, const QVector<QPointF> &experimental)
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

double MainWindow::calculateError(const QVector<QPointF> &error)
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

double MainWindow::calculateError(const QVector<QPointF> &theoretical, const QVector<QPointF> &experimental)
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

MainWindow::TheoreticalModelParameters MainWindow::minimizeError()
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
    const DropType dropType = m_dropType->currentIndex() == 0 ? DropType::PENDANT : DropType::SPINNING;
    double precision;
    {
        bool precisionValid;
        precision = m_inputprecision->text().replace(',', '.').toDouble(&precisionValid);
        if (!precisionValid)
            precision = 0.1;
    }

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

        auto f = [this, b, dropType, precision](double c){
            return calculateError(generateTheoreticalModel(b, c, dropType, precision), m_currentExperimentalModel);
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
