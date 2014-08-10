#ifndef STATISTICS_H
#define STATISTICS_H

#include <stdlib.h>

#include "filesdescription.h"

#define NUMTHRDS 4

/**
 * @brief The Statistics struct
 * This structure accumulates information
 * about files being processed
 */
struct Statistics
{
    size_t workDoneByThread[NUMTHRDS];
    size_t workNotDoneByThread[NUMTHRDS];
    size_t totalSuccessful() const;
    size_t totalUnsuccessful() const;
    void print( FilesDescription& filesDescription) const;

    /* this isn't necessary here as global instance
     * is zero initialized anyway, however we can
     * very easy imagine the case when it is
     */
    Statistics() {
        for ( size_t i = 0; i < NUMTHRDS; ++i) {
            workDoneByThread[i] = 0;
            workNotDoneByThread[i] = 0;
        }
    }
};

#endif // STATISTICS_H
