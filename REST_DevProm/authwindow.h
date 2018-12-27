#ifndef AUTHWINDOW_H
#define AUTHWINDOW_H

#include <QMainWindow>

namespace Ui {
class AuthWindow;
}

class QNetworkAccessManager;
class QNetworkReply;
class WorkingWindow;
class QJsonObject;
class QJsonDocument;

class AuthWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AuthWindow(QWidget *parent = nullptr);
    ~AuthWindow();

    void DoGetRequest(QString uri);
    void DoPostRequest(QString uri, QJsonDocument* request);

private slots:
    void Auth();
    void ReplyFinished(QNetworkReply* reply);
    void HandleChild();

signals:
    void ReceivedResponse(QJsonObject* resp_obj);

private:
    Ui::AuthWindow *ui;
    QNetworkAccessManager *manager;
    QString api_key;
    QString proj_id;
    bool logged_in;
    WorkingWindow* ww;
};

#endif // AUTHWINDOW_H
