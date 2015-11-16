///  @file      mpe_uint_validator.h
///  @brief     Validator of inputs of type unsigned integer.
///  @author    peterg at sytel.com
///  @date      03 Nov 2015 3:21 PM
///  @copyright GPL2

#ifndef MPE_UINT_VALIDATOR_H
#define MPE_UINT_VALIDATOR_H

#include <QValidator>

class Mpe_uint_validator : public QValidator
{
    Q_OBJECT
public:
    explicit Mpe_uint_validator(unsigned long min,
                                unsigned long max,
                                QObject *parent = NULL);
    virtual State validate(QString &input, int &pos) const;

private:
    unsigned long min, max;

signals:

public slots:

};

#endif // MPE_UINT_VALIDATOR_H
