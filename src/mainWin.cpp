#include "mainWin.h"

static void ParseDatagrams(QByteArray& d);

MainWin::MainWin(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWin)
	, speeDeta(0.0001), udpTimer(this), radarUdpTimer(this), aisUdpTimer(this)
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
	connect(&udpTimer, SIGNAL(timeout()), this, SLOT(SendUdpInfoOneOnTime()));

	connect(ui->btnRadarStart, SIGNAL(clicked()), this, SLOT(BtnRadarStartClicked()));
	connect(ui->btnRadarStop, SIGNAL(clicked()), this, SLOT(BtnRadarStopClicked()));
	connect(ui->radarSpinBox, SIGNAL(valueChanged(int)), this, SLOT(BtnRadarSpinChange(int)));
	connect(&radarUdpTimer, SIGNAL(timeout()), this, SLOT(RadarSendUdpPackageOnTime()));
	connect(ui->btnRadarNew, SIGNAL(clicked()), this, SLOT(BtnRadarCreateClicked()));

	aisLon = ui->aisLon->text().toDouble();
	aisLat = ui->aisLat->text().toDouble();
	connect(ui->btnAisStart, SIGNAL(clicked()), this, SLOT(BtnAisStartClicked()));
	connect(ui->btnAisStop, SIGNAL(clicked()), this, SLOT(BtnAisStopClicked()));
	connect(ui->aisSpinBox, SIGNAL(valueChanged(int)), this, SLOT(BtnAisSpinChange(int)));
	connect(&aisUdpTimer, SIGNAL(timeout()), this, SLOT(AisSendUdpPackageOnTime()));
	connect(ui->btnAisNew, SIGNAL(clicked()), this, SLOT(BtnAisCreateClicked()));
}

void MainWin::BtnAisCreateClicked() {
	QStringList idstr = ui->aisId->text().split("_");
	int id = idstr[1].toInt();
	QString str = QString("%1").arg(id + 1, 4, 10, QLatin1Char('0'));
	ui->aisId->setText(QString("AIS_").append(str));
}

void MainWin::BtnAisStartClicked() {
	ui->btnAisStart->setEnabled(false);
	ui->btnAisStop->setEnabled(true);

	aisUdpTimer.start(ui->aisSpinBox->text().toInt() * 1000);
}

void MainWin::BtnAisStopClicked() {
	ui->btnAisStop->setEnabled(false);
	ui->btnAisStart->setEnabled(true);

	aisUdpTimer.stop();
}


//s << (int)(lon * 600000.0);
//s << (int)(lat * 600000.0);



void MainWin::AisSendUdpPackageOnTime() {
	QStringList idstr = ui->aisId->text().split("_");
	int id = idstr[1].toInt();
	
	QString dataPacket = QString("UdPAIS,%1,%2,%3,%4,%5,*hh\r\n")
		.arg(ui->radarId->text().remove("_"))
		.arg(aisLon)
		.arg(aisLat)
		.arg(ui->aisSpeed->text().toInt() * 10)						
		.arg(ui->aisDirection->text().toInt() * 10) 
		;

	udpTransceiver->SendDataNow(dataPacket.toUtf8().data(), ui->aisIpAddress->text(), ui->aisIpPort->text().toInt());

	aisLon += qSin(ui->aisDirection->text().toDouble() * M_PI / 180) * speeDeta * ui->aisSpeed->text().toDouble();
	aisLat += qCos(ui->aisDirection->text().toDouble() * M_PI / 180) * speeDeta * ui->aisSpeed->text().toDouble();
}

void MainWin::BtnAisSpinChange(int value) {
	aisUdpTimer.stop();
	aisUdpTimer.start(ui->aisSpinBox->text().toInt() * 1000);
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

	QTime nTime = QTime::currentTime();
	QString timeStr = QString("%1%2%3.%4")
		.arg(QString("%1").arg(nTime.hour(), 2, 10, QLatin1Char('0')))
		.arg(QString("%1").arg(nTime.minute(), 2, 10, QLatin1Char('0')))
		.arg(QString("%1").arg(nTime.second(), 2, 10, QLatin1Char('0')))
		.arg(QString("%1").arg(nTime.msec(), 3, 10, QLatin1Char('0')))
		;

	QString dataPacket = QString("UdPbC0\s:RA%1,n:%2*HH\$GETTM,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15,%16,%17*hh\r\n")
		.arg(ui->radarId->text().remove("_"))
		.arg(id)
		.arg(id)
		.arg(ui->radarSpeed->text().toInt() * 1000)											//2，距离
		.arg(ui->RadarDirection->text()) 										//3，方位
		.arg("T")										//4，绝对，相对
		.arg(6)												//5，速度
		.arg(45)		//6，航向
		.arg("R")		//7，绝对，相对航向指示
		.arg("0.01")		//8，CPA
		.arg("0.02")	//9，TCPA
		.arg("N")		//10，目标速度距离单位
		.arg("")			//11，目标名称
		.arg("L")		//12，航迹状态
		.arg("")			//13，未用
		.arg(timeStr)				//14，时间
		.arg("A")			//15，目标数据捕获类型
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

	const char speedToWater[] = { 0x00, 0x00, 0x00, 0x12 };
	s.writeRawData(speedToWater, 4);

	s << (int)(ui->speed->text().toInt() * 10);			//speed to ground

	s << (int)(ui->direction->text().toInt() *10);		//drection to groud

	const char lastGroup[] = { 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x00, 0x20, 0x00, 0x16, 0x01, 0x00, 0x00, 0x91, 0x00, 0x67, 0x00, 0x43, 0x00, 0x51 };
	s.writeRawData(lastGroup, 20);

    udpTransceiver->SendDataNow(data, ui->ipAddress->text(), ui->ipPort->text().toInt());
}

void MainWin::SendUdpInfoOneOnTime() {
	QByteArray data;
	QDataStream s(&data, QIODevice::WriteOnly);

	const char head[] = { 0x90, 0x25 };
	s.writeRawData(head, 2);
	const char len[] = { 0x00, 0x1C };
	s.writeRawData(len, 2);
	const char sender[] = { 0x00, 0x00, 0x90, 0x01 };
	s.writeRawData(sender, 4);
	const char recver[] = { 0xFF, 0xFF, 0xFF, 0xFF };
	s.writeRawData(recver, 4);

	s << (short)1;
	s << (short)0;
	s << (int)(ui->heading->text().toInt()) * 10;
	s << (int)(ui->pitchAngle->text().toInt()) * 10;
	s << (int)(ui->rollAngle->text().toInt()) * 10;

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

