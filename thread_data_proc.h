#ifndef THREAD_DATA_PROC_H
#define THREAD_DATA_PROC_H

#include <QThread>
#include <QString>
#include "common.h"
#include "qcommon.h"


/*********************数据处理线程*****************************/
class DataProcessThread : public QThread
{
    Q_OBJECT
public:
    explicit DataProcessThread(QObject *parent = 0);
    void stop();
    void test();

protected:
    void run();

private:
    volatile bool stopped;
    SVM_TASK svm_task;
    bool excute_svm_task;

signals:
    void send_to_textbrower_display_output(QString str);

    void task_done();

public slots:
    void recei_fro_widget_svm_task(SVM_TASK svm_task_para);

private slots:

};


#endif // THREAD_DATA_PROC_H
