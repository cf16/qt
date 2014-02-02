#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <sstream>

#include "mapping.h"

/*
The following structure contains the necessary information
to allow threads for files access and to sync threads.
*/

struct FilesDescription
{
    QStringList             names_;
    int            filesToProcess_;
    QDir                directory_;
    int fileIdFromName( QString fileName);
};

/* Define globally accessible variables and a mutex */
#define NUMTHRDS 4
FilesDescription filesDescription_;
int newNumberOfCols;
int newNumberOfRows;
pthread_t callThd[NUMTHRDS];
pthread_mutex_t mutexFilesDescription;
Mapping mapping;

/**
 * @brief convertFile
 * Extract desired fields from file and write new output.
 * @param fileName
 * @param mapping
 * @param fileNumber
 * @return
 */
int convertFile( QString fileName, Mapping const& mapping, int fileNumber)
{
    using namespace std;
    ifstream in( fileName.toStdString().c_str());
    if ( !in.is_open()) return -1;

    vector< string> vec;
    vec.reserve( mapping.columns_.back().oldId_);
    string line;
    vector< Mapping::Change>::const_iterator rowsIt = mapping.rows_.begin();
    size_t nextRow = (*rowsIt).oldId_;
    size_t currRow = 0;

    stringstream s;
    s << "outputFile" << fileNumber << ".tsv";;
    string newFileName = s.str();
    ofstream newFile( newFileName.c_str());
    if ( !newFile.is_open()) return -1;

    while ( !( currRow > mapping.rows_.back().oldId_) && getline( in, line))
    {
        /* if this is column header */
        if ( currRow == 0) {
            vector< Mapping::Change>::const_iterator it = mapping.columns_.begin();
            while ( it != mapping.columns_.end() - 1) {
                newFile << (*it).newName_ << "\t";
                ++it;
            }
            newFile << (*it).newName_;
        }

        if ( (newNumberOfCols > 0) && (currRow == nextRow)) {
            vec.clear();
            newFile << endl;
            istringstream iss( line);
            copy( istream_iterator<string>(iss), istream_iterator<string>(), back_inserter( vec));
            vector< string>::const_iterator vecIt = vec.begin();
            int i = 0;
            unsigned int columnId;
            for ( ; i < newNumberOfCols - 1; ++i) {
                columnId = mapping.columns_[i].oldId_;
                if ( columnId > vec.size() - 1) continue;
                if ( columnId == 0) {
                    newFile << (*rowsIt).newName_ << "\t";
                } else {
                    newFile << vec[columnId]<< "\t";
                }
            }
            columnId = mapping.columns_[i].oldId_;
            ++rowsIt;
            nextRow = (*rowsIt).oldId_;
            if ( columnId > vec.size() - 1) {
                ++currRow;
                continue;
            }
            newFile << vec[ columnId];

        }
        ++currRow;
    }
    return 0;
}

/**
 * @brief processFile
 * Threads routine. Process each file to extract desired fields.
 * @param arg
 * @return
 */
void *processFile(void *arg)
{
    long threadId = ( long) arg;

    /* Lock a mutex prior to updating the value in the shared
     *structure, and unlock it upon updating.
    */
    while ( true) {
        pthread_mutex_lock (&mutexFilesDescription);
        int fileNumber = filesDescription_.filesToProcess_;
        if ( fileNumber < 1) {
            pthread_mutex_unlock (&mutexFilesDescription);
            break;
        }
        --fileNumber;
        filesDescription_.filesToProcess_ = fileNumber;
        pthread_mutex_unlock (&mutexFilesDescription);

        /* do the task */
        QString fileName = filesDescription_.names_[fileNumber];
        int fileIdFromName = filesDescription_.fileIdFromName( fileName);
        int y = convertFile( filesDescription_.directory_ .absolutePath() + "/" + filesDescription_.names_[fileNumber],
                             mapping, fileIdFromName);

        printf( "Thread %ld did\n", threadId);
        if ( fileNumber == 0) break;
    }
    printf( "Thread %ld exit\n", threadId);
    pthread_exit((void*) 0);
}

/*
The main program creates threads which do all the work and then
print out result upon completion. Before creating the threads,
The input data is created. Since all threads update a shared structure, we
need a mutex for mutual exclusion. The main thread needs to wait for
all threads to complete, it waits for each one of the threads. We specify
a thread attribute value that allow the main thread to join with the
threads it creates. Note also that we free up handles  when they are
no longer needed.
*/

int main (int argc, char *argv[])
{
    if ( !(argc == 2)) {
        fprintf( stderr, "Program should be called with 1 argument but %d given.\n", argc-1);
        if ( argc < 2) {
            fprintf( stderr, "Exiting with code -1.\n");
            return -1;
        }
        fprintf( stderr, "Using first argument as folder location.\n");
    }
    QString folderLocation = argv[1];

    QCoreApplication app(argc, argv);

    /* get files info and initialize global structure */
    QStringList nameFilter("dataFile*.tsv");
    filesDescription_.directory_ = QDir( folderLocation);
    filesDescription_.names_ = filesDescription_.directory_ .entryList( nameFilter);
    filesDescription_.filesToProcess_ = filesDescription_.names_.size();

    QString columnMapping = filesDescription_.directory_ .absolutePath() + "/" + "column_mapping.tsv";
    QString identifierMapping = filesDescription_.directory_ .absolutePath() + "/" + "identifier_mapping.tsv";
    mapping = Mapping( columnMapping, identifierMapping);

    try {
        mapping.map();
    } catch ( std::exception& e) {
        fprintf( stderr, e.what());
        return -1;
    }

    newNumberOfCols = mapping.columns_.size();
    newNumberOfRows = mapping.rows_.size() + 1;
    int y = convertFile( filesDescription_.directory_ .absolutePath() + "/" + filesDescription_.
                         names_[1], mapping, 1);

    /* create threads */
    long i;
    void *status;
    pthread_attr_t attr;

    pthread_mutex_init(&mutexFilesDescription, NULL);

    /* Create threads to perform fields extraction  */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for( i = 0; i < NUMTHRDS; ++i)
    {
        /* Each thread works on a different set of data.
         * Threads will process files starting from the last one
         * and ending on the file with least identifier
         * (number in the name)
        */
        pthread_create(&callThd[i], &attr, processFile, (void *)i);
    }

    pthread_attr_destroy(&attr);


    /* Wait for the other threads */
    for( i = 0; i < NUMTHRDS; ++i) {
        pthread_join(callThd[i], &status);
    }
    /* After joining, print out the results and cleanup */
    pthread_mutex_destroy(&mutexFilesDescription);
    pthread_exit(NULL);

    return 0;
}

/**
 * @brief FilesDescription::fileIdFromName
 * returns int value of id
 * file name has to contain "." !
 * @param fileName
 * @return
 */
int FilesDescription::fileIdFromName( QString fileName)
{
    std::string name = fileName.toStdString();
    return atoi( &name[name.find(".") - 1]);
}
