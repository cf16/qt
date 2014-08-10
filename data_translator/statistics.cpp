#include "statistics.h"

size_t Statistics::totalSuccessful() const
{
    size_t sum = 0;
    for ( size_t i = 0; i < NUMTHRDS; ++i) {
        sum += workDoneByThread[i];
    }
    return sum;
}

size_t Statistics::totalUnsuccessful() const
{
    size_t sum = 0;
    for ( size_t i = 0; i < NUMTHRDS; ++i) {
        sum += workNotDoneByThread[i];
    }
    return sum;
}

void Statistics::print( FilesDescription& filesDescription) const
{
    fprintf( stderr, "Number of files processed:\n");
    fprintf( stderr, "Successful: %ld | Thread", totalSuccessful());
    for ( size_t i = 0; i < NUMTHRDS; ++i) {
        fprintf( stderr, " %ld[%ld]", i, workDoneByThread[i]);
    }
    fprintf( stderr, "\nUnsuccesful: %ld | Thread", totalUnsuccessful());
    for ( size_t i = 0; i < NUMTHRDS; ++i) {
        fprintf( stderr, " %ld[%ld]", i, workNotDoneByThread[i]);
    }
    fprintf( stderr, "\n");
    if ( filesDescription.filesMissingNumber_ > 0)
    {
        fprintf( stderr, "Missing files: %d | Ids: ", filesDescription.filesMissingNumber_ );
        std::vector< int>::iterator it = filesDescription.filesMissing_.begin();
        do {
            fprintf( stderr, " %d", *it);
            ++it;
            if ( it != filesDescription.filesMissing_.end()) {
                fprintf( stderr, ",");
            } else {
                fprintf( stderr, "\n");
            }
        } while ( it != filesDescription.filesMissing_.end());
    }
}
