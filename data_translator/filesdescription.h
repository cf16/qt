#ifndef FILESDESCRIPTION_H
#define FILESDESCRIPTION_H

#include <QStringList>
#include <QDir>

#include <vector>

/*
The following structure contains the necessary information
to allow threads for files access and to sync threads.
*/

struct FilesDescription
{
    QStringList              names_;
    int             filesToProcess_;
    int         filesMissingNumber_;
    QDir                 directory_;
    std::vector< int> filesMissing_;

    int fileIdFromName( QString fileName) const;

    FilesDescription() {}
    FilesDescription( QString folderLocation) : directory_( folderLocation)
    {
        QStringList nameFilter("dataFile*.tsv");
        /* sorted list of files */
        names_ = directory_ .entryList( nameFilter);
        filesToProcess_ = names_.size();
        if ( filesToProcess_) {
        filesMissing_.resize( fileIdFromName( names_.back()));
        std::vector< int>::iterator it = filesMissing_.begin();
        filesMissingNumber_ = missingFiles( it);
        filesMissing_.resize( filesMissingNumber_);
        } else {
            filesMissingNumber_ = 0;
        }
    }

    size_t missingFiles(std::vector< int>::iterator out) const;
};

#endif // FILESDESCRIPTION_H
