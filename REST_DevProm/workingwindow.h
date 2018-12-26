#ifndef WORKINGWINDOW_H
#define WORKINGWINDOW_H

#include <QDialog>

namespace Ui {
class WorkingWindow;
}

class AuthWindow;
class QJsonObject;
class QCloseEvent;
class QJsonArray;
class Issue;

class WorkingWindow : public QDialog
{
    Q_OBJECT

public:
    explicit WorkingWindow(QWidget *parent = nullptr, AuthWindow *holder = nullptr);
    ~WorkingWindow();

protected:
    void closeEvent(QCloseEvent *event);


private slots:
    void GetTasks();
    void SetTasks();
    void HandleResponse(QJsonObject *resp);
    void AppendChange(int row, int col);
    void SetChange(int row, int col);

signals:
    void Closed();

private:
    Ui::WorkingWindow *ui;
    AuthWindow* api_holder;
    QJsonArray* req;
    QList<Issue*> issue_list;
    bool change;
};

#endif // WORKINGWINDOW_H
