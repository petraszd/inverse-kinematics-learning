#include <QtWidgets>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->radioButtonEdit, SIGNAL(clicked(bool)), ui->ikWidget, SLOT(editModeToggled(bool)));
    connect(ui->radioButtonIK, SIGNAL(clicked(bool)), ui->ikWidget, SLOT(ikModeToggled(bool)));
}

MainWindow::~MainWindow()
{
    disconnect(ui->radioButtonEdit, SIGNAL(clicked(bool)), ui->ikWidget, SLOT(editModeToggled(bool)));
    disconnect(ui->radioButtonIK, SIGNAL(clicked(bool)), ui->ikWidget, SLOT(ikModeToggled(bool)));

    delete ui;
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Q || event->key() == Qt::Key_Escape) {
        QCoreApplication::quit();
    }
}
