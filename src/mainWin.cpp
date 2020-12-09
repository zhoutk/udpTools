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

    aisLon = ui->aisLon->text().toDouble();
	aisLat = ui->aisLat->text().toDouble();

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

	QTime nTime = QTime::currentTime();
	QString timeStr = QString("%1%2%3.%4")
		.arg(QString("%1").arg(nTime.hour(), 2, 10, QLatin1Char('0')))
		.arg(QString("%1").arg(nTime.minute(), 2, 10, QLatin1Char('0')))
		.arg(QString("%1").arg(nTime.second(), 2, 10, QLatin1Char('0')))
		.arg(QString("%1").arg(nTime.msec(), 3, 10, QLatin1Char('0')))
		;
	
	QString dataPacket = QString("UdPAIS,%1,%2,%3,%4,%5,%6,*hh\r\n")
		.arg(ui->aisId->text().remove("_"))
		.arg(aisLon)
		.arg(aisLat)
		.arg(ui->aisSpeed->text().toInt() * 10)						
		.arg(ui->aisDirection->text().toInt() * 10) 
		.arg(timeStr)
		;

    QByteArray ba(dataPacket.toStdString().c_str(), dataPacket.size());
    udpTransceiver->SendDataNow(ba, ui->aisIpAddress->text(), ui->aisIpPort->text().toInt());

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
    g_timeElapsed = 0;
}

void MainWin::RadarSendUdpPackageOnTime() {                 //radar encode
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
        .arg(g_radarDist * 1000)											//2������
        .arg(g_radarBear) 										//3����λ
        .arg("T")										//4�����ԣ�����
        .arg(ui->radarVelocity->text().toDouble())												//5���ٶ�
        .arg(ui->radarCourse->text().toDouble())		//6������
        .arg("R")		//7�����ԣ����Ժ���ָʾ
        .arg("0.01")		//8��CPA
        .arg("0.02")	//9��TCPA
        .arg("N")		//10��Ŀ���ٶȾ��뵥λ
        .arg("")			//11��Ŀ������
        .arg("L")		//12������״̬
        .arg("")			//13��δ��
        .arg(timeStr)				//14��ʱ��
        .arg("A")			//15��Ŀ�����ݲ�������
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

    s << (int)(ui->height->text().toInt());

    QTime nTime = QTime::currentTime();

    s << (short)nTime.hour();
    s << (short)nTime.minute();
    s << (short)nTime.second();

    const char speedState[] = { 0x00, 0x01 };
    s.writeRawData(speedState, 2);

    s << (int)(ui->speedToWater->text().toInt() * 10);

    s << (int)(ui->speedToGround->text().toInt() * 10);			//speed to ground

    s << (int)(ui->direction->text().toInt() *10);		//drection to groud

	const char weatherState[] = { 0x00, 0x1F };
	s.writeRawData(weatherState, 2);

	s << (short)(ui->trueWindSpeed->text().toInt() * 10);
	s << (short)(ui->trueWindDirection->text().toInt());

    s << (short)(ui->relativeWindSpeed->text().toInt() * 10);
	s << (short)(ui->relativeWindDirection->text().toInt());
	
    s << (short)(ui->temperature->text().toInt() * 10);
	s << (short)(ui->humidity->text().toInt() * 10);

	s << (short)(ui->pressure->text().toInt() * 10);
	
	const char depthState[] = { 0x00, 0x01 };
	s.writeRawData(depthState, 2);

	s << (short)(ui->depth->text().toInt() * 10);

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

