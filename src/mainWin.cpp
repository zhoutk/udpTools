#include "mainWin.h"

static void ParseDatagrams(QByteArray& d);

MainWin::MainWin(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWin)
	, speeDeta(0.0001), udpTimer(this), radarUdpTimer(this)
{
    ui->setupUi(this);
	udpTransceiver = new UDPTransceiver(this);
	udpTransceiver->SetDatagramProcessor(ParseDatagrams);
    ui->ipAddress->setFocus();
    ui->btnStop->setEnabled(false);
	ui->btnRadarStop->setEnabled(false);
	ui->btnAisStop->setEnabled(false);

	lon = ui->Lon->text().toDouble();
	lat = ui->Lat->text().toDouble();
	connect(ui->btnStart, SIGNAL(clicked()), this, SLOT(BtnStartClicked()));
	connect(ui->btnStop, SIGNAL(clicked()), this, SLOT(BtnStopClicked()));
    connect(ui->spinBox, SIGNAL(valueChanged(int)), this, SLOT(BtnSpinChange(int)));
	connect(&udpTimer, SIGNAL(timeout()), this, SLOT(SendUdpPackageOnTime()));

	connect(ui->btnRadarStart, SIGNAL(clicked()), this, SLOT(BtnRadarStartClicked()));
	connect(ui->btnRadarStop, SIGNAL(clicked()), this, SLOT(BtnRadarStopClicked()));
	connect(ui->radarSpinBox, SIGNAL(valueChanged(int)), this, SLOT(BtnRadarSpinChange(int)));
	connect(&radarUdpTimer, SIGNAL(timeout()), this, SLOT(RadarSendUdpPackageOnTime()));
	connect(ui->btnRadarNew, SIGNAL(clicked()), this, SLOT(BtnRadarCreateClicked()));
}

void MainWin::BtnRadarCreateClicked() {
	QStringList idstr = ui->radarId->text().split("_");
	int id = idstr[1].toInt();
	QString str = QString("%1").arg(id + 1, 4, 10, QLatin1Char('0'));
	ui->radarId->setText(QString("RA_").append(str));
}

void MainWin::BtnRadarStartClicked() {
	ui->btnRadarStart->setEnabled(false);
	ui->btnRadarStop->setEnabled(true);

	radarUdpTimer.start(ui->radarSpinBox->text().toInt() * 1000);
}

void MainWin::BtnRadarStopClicked() {
	ui->btnRadarStop->setEnabled(false);
	ui->btnRadarStart->setEnabled(true);

	radarUdpTimer.stop();
}

void MainWin::RadarSendUdpPackageOnTime() {
	QStringList idstr = ui->radarId->text().split("_");
	int id = idstr[1].toInt();
	QString dataPacket = QString("UdPbC0\s:RA%1,n:%2*HH\$GETTM,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15,%16,%17*hh\r\n")
		.arg(ui->radarId->text().remove("_"))
		.arg(id)
		.arg(id)
		.arg(ui->radarSpeed->text().toInt() * 1000)											//2������
		.arg(ui->RadarDirection->text()) 										//3����λ
		.arg("T")										//4�����ԣ����
		.arg(6)												//5���ٶ�
		.arg(45)		//6������
		.arg("R")		//7�����ԣ���Ժ���ָʾ
		.arg("0.01")		//8��CPA
		.arg("0.02")	//9��TCPA
		.arg("N")		//10��Ŀ���ٶȾ��뵥λ
		.arg("")			//11��Ŀ������
		.arg("L")		//12������״̬
		.arg("")			//13��δ��
		.arg("")				//14��ʱ��
		.arg("A")			//15��Ŀ�����ݲ�������
		;

	udpTransceiver->SendDataNow(dataPacket.toUtf8().data(), ui->radarIpAddress->text(), ui->radarIpPort->text().toInt());
}

void MainWin::BtnRadarSpinChange(int value) {
	radarUdpTimer.stop();
	radarUdpTimer.start(ui->radarSpinBox->text().toInt() * 1000);
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

	s << (int)(ui->direction->text().toInt());		//drection to groud

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

	lon = ui->Lon->text().toDouble();
	lat = ui->Lat->text().toDouble();

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

