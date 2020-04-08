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
#include <QDoubleValidator>

#include <QDebug>

using namespace QtCharts;

static QColor colors[] = {Qt::red, Qt::green, Qt::blue, Qt::cyan, Qt::magenta};

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
    , m_chartsLayout(new QHBoxLayout)
    , m_inputb(new QLineEdit)
    , m_inputd(new QLineEdit)
    , m_chart(new QChartView)
    , m_dropType(new QComboBox)
{
    m_dropType->addItems({"Pendant", "Rotating"});
    m_chart->setMinimumSize(500, 500);
    m_inputb->setValidator(new QDoubleValidator(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), 1000));
    m_inputd->setValidator(new QDoubleValidator(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), 1000));
    //setFixedSize(500, 500);
	QWidget *w = new QWidget;
    w->setLayout(m_chartsLayout);
	setCentralWidget(w);
    QGridLayout *grid = new QGridLayout;
    grid->addWidget(new QLabel("Drop type: "), 0, 0);
    grid->addWidget(m_dropType, 0, 1);
    grid->addWidget(new QLabel("b: "), 1, 0);
    grid->addWidget(new QLabel("d: "), 2, 0);
    grid->setColumnStretch(1, 1);
    grid->addWidget(m_inputb, 1, 1);
    grid->addWidget(m_inputd, 2, 1);
    m_chartsLayout->addLayout(grid);
    QPushButton *button = new QPushButton("Input");
    grid->addWidget(button, 3, 0, 1, 2);
    connect(button, &QPushButton::clicked, this, &MainWindow::onInputButtonClicked);
    connect(m_inputb, &QLineEdit::returnPressed, this, &MainWindow::onInputButtonClicked);
    connect(m_inputd, &QLineEdit::returnPressed, this, &MainWindow::onInputButtonClicked);
    m_chartsLayout->addWidget(m_chart, 1);
}

static bool expectChar(QTextStream &in, char c)
{
	char ch;
	in >> ch;
	return c == ch;
}

/*
void MainWindow::addFile(const QString &file)
{
	QFile f(file);
	if (!f.open(QIODevice::ReadOnly)) {
		QMessageBox::critical(this, "Error", QString("Failed to open file %1: %2").arg(file).arg(f.errorString()));
		return;
	}
	QTextStream in(&f);
	Drop drop;
	try {
		while (!in.atEnd()) {
			if (!expectChar(in, '{'))
				throw;
			double x, y;
			in >> x;
			if (!expectChar(in, ','))
				throw;
			in >> y;
			if (!expectChar(in, '}'))
				throw;
			drop.m_points.append(QPointF(x, y));
			if (y > m_height)
				m_height = y;
			if (!expectChar(in, ','))
				break;
		}
	} catch(...) {
		QMessageBox::critical(this, "Error", "File format error");
		return;
	}

	m_drops.append(drop);
	update();
}
*/

void MainWindow::plot(QVector<QVector<QPointF>> things)
{
    qDebug() << "Plotting...";
    m_chart->chart()->removeAllSeries();
	for (auto points : things) {
		QLineSeries *series = new QLineSeries(this);

		for (auto p : points)
			series->append(p.x(), p.y());

        m_chart->chart()->addSeries(series);
    }
    m_chart->chart()->legend()->hide();
    m_chart->chart()->createDefaultAxes();
    m_chart->setRenderHint(QPainter::Antialiasing);
    qDebug() << "Done";
}

/*
void MainWindow::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	double width = m_maxX - m_minX;
	double height = m_maxY - m_minY;
	p.translate(0, this->height());
	p.scale(1, -1);
	double scaleX = this->height() / height;
	double scaleY = this->width() / width;
	for (int d = 0; d < m_drops.size(); ++d) {
		p.setPen(QPen(colors[d], 3));
		const Drop &drop = m_drops[d];
		for (int i = 1; i < drop.m_points.size(); ++i) {
			QPointF p1 = drop.m_points[i - 1] - QPointF(m_minX, m_minY);
			QPointF p2 = drop.m_points[i] - QPointF(m_minX, m_minY);
			p.drawLine(QLineF(p1 * scaleX, p2 * scaleY));
			qDebug() << QLineF(p1 * scaleX, p2 * scaleY);
		}
	}
}
*/

void MainWindow::onInputButtonClicked()
{
    qDebug() << "Generating model...";
    double b, d;
    b = m_inputb->text().replace(',', '.').toDouble();
    d = m_inputd->text().replace(',', '.').toDouble();
    QVector<QPointF> drop;
    double x0 = 0, z0 = 0, phi0 = 0;
    double x1, z1, phi1;
    const double h = 0.1;
    if(m_dropType->currentIndex() == 0)
    {
        for (;;)
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
                phi1 = phi0 + h * (2*b + d*z0 - sin(phi0)/x0);
            }
            x0 = x1;
            z0 = z1;
            phi0 = phi1;
        }
    }
    else
    {
        drop.append(QPointF(x0, z0));
        for (;;)
        {
            x1 = x0 + h * cos(phi0);
            z1 = z0 + h * sin(phi0);
            if(x0 == 0)
            {
                phi1 = phi0 + h * b;
            }
            else
            {
                phi1 = phi0 + h * (2*b + d*x0*x0 - sin(phi0)/x0);
            }
            x0 = x1;
            z0 = z1;
            phi0 = phi1;
            drop.append(QPointF(x0, z0));
            if (x0 <= std::numeric_limits<double>::epsilon())
                break;
        }
    }
    qDebug() << "Done";

    plot({drop});

}
