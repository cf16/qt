#include <QCoreApplication>
#include <QRunnable>
#include <QThreadPool>
#include <QThread>
#include <QDebug>

// hellothreadpool/main.cpp
 class Work : public QRunnable
 {
 public:
     void run()
     {
         qDebug() << "Hello from thread " << QThread::currentThread();
     }
 };

 int main(int argc, char *argv[])
 {
     QCoreApplication app(argc, argv);
     QThreadPool *threadPool = QThreadPool::globalInstance();

         Work work;
         work.setAutoDelete(false);
         threadPool->start(&work);

     qDebug() << "hello from GUI thread " << QThread::currentThread();
     qDebug() << "ideal thread count " << QThread::idealThreadCount();
     threadPool->waitForDone();
     return 0;
 }
