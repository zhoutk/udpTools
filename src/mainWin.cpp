#include "mainWin.h"

static void ParseDatagrams(QByteArray& d);

MainWin::MainWin(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWin)
    , udpTimer(this), lon(61.0), lat(-32.5), speeDeta(0.0001)
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
    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);

	const char head[] = { 0x90, 0x26 };
	s.writeRawData(head, 2);
	const char len[] = { 0x00, 0x44 };
	s.writeRawData(len, 2);
	const char sender[] = { 0x00, 0x00, 0x90, 0x01 };
	s.writeRawData(sender, 4);
	const char recver[] = { 0xFF, 0xFF, 0xFF, 0xFF };
	s.writeRawData(recver, 4);
	const char gpsState[] = { 0x00, 0x01, 0x00, 0x00 };             //add memo
	s.writeRawData(gpsState, 4);

    s << (int)(lon * 600000.0);
    s << (int)(lat * 600000.0);
	lon += qSin(ui->direction->text().toDouble() * M_PI / 180) * speeDeta * ui->speed->text().toDouble();
	lat += qCos(ui->direction->text().toDouble() * M_PI / 180) * speeDeta * ui->speed->text().toDouble();

	const char height[] = { 0x00, 0x00, 0x00, 0x2e };            
	s.writeRawData(height, 4);

    QTime nTime = QTime::currentTime();

	s << (short)nTime.hour();
	s << (short)nTime.minute();
	s << (short)nTime.second();

	const char speedState[] = { 0x00, 0x01 };
	s.writeRawData(speedState, 2);

	const char speedToWater[] = { 0x00, 0x00, 0x00, 0x02 };
	s.writeRawData(speedToWater, 4);

	s << (int)(ui->speed->text().toInt());			//speed to ground

	const char drectionToGroud[] = { 0x00, 0x00, 0x00, 0x00 };
	s.writeRawData(drectionToGroud, 4);

	const char lastGroup[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	s.writeRawData(lastGroup, 20);

    udpTransceiver->SendDataNow(data, ui->ipAddress->text(), ui->ipPort->text().toInt());
}

void MainWin::BtnSpinChange(int value) {
    udpTimer.stop();
    udpTimer.start(ui->spinBox->text().toInt() * 1000);
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

