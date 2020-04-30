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
    double x0 = 0, z0 = 0, phi0 = 0;
    double x1, z1, phi1;
    double h = precision;

    const int maxIterations = 2000000;
    int iteration;

    if(type == DropType::PENDANT)
    {
        for (iteration = 0; iteration < maxIterations; ++iteration)
        {
            drop.append(QPointF(x0, z0));
            if (x0 > 1.0)
                break;
            x1 = x0 + h * cos(phi0);
            z1 = z0 + h * sin(phi0);
            if(x0 == 0)
            {
                phi1 = phi0 + h * b;
            }
            else
            {
                phi1 = phi0 + h * (2*b + c*z0 - sin(phi0)/x0);
            }
            x0 = x1;
            z0 = z1;
            phi0 = phi1;
        }
    }
    else
    {
        drop.append(QPointF(x0, z0));
        for (iteration = 0; iteration < maxIterations; ++iteration)
        {
            x1 = x0 + h * cos(phi0);
            z1 = z0 + h * sin(phi0);
            if(x0 == 0)
            {
                phi1 = phi0 + h * b;
            }
            else
            {
                phi1 = phi0 + h * (2*b + c*x0*x0 - sin(phi0)/x0);
            }
            x0 = x1;
            z0 = z1;
            phi0 = phi1;
            drop.append(QPointF(x0, z0));
            if (x0 <= std::numeric_limits<double>::epsilon())
                break;
        }
    }

    if (iteration >= maxIterations) {
        return {};
    }

    return drop;
}

QVector<QPointF> MainWindow::generateError(const QVector<QPointF> &theoretical, const QVector<QPointF> &experimental)
{
    if (experimental.size() < 2 || theoretical.isEmpty()) {
        return {};
    }

    QVector<QPointF> error;

    for (int j = 0; j < theoretical.size(); ++j) {
        const QPointF &t = theoretical[j];
        double minDist = qInf();
        for (int i = 0; i < experimental.size() - 1; ++i) {
            const QPointF &e0 = experimental[i], &e1 = experimental[i + 1];
            const QPointF d = e1 - e0;
            const QPointF dp(-d.y(), d.x());
            double l = dp.x() * (e0.y() - t.y()) - dp.y() * (e0.x() - t.x()) / (d.x() * dp.y() - d.y() * dp.x());
            QPointF target = e0 + qBound(0.0, l, 1.0) * d;
            double dist = qMin(QVector2D(t - target).length(),
                               qMin(QVector2D(t - e0).length(),
                                    QVector2D(t - e1).length()));
            if (dist < minDist)
                minDist = dist;
        }
        double vx = double(j)/theoretical.size();
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

MainWindow::TheoreticalModelParameters MainWindow::minimizeError()
{
    qDebug() << "In minimizeError()";
    const double b = 1.843;
    double c = -2, cNext;
    const double gdPrecision = 1e-3;
    DropType dropType = m_dropType->currentIndex() == 0 ? DropType::PENDANT : DropType::SPINNING;
    bool precisionValid;
    double precision = m_inputprecision->text().replace(',', '.').toDouble(&precisionValid);
    if (!precisionValid)
        precision = 0.1;

    auto f = [this, b, dropType, precision](double c){
        return calculateError(generateError(generateTheoreticalModel(b, c, dropType, precision), m_currentExperimentalModel));
    };

    auto der = [f](double c){
        const double h = 0.001;
        return (f(c + h) - f(c)) / h;
    };


    int steps = 0;
    while(steps <= 10000)
    {
        double alpha = 0.1;

        do
        {
            cNext = c -alpha*der(c);
            alpha /= 2;

            if(qAbs(cNext - c) <= gdPrecision)
                break;
        } while(f(c) <= f(cNext));

        if(qAbs(cNext - c) <= gdPrecision)
            break;

        c = cNext;
        ++steps;

        qDebug() << "step: " << steps << " c: " << c;
    }

    qDebug() << "minimizeError completed after" << steps << "steps";
    qDebug() << "b =" << b << "c =" << cNext; // Or just c?
    qDebug() << "prev c = " << c;
    qDebug() << "f(c) = " << f(cNext);
    return TheoreticalModelParameters(dropType, b, c, precision);
}
