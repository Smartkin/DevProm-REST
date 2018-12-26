#ifndef ISSUE_H
#define ISSUE_H

#include<QString>

class Issue
{
public:
    QString id;
    QString caption;
    QString description;
    QString type;
    QString priority;
    QString estimation;
    QString function;
    QString owner;
    QString planned_release;
    QString author;
    QString start_date;
    QString finish_date;
    QString submitted_version;
    QString closed_in_version;
    QString order_num;
    QString record_created;
    QString record_modified;
    QString state;

};

#endif // ISSUE_H
