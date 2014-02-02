#include "mapping.h"

#include <iterator>
#include <fstream>
#include <sstream>

Mapping::Mapping( QString columnMapping, QString identifierMapping) :
    columnMapping_( columnMapping),
    identifierMapping_( identifierMapping)
{
}

void Mapping::map() throw ( MappingException)
{
    using namespace std;

    /* column mapping */
    ifstream in( columnMapping_.toStdString().c_str(), ios_base::in);
    if ( !in.is_open()) throw MappingException( "Cannot open column mapping file.");

    vector< string> vec;
    string line;
    size_t id = 0;

    while ( getline( in, line))
    {
        Change change;
        istringstream iss( line);
        iss >> change.oldName_;
        iss >> change.newName_;
        change.oldId_ = atoi( &change.oldName_[ change.oldName_.size() - 1]);
        change.newId_ = id++;
        columns_.push_back( change);
    }
    in.close();

    /* identifiers mapping */
    in.open( identifierMapping_.toStdString().c_str(), ios_base::in);
    if ( !in.is_open()) throw MappingException( "Cannot open identifier mapping file.");
    id = 0;

    while ( getline( in, line))
    {
        Change change;
        istringstream iss( line);
        iss >> change.oldName_;
        iss >> change.newName_;
        change.oldId_ = atoi( &change.oldName_[ change.oldName_.size() - 1]);
        change.newId_ = id++;
        rows_.push_back( change);
    }
    in.close();
}
