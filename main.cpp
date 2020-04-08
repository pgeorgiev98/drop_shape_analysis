#include "mainwindow.h"

#include <QApplication>
#include <QtMath>
#include <QDebug>

int main(int argc, char *argv[])
{

	QApplication a(argc, argv);
	MainWindow w;
    /*
	const double h = QString(argv[1]).toDouble();
	const int n = QString(argv[2]).toInt();
	const double b = QString(argv[3]).toDouble();
	const double d = QString(argv[4]).toDouble();

	// u'(x) = e^-x
	// u(0) = 1

	// lim h->0 (u(x + h) - u(x) ) / h = u'(x)

	// yi = u(xi)
	// (y_i+1 - y_i) / h = fi = f(x_i, u(xi))
	// y_i+1 = f_i * h + y_i
	// f_i = f(x_i, u(xi)) = e^-x_i


	QVector<QPointF> exact, approx, error;
	double x = 0;
	double y0 = 0;
	double y1;
	while (x < 10) {
		approx.append(QPointF(x, y0));
		exact.append(QPointF(x, qPow(M_E, x) - 1));
		error.append(QPointF(x, qAbs(approx.last().y() - exact.last().y())));
		y1 = qPow(M_E, x) * h + y0;
		x = x + h;
		y0 = y1;
	}

	QVector<QPointF> drop;
	double x0 = 0, z0 = 0, phi0 = 0;
	double x1, z1, phi1;
	while(x0 <= 1)
	{
		drop.append(QPointF(x0, z0));
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

//	w.plot({drop});
	//w.plot({exact, approx});
	//w.plot({error});
*/
	w.show();
	return a.exec();
}
