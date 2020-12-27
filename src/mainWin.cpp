#include <QtMath>
#include "mainWin.h"

static void ParseDatagrams(QByteArray& d);

double g_gpsPoxx = 0;
double g_gpsPoxy = 0;
double g_radarDist = 0;
double g_radarBear = 0;
double g_timeElapsed = 0;

static void calc_deg_per_m(double latMid, double lonMid, double &deg_per_m_lat, double &deg_per_m_lon)
{
    deg_per_m_lat = 1.0 / (111132.954 - 559.822 * cos( 2 * latMid ) + 1.175 * cos( 4 * latMid));
    deg_per_m_lon = 1.0 / (111132.954 * cos ( latMid ));
}

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
    ui->aisArriveTime->setDateTime(QDateTime::currentDateTime());
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
    connect(&radarUdpTimer, SIGNAL(timeout()), this, SLOT(CalcRadarTargetInfo()));
    connect(ui->btnRadarNew, SIGNAL(clicked()), this, SLOT(BtnRadarCreateClicked()));

    aisLon = ui->aisLon->text().toDouble();
    aisLat = ui->aisLat->text().toDouble();
    connect(ui->btnAisStart, SIGNAL(clicked()), this, SLOT(BtnAisStartClicked()));
    connect(ui->btnAisStop, SIGNAL(clicked()), this, SLOT(BtnAisStopClicked()));
    connect(ui->aisSpinBox, SIGNAL(valueChanged(int)), this, SLOT(BtnAisSpinChange(int)));
	connect(&aisUdpTimer, SIGNAL(timeout()), this, SLOT(AisSendUdpDynamicPackageOnTime()));
	connect(&aisUdpTimer, SIGNAL(timeout()), this, SLOT(AisSendUdpStaticPackageOnTime()));
    connect(ui->btnAisNew, SIGNAL(clicked()), this, SLOT(BtnAisCreateClicked()));
}

void MainWin::BtnAisCreateClicked() {
    long long id = ui->aisId->text().toLongLong();
    QString str = QString("%1").arg(id + 1, 7, 10, QLatin1Char('0'));
    ui->aisId->setText(str);
}

void MainWin::BtnAisStartClicked() {
    ui->btnAisStart->setEnabled(false);
    ui->btnAisStop->setEnabled(true);

    aisLon = ui->aisLon->text().toDouble();
	aisLat = ui->aisLat->text().toDouble();

	aisUdpTimer.start(ui->aisSpinBox->text().toInt() * 1000);
}

void MainWin::BtnAisStopClicked() {
    ui->btnAisStop->setEnabled(false);
    ui->btnAisStart->setEnabled(true);

    aisUdpTimer.stop();
}

void MainWin::AisSendUdpStaticPackageOnTime() {
    long long id = ui->aisId->text().toLongLong();

	QByteArray data;
	QDataStream s(&data, QIODevice::WriteOnly);

	const unsigned char head[] = { 0x90, 0x51 };
	s.writeRawData((const char*)head, 2);
	const char len[] = { 0x00, 0x5E };
	s.writeRawData(len, 2);
	const unsigned char sender[] = { 0x00, 0x00, 0x90, 0x03 };
	s.writeRawData((const char*)sender, 4);
	const unsigned char recver[] = { 0xFF, 0xFF, 0xFC, 0x01 };
	s.writeRawData((const char*)recver, 4);

	// Timestamp
	QTime ltm = QTime::currentTime();
	int timeInMs = (ltm.hour() * 60 * 60 + ltm.minute() * 60 + ltm.second()) * 1000;
	s << timeInMs;

	s << (int)id;
	s << (int)(ui->aisIMO->text().toDouble());
	s << (short)(ui->aisShipRank->currentText().toDouble());
    s << (short)1;          // location sensor
	s << (short)(ui->aisTargetLength->text().toDouble());
	s << (short)(ui->aisTargetWidth->text().toDouble());
	s << (short)(ui->aisTargetDraft->text().toDouble() * 10);

	char shipType[2];
	memset(shipType, 0, 2);
	strncpy(shipType, ui->aisTargetType->currentText().toStdString().c_str(), 2);
	s.writeRawData(shipType, 2);

	// destination
	char destination[21];
	memset(destination, 0, 21);
	strncpy(destination, ui->aisTargetDestination->text().toStdString().c_str(), 21);
	s.writeRawData(destination, 21);

	QDate nDate = ui->aisArriveTime->date();
	QTime nTime = ui->aisArriveTime->time();
	int ETA = 0;
	ETA = nDate.month() << 16;
	ETA |= nDate.day() << 11;
	ETA |= nTime.hour() << 6;
	ETA |= nTime.minute();
	s << ETA;

	// Call sign
	char callsign[12];
	memset(callsign, 0, 12);
	strncpy(callsign, ui->aisCallSign->text().toStdString().c_str(), 12);
	s.writeRawData(callsign, 12);

	// ship name
	char shipname[21];
	memset(shipname, 0, 21);
	strncpy(shipname, ui->aisShipName->text().toStdString().c_str(), 21);
	s.writeRawData(shipname, 21);

	udpTransceiver->SendDataNow(data, ui->aisIpAddress->text(), ui->aisIpPort->text().toInt());

	aisLon += qSin(ui->aisDirection->text().toDouble() * M_PI / 180) * speeDeta * ui->aisSpeed->text().toDouble();
	aisLat += qCos(ui->aisDirection->text().toDouble() * M_PI / 180) * speeDeta * ui->aisSpeed->text().toDouble();
}

void MainWin::AisSendUdpDynamicPackageOnTime() {
	long long id = ui->aisId->text().toLongLong();

	QByteArray data;
	QDataStream s(&data, QIODevice::WriteOnly);

	const unsigned char head[] = { 0x90, 0x50 };
	s.writeRawData((const char*)head, 2);
	const char len[] = { 0x00, 0x2C };
	s.writeRawData(len, 2);
	const unsigned char sender[] = { 0x00, 0x00, 0x90, 0x03 };
	s.writeRawData((const char*)sender, 4);
	const unsigned char recver[] = { 0xFF, 0xFF, 0xFC, 0x01 };
	s.writeRawData((const char*)recver, 4);

	// Timestamp
	QTime ltm = QTime::currentTime();
	int timeInMs = (ltm.hour() * 60 * 60 + ltm.minute() * 60 + ltm.second()) * 1000 + ltm.msec();
	s << timeInMs;

    s << (int)id;

	// State 1
	s << (short)(0x0000);
	// State 2
	s << (short)(0x0000);

    s << (short)(ui->aisROT->text().toDouble() * 100);
    s << (short)(ui->aisSpeed->text().toDouble() * 10);
    s << (int)0;            //memo
	// longitude
	s << (int)(aisLon * 10000 * 60);
	// latitude
	s << (int)(aisLat * 10000 * 60);
	s << (short)(ui->aisDirection->text().toDouble() * 10);
	s << (short)(ui->aisTrueCourse->text().toDouble());

	udpTransceiver->SendDataNow(data, ui->aisIpAddress->text(), ui->aisIpPort->text().toInt());

    aisLon += qSin(ui->aisDirection->text().toDouble() * M_PI / 180) * speeDeta * ui->aisSpeed->text().toDouble();
    aisLat += qCos(ui->aisDirection->text().toDouble() * M_PI / 180) * speeDeta * ui->aisSpeed->text().toDouble();
}

void MainWin::BtnAisSpinChange(int value) {
    aisUdpTimer.stop();
    aisUdpTimer.start(ui->aisSpinBox->text().toInt() * 1000);
}

void MainWin::CalcRadarTargetInfo()
{
    double course = ui->direction->text().toDouble();
    double speed = ui->speedToGround->text().toDouble();
    double initDist = ui->radarSpeed->text().toDouble();
    double initBear = ui->RadarDirection->text().toDouble();
    double rdSpeed = ui->radarVelocity->text().toDouble();
    double rdCourse = ui->radarCourse->text().toDouble();

    // radar initial pos
    double rdInitPosx = initDist * qSin(qDegreesToRadians(initBear)) * 1852.0;
    double rdInitPosy = initDist * qCos(qDegreesToRadians(initBear)) * 1852.0;

    double rdPosx = rdInitPosx + rdSpeed * qSin(qDegreesToRadians(rdCourse)) * 1852.0 / 3600.0 * g_timeElapsed;
    double rdPosy = rdInitPosy + rdSpeed * qCos(qDegreesToRadians(rdCourse)) * 1852.0 / 3600.0 * g_timeElapsed;

    g_gpsPoxx = speed * qSin(qDegreesToRadians(course)) * 1852.0 / 3600.0 * g_timeElapsed;
    g_gpsPoxy = speed * qCos(qDegreesToRadians(course)) * 1852.0 / 3600.0 * g_timeElapsed;

    g_radarDist = qSqrt((rdPosx - g_gpsPoxx) * (rdPosx - g_gpsPoxx) +
                        (rdPosy - g_gpsPoxy) * (rdPosy - g_gpsPoxy));

    g_radarBear = 90.0 - qRadiansToDegrees(qAtan2(rdPosy - g_gpsPoxy, rdPosx - g_gpsPoxx));

    g_timeElapsed++;
}

void MainWin::BtnRadarCreateClicked() {
    int id = ui->radarId->text().toInt();
    QString str = QString("%1").arg(id + 1, 4, 10, QLatin1Char('0'));
    ui->radarId->setText(str);
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
    g_timeElapsed = 0;
}

void MainWin::RadarSendUdpPackageOnTime() {                 //radar encode
    int id = ui->radarId->text().toInt();

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
        .arg(ui->radarBatchNumber->text().toInt())                                //1 批号
        .arg(g_radarDist)											//2 距离
        .arg(g_radarBear) 										//3 方位
        .arg(ui->radarBearingIndication->currentText())										//4 方位指示
        .arg(ui->radarVelocity->text().toDouble())												//5 目标速度
        .arg(ui->radarCourse->text().toDouble())		//6 目标航向
        .arg(ui->radarCourseIndication->currentText())		//7 航向指示
        .arg(0)		//8 CPA
        .arg(0)	//9 TCPA
        .arg(ui->radarUnit->currentText())		//10 单位
        .arg(ui->radarName->text())			//11 名称
        .arg(ui->radarTrackState->currentText())		//12 航迹状态
        .arg("")			//13 未用
        .arg(timeStr)				//14 UTC时间
        .arg(ui->radarCaptureType->currentText())			//15 捕获类型
        ;

    QByteArray ba(dataPacket.toStdString().c_str(), dataPacket.size());
    udpTransceiver->SendDataNow(ba, ui->radarIpAddress->text(), ui->radarIpPort->text().toInt());
}

void MainWin::BtnRadarSpinChange(int value) {
    radarUdpTimer.stop();
    radarUdpTimer.start(ui->radarSpinBox->text().toInt() * 1000);
}

void MainWin::SendUdpPackageOnTime() {                  //gps encode
    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);

    const unsigned char head[] = { 0x90, 0x26 };
    s.writeRawData((const char*)head, 2);
    const char len[] = { 0x00, 0x44 };
    s.writeRawData(len, 2);
    const unsigned char sender[] = { 0x00, 0x00, 0x90, 0x01 };
    s.writeRawData((const char *)sender, 4);
    const unsigned char recver[] = { 0xFF, 0xFF, 0xFF, 0xFF };
    s.writeRawData((const char *)recver, 4);
    const char gpsState[] = { 0x00, 0x01, 0x00, 0x00 };             //add memo
    s.writeRawData(gpsState, 4);

    s << (int)(lon * 600000.0);
    s << (int)(lat * 600000.0);
    double deg_per_m_lon, deg_per_m_lat;
    calc_deg_per_m(lat, lon, deg_per_m_lat, deg_per_m_lon);
    lon += qSin(ui->direction->text().toDouble() * M_PI / 180) * deg_per_m_lon * ui->speedToGround->text().toDouble();
    lat += qCos(ui->direction->text().toDouble() * M_PI / 180) * deg_per_m_lat * ui->speedToGround->text().toDouble();

    s << (int)(ui->height->text().toDouble());

    QTime nTime = QTime::currentTime();

    s << (short)nTime.hour();
    s << (short)nTime.minute();
    s << (short)nTime.second();

    const char speedState[] = { 0x00, 0x01 };
    s.writeRawData(speedState, 2);

    s << (int)(ui->speedToWater->text().toDouble() * 10);

    s << (int)(ui->speedToGround->text().toDouble() * 10);			//speed to ground

    s << (int)(ui->direction->text().toDouble() *10);		//drection to groud

	const char weatherState[] = { 0x00, 0x1F };
	s.writeRawData(weatherState, 2);

	s << (short)(ui->trueWindSpeed->text().toDouble() * 10);
	s << (short)(ui->trueWindDirection->text().toDouble());

    s << (short)(ui->relativeWindSpeed->text().toDouble() * 10);
	s << (short)(ui->relativeWindDirection->text().toDouble());
	
    s << (short)(ui->temperature->text().toDouble() * 10);
	s << (short)(ui->humidity->text().toDouble() * 10);

	s << (short)(ui->pressure->text().toDouble() * 10);
	
	const char depthState[] = { 0x00, 0x01 };
	s.writeRawData(depthState, 2);

	s << (short)(ui->depth->text().toDouble() * 10);

    udpTransceiver->SendDataNow(data, ui->ipAddress->text(), ui->ipPort->text().toInt());
}

void MainWin::SendUdpInfoOneOnTime() {                      //gps 1 encode
    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);

    const unsigned char head[] = { 0x90, 0x25 };
    s.writeRawData((const char*)head, 2);
    const char len[] = { 0x00, 0x1C };
    s.writeRawData(len, 2);
    const unsigned char sender[] = { 0x00, 0x00, 0x90, 0x01 };
    s.writeRawData((const char*)sender, 4);
    const unsigned char recver[] = { 0xFF, 0xFF, 0xFF, 0xFF };
    s.writeRawData((const char*)recver, 4);

    s << (short)1;
    s << (short)0;
    s << (int)(ui->heading->text().toDouble() * 10);
    s << (int)(ui->pitchAngle->text().toDouble() * 10);
    s << (int)(ui->rollAngle->text().toDouble() * 10);

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

