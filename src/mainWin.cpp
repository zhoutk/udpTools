#include "mainWin.h"

static void ParseDatagrams(QByteArray& d);

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

	udpTransceiver = new UDPTransceiver(this);
    udpTransceiver->SetDatagramProcessor(ParseDatagrams);

	connect(&udpTimer, SIGNAL(timeout()), this, SLOT(SendUdpPackageOnTime()));
}

void MainWin::SendUdpPackageOnTime() {
    QByteArray data = "test data.";
    udpTransceiver->SendDataNow(data, ui->ipAddress->text(), ui->ipPort->text().toInt());
}

void MainWin::BtnSpinChange(int value) {

}

void MainWin::BtnStartClicked() {
    ui->btnStart->setEnabled(false);
    ui->btnStop->setEnabled(true);

	udpTimer.start(ui->spinBox->text().toInt() * 1000);
}

void MainWin::BtnStopClicked() {
    ui->btnStop->setEnabled(false);
    ui->btnStart->setEnabled(true);

    udpTimer.stop();
}

void ParseDatagrams(QByteArray& d)
{
}

MainWin::~MainWin()
{
    udpTimer.stop();
    delete ui;
    delete udpTransceiver;
}

