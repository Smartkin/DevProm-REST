#include "workingwindow.h"
#include "ui_workingwindow.h"
#include "authwindow.h"
#include "issue.h"
#include <QJsonObject>
#include <QCloseEvent>
#include <QJsonArray>
#include <QJsonDocument>
#include <QList>

WorkingWindow::WorkingWindow(QWidget *parent, AuthWindow* holder) :
    QDialog(parent),
    ui(new Ui::WorkingWindow),
    api_holder(holder),
    req(new QJsonArray()),
    change(false)
{
    ui->setupUi(this);

    connect(ui->btnGetTasks, &QAbstractButton::pressed, this, &WorkingWindow::GetTasks);
    connect(ui->btnSetTasks, &QAbstractButton::pressed, this, &WorkingWindow::SetTasks);
    connect(ui->btnLogout, &QAbstractButton::pressed, this, &QWidget::close);
    connect(ui->tableTasks, &QTableWidget::cellChanged, this, &WorkingWindow::AppendChange);
    connect(ui->tableTasks, &QTableWidget::cellDoubleClicked, this, &WorkingWindow::SetChange);
    connect(api_holder, &AuthWindow::ReceivedResponse, this, &WorkingWindow::HandleResponse);

    ui->tableTasks->setColumnCount(3);
    ui->tableTasks->setColumnWidth(0, 500);
    ui->tableTasks->setColumnWidth(1, 500);
    ui->tableTasks->setHorizontalHeaderLabels(QStringList({"Id", "Story name", "Description"}));
}

void WorkingWindow::GetTasks()
{
    api_holder->DoGetRequest("issues");
}

void WorkingWindow::SetTasks()
{
    for(auto obj : *req)
    {
        QJsonDocument* doc_req = new QJsonDocument(obj.toObject());
        api_holder->DoPostRequest("issues", doc_req);
    }
    //Create new request because api_holder will delete it after sending
    req = new QJsonArray();
}

void WorkingWindow::HandleResponse(QJsonObject* resp)
{
    QJsonObject& ref = *resp;
    Issue* issue = new Issue;
    issue->author = ref["Author"].toString();
    issue->caption = ref["Caption"].toString();
    issue->closed_in_version = ref["ClosedInVersion"].toString();
    issue->description = ref["Description"].toString();
    issue->estimation = ref["Estimation"].toString();
    issue->finish_date = ref["FinishDate"].toString();
    issue->function = ref["Function"].toString();
    issue->id = ref["Id"].toString();
    issue->order_num = ref["OrderNum"].toString();
    issue->owner = ref["Owner"].toString();
    issue->planned_release = ref["PlannedRelease"].toString();
    issue->priority = ref["Priority"].toString();
    issue->record_created = ref["RecordCreated"].toString();
    issue->record_modified = ref["RecordModified"].toString();
    issue->start_date = ref["StartDate"].toString();
    issue->state = ref["State"].toString();
    issue->submitted_version = ref["SubmittedVersion"].toString();
    issue->type = ref["Type"].toString();

    ui->tableTasks->setRowCount(ui->tableTasks->rowCount()+1);
    QTableWidgetItem* item = new QTableWidgetItem((*resp)["Id"].toString());
    ui->tableTasks->setItem(ui->tableTasks->rowCount()-1, 0, item);
    item = new QTableWidgetItem((*resp)["Caption"].toString());
    ui->tableTasks->setItem(ui->tableTasks->rowCount()-1, 1, item);
    item = new QTableWidgetItem((*resp)["Description"].toString());
    ui->tableTasks->setItem(ui->tableTasks->rowCount()-1, 2, item);
    ui->tableTasks->setRowHeight(ui->tableTasks->rowCount()-1, 200);
    issue_list.append(std::move(issue));
    delete resp;
}

void WorkingWindow::AppendChange(int row, int col)
{
    if (change)
    {
        qDebug("Changes recorded...");
        Issue& is = *issue_list[row];
        is.id = ui->tableTasks->item(row, 0)->text();
        is.caption = ui->tableTasks->item(row, 1)->text();
        is.description = ui->tableTasks->item(row, 2)->text();

        req->append(QJsonObject({
                                    {"Id", is.id},
                                    {"Caption", is.caption},
                                    {"Description", is.description},
                                    {"Type", is.type},
                                    {"Priority", is.priority},
                                    {"Estimation", is.estimation},
                                    {"Function", is.function},
                                    {"Owner", is.owner},
                                    {"PlannedRelease", is.planned_release},
                                    {"Author", is.author},
                                    {"StartDate", is.start_date},
                                    {"FinishDate", is.finish_date},
                                    {"SubmittedVersion", is.submitted_version},
                                    {"ClosedInVersion", is.closed_in_version},
                                    {"OrderNum", is.order_num},
                                    {"RecordCreated", is.record_created},
                                    {"RecordModified", is.record_modified},
                                    {"State", is.state}
                                }));
        change = false;
    }
}

void WorkingWindow::SetChange(int row, int col)
{
    change = true;
}

void WorkingWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
    emit Closed();
    QWidget::closeEvent(event);
}

WorkingWindow::~WorkingWindow()
{
    qDebug("WW deleted");
    for(auto is : issue_list) delete is;
    delete ui;
    if (req) delete req;
}
