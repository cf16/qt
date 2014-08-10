#ifndef MAPPING_H
#define MAPPING_H

#include <QString>

#include <vector>
#include <stdexcept>

struct Mapping
{
    struct Change {
        std::string oldName_;
        std::string newName_;
        size_t oldId_;
        size_t newId_;
    };

    std::vector< Change> columns_;
    std::vector< Change>       rows_;
    QString columnMapping_;
    QString identifierMapping_;

    struct MappingException : public std::runtime_error {
    public:
            MappingException (std::string const& msg):
                std::runtime_error(msg)
            {}
    };

    Mapping() {}
    Mapping(QString columnMapping, QString identifierMapping);
    void map() throw ( MappingException);
};

#endif // MAPPING_H
