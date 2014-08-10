#include "filesdescription.h"

/**
 * @brief FilesDescription::fileIdFromName
 * returns int value of id
 * file name has to contain "." !
 * @param fileName
 * @return
 */
int FilesDescription::fileIdFromName( QString fileName) const
{
    std::string name = fileName.toStdString();
    return atoi( &name[name.find(".") - 1]);
}

int intNext( int i) {
    (void)i;
    static int i_ = 0;
    return i_++;
}

size_t FilesDescription::missingFiles( std::vector< int>::iterator out) const
{
    int maxId = fileIdFromName( names_.back());
    std::vector< int> idsRe;                 // actual, observed
    std::vector< int> idsIm( maxId + 1);         // hypothetical
    std::vector< int> idsDifference( maxId); // set difference
    idsRe.reserve( filesToProcess_);

    QStringList::ConstIterator qIt = names_.begin();
    while ( qIt != names_.end()) {
        idsRe.push_back( fileIdFromName( *qIt++));
    }

    /* fill hypothetical numbers */
    std::transform( idsIm.begin(), idsIm.end(), idsIm.begin(), intNext);

    /* compute set difference */
    std::vector< int>::iterator it;
    it = std::set_difference( idsIm.begin(), idsIm.end(),
                              idsRe.begin(), idsRe.end(), out);
    return ( it - out);
}
