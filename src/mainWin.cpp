#include "mainWin.h"
#include "./ui_mainWin.h"

MainWin::MainWin(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWin)
{
    ui->setupUi(this);
    ui->ipAddress->setFocus();
    ui->btnStop->setEnabled(false);

	connect(ui->btnStart, SIGNAL(clicked()), this, SLOT(BtnStartClicked()));
	connect(ui->btnStop, SIGNAL(clicked()), this, SLOT(BtnStopClicked()));
}

void MainWin::BtnStartClicked() {
    ui->btnStart->setEnabled(false);
    ui->btnStop->setEnabled(true);
}

void MainWin::BtnStopClicked() {
    ui->btnStop->setEnabled(false);
    ui->btnStart->setEnabled(true);
}


MainWin::~MainWin()
{
    delete ui;
}

