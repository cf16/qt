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
