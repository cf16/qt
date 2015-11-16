#include "mpe_uint_validator.h"

Mpe_uint_validator::Mpe_uint_validator(unsigned long min,
                                       unsigned long max,
                                       QObject *parent) :
    QValidator(parent), min(min), max(max)
{
}

QValidator::State
Mpe_uint_validator::validate(
        QString &input, int &pos) const
{
    (void) pos; // unused, silence the warning,
                // we know what we are doing
    if (input.isEmpty())
        return Intermediate;
    bool b;
    unsigned int val = input.toUInt(&b);
    if ((b == true) && (val >= min) && (val <= max))
        return Acceptable;
    return Invalid;
}
