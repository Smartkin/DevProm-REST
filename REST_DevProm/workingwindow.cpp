#include "workingwindow.h"
#include "ui_workingwindow.h"
#include "authwindow.h"
#include <QJsonObject>
#include <QCloseEvent>
#include <QJsonArray>

WorkingWindow::WorkingWindow(QWidget *parent, AuthWindow* holder) :
    QDialog(parent),
    ui(new Ui::WorkingWindow),
    api_holder(holder),
    req(new QJsonArray())
{
    ui->setupUi(this);

    connect(ui->btnGetTasks, &QAbstractButton::pressed, this, &WorkingWindow::GetTasks);
    connect(ui->btnSetTasks, &QAbstractButton::pressed, this, &WorkingWindow::SetTasks);
    connect(ui->btnLogout, &QAbstractButton::pressed, this, &QWidget::close);
    connect(ui->tableTasks, &QTableWidget::cellEntered, this, &WorkingWindow::AppendChange);
    connect(api_holder, &AuthWindow::ReceivedResponse, this, &WorkingWindow::HandleResponse);

    ui->tableTasks->setColumnCount(2);
    ui->tableTasks->setColumnWidth(0, 500);
    ui->tableTasks->setColumnWidth(1, 500);
    ui->tableTasks->setHorizontalHeaderLabels(QStringList({"Story name", "Description"}));
}

void WorkingWindow::GetTasks()
{
    api_holder->DoGetRequest("issues");
}

void WorkingWindow::SetTasks()
{
    api_holder->DoPostRequest("issues", req);
}

void WorkingWindow::HandleResponse(QJsonObject* resp)
{
    ui->tableTasks->setRowCount(ui->tableTasks->rowCount()+1);
    QTableWidgetItem* item = new QTableWidgetItem((*resp)["Caption"].toString());
    ui->tableTasks->setItem(ui->tableTasks->rowCount()-1, 0, item);
    item = new QTableWidgetItem((*resp)["Description"].toString());
    ui->tableTasks->setItem(ui->tableTasks->rowCount()-1, 1, item);
    ui->tableTasks->setRowHeight(ui->tableTasks->rowCount()-1, 200);
    delete resp;
}

void WorkingWindow::AppendChange(int row, int col)
{
    req->append(QJsonObject({{"Caption", ui->tableTasks->item(row, 0)->text()}}));
    req->append(QJsonObject({{"Description", ui->tableTasks->item(row, 1)->text()}}));
}

void WorkingWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
    emit Closed();
    QWidget::closeEvent(event);
}

WorkingWindow::~WorkingWindow()
{
    delete ui;
}
