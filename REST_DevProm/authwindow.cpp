#include "authwindow.h"
#include "workingwindow.h"
#include "ui_authwindow.h"
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QMessageBox>
#include <sstream>

AuthWindow::AuthWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AuthWindow),
    manager(new QNetworkAccessManager(this)),
    api_key(""),
    proj_id(""),
    logged_in(false),
    ww(nullptr)
{

    ui->setupUi(this);

    connect(ui->btnAuth, &QAbstractButton::pressed, this, &AuthWindow::Auth);
    connect(manager, &QNetworkAccessManager::finished, this, &AuthWindow::ReplyFinished);
}

void AuthWindow::Auth()
{
    //Get the user entered api-key and their project id
    QString api_key_ = ui->leApiField->text();
    QString proj_id_ = ui->leProjectId->text();

    //Form and send GET request for a test request to check API key
    QNetworkRequest req;
    req.setHeader(QNetworkRequest::KnownHeaders::ContentLengthHeader, QVariant(0));
    req.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, QVariant("application/json"));
    req.setRawHeader("Devprom-Auth-Key", api_key_.toStdString().c_str());
    req.setUrl(QUrl("http://alm.mtuci.ru/pm/"+proj_id_+"/api/v1/issues"));
    manager->get(req);
    api_key = api_key_;
    proj_id = proj_id_;
}

void AuthWindow::DoGetRequest(QString uri)
{
    qDebug("API-KEY: " + api_key.toLatin1());
    qDebug("Project ID: " + proj_id.toLatin1());
    QNetworkRequest req;
    req.setHeader(QNetworkRequest::KnownHeaders::ContentLengthHeader, QVariant(0));
    req.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, QVariant("application/json"));
    req.setRawHeader("Devprom-Auth-Key", api_key.toStdString().c_str());
    req.setUrl(QUrl("http://alm.mtuci.ru/pm/"+proj_id+"/api/v1/"+uri));
    manager->get(req);
}

void AuthWindow::DoPostRequest(QString uri, QJsonDocument* request)
{
    QJsonDocument req_doc(*request);
    qDebug(req_doc.toJson(QJsonDocument::Indented));
    std::stringstream str;
    str << req_doc.toBinaryData().size();
    std::string num_str = str.str();
    qDebug("Size:");
    qDebug(num_str.c_str());
    QNetworkRequest req;
    req.setHeader(QNetworkRequest::KnownHeaders::ContentLengthHeader, QVariant(req_doc.toJson().size()));
    req.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, QVariant("application/json; charset=utf-8"));
    req.setRawHeader("Devprom-Auth-Key", api_key.toStdString().c_str());
    req.setUrl(QUrl("http://alm.mtuci.ru/pm/"+proj_id+"/api/v1/"+uri));
    manager->post(req, req_doc.toJson());
    delete request;
}

void AuthWindow::DoDeleteRequest(QString uri, QString id)
{
    QNetworkRequest req;
    req.setHeader(QNetworkRequest::KnownHeaders::ContentLengthHeader, QVariant(0));
    req.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, QVariant("application/json; charset=utf-8"));
    req.setRawHeader("Devprom-Auth-Key", api_key.toStdString().c_str());
    req.setUrl(QUrl("http://alm.mtuci.ru/pm/"+proj_id+"/api/v1/"+uri+"/"+id));
    manager->deleteResource(req);
}

void AuthWindow::ReplyFinished(QNetworkReply* reply)
{
    qDebug("Got data");
    auto data_size = reply->size();
    std::stringstream str;
    str << data_size;
    std::string num;
    num = str.str();
    //Read the reply
    QString str_rep = reply->readAll();
    qDebug("Data size:");
    qDebug(num.c_str());
    if (!logged_in)
    { //Check test request's result
        if (data_size == 0)
        {
            QMessageBox msg_box;
            msg_box.setWindowTitle("Request error!");
            msg_box.setText("Test request did not succeed! Check the validity of your API key");
            msg_box.setStandardButtons(QMessageBox::Ok);
            msg_box.exec();
        }
        else
        {
            if (ww) delete ww;
            ww = new WorkingWindow(this, this);
            ww->show();
            connect(ww, &WorkingWindow::Closed, this, &AuthWindow::HandleChild);
            logged_in = true;
        }
    }
    else // Parse answer and emit the result
    {
        QJsonParseError er;
        QJsonDocument json_resp = QJsonDocument::fromJson(str_rep.toUtf8(), &er);
        //Check for any parsing errors
        if (er.error != QJsonParseError::NoError)
        {
            qDebug(er.errorString().toLatin1());
        }
        if (json_resp.isArray())
        {
            QJsonArray resp_ar = json_resp.array();
            //Emit response for every entry
            for (auto o : resp_ar)
            {
                QJsonObject* resp_obj = new QJsonObject(o.toObject());
                emit ReceivedResponse(resp_obj);
            }
        }
        else
        {
            QJsonObject* resp_obj = new QJsonObject(json_resp.object());
            emit ReceivedResponse(resp_obj);
        }
    }
    //Print out full reply in debug
    auto headers = reply->rawHeaderPairs();
    for(int i = 0; i < headers.size(); ++i)
    {
       qDebug(headers[i].first + " " + headers[i].second);
    }
    qDebug(str_rep.toLatin1());
}

void AuthWindow::HandleChild()
{
    qDebug("Child closed.");
    disconnect(ww, &WorkingWindow::Closed, this, &AuthWindow::HandleChild);
    logged_in = false;
    api_key = "";
    proj_id = "";
}

AuthWindow::~AuthWindow()
{
    delete ui;
    delete manager;
    if (ww) ww->close();
}
