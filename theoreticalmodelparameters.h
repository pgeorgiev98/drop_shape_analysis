#ifndef THEORETICALMODELPARAMETERS_H
#define THEORETICALMODELPARAMETERS_H

#include <QMetaType>

struct TheoreticalModelParameters
{
    enum DropType {PENDANT, SPINNING};
    Q_ENUMS(DropType)

    DropType dropType;
    double b, c;
    double precision;
    int cutoffMoment;

    TheoreticalModelParameters(DropType dropType, double b, double c, double precision, int cutoffMoment)
        : dropType(dropType), b(b), c(c), precision(precision), cutoffMoment(cutoffMoment)
    {}

    TheoreticalModelParameters()
    {}
};

Q_DECLARE_METATYPE(TheoreticalModelParameters)

#endif // THEORETICALMODELPARAMETERS_H
