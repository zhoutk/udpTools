#include "mainWin.h"

MainWin::MainWin(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWin)
    , udpTimer(this)
{
    ui->setupUi(this);
    ui->ipAddress->setFocus();
    ui->btnStop->setEnabled(false);

	connect(ui->btnStart, SIGNAL(clicked()), this, SLOT(BtnStartClicked()));
	connect(ui->btnStop, SIGNAL(clicked()), this, SLOT(BtnStopClicked()));
    connect(ui->spinBox, SIGNAL(valueChanged(int)), this, SLOT(BtnSpinChange(int)));

	connect(&udpTimer, SIGNAL(timeout()), this, SLOT(SendUdpPackageOnTime()));
    udpTimer.start(ui->spinBox->text().toInt() * 1000);
}

void MainWin::SendUdpPackageOnTime() {
	
}

void MainWin::BtnSpinChange(int value) {

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

