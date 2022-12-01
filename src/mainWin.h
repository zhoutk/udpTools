#ifndef MAINWIN_H
#define MAINWIN_H

#define _USE_MATH_DEFINES

#include "./ui_mainWin.h"
#include <QWidget>
#include <QTimer>
#include <QTime>
#include <QtMath>
#include <QVector>
#include "math.h"
#include "QByteArray"
#include "network/udptransceiver.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWin; }
QT_END_NAMESPACE

class NaviRadar {
public:
	int id;					//01 - 目标批号	1000+
	int threatLevel;		//02 - 威胁等级	3
	int IFF;				//03 - 敌我属性	3
	QString state;			//04 - 国家代号
	int aimType;			//05 - 目标类型	15
	int hasWarning;			//06 - 告警标志	1
							   
	double direction;		//07 - 目标方位
	double dist;			//08 - 目标距离
	double aimDirect;		//09 - 目标绝对航向
	double aimSpeed;		//10 - 目标绝对速度
	double upAngle;			//11 - 仰角
	double height;			//12 - 高度
};

class MainWin : public QWidget
{
    Q_OBJECT

public:
    MainWin(QWidget *parent = nullptr);
    ~MainWin();

public slots:
	void BtnNRadarStartClicked();
	void BtnNRadarStopClicked();
	void NRadarSendOnTime();

    void BtnStartClicked();
    void BtnStopClicked();
	void SendUdpPackageOnTime();
	void SendUdpInfoOneOnTime();
    void BtnSpinChange(int);

	void BtnRadarStartClicked();
	void BtnRadarStopClicked();
	void BtnRadarCreateClicked();
	void RadarSendUdpPackageOnTime();
	void BtnRadarSpinChange(int);

	void BtnAisStartClicked();
	void BtnAisStopClicked();
	void BtnAisCreateClicked();
	void AisSendUdpDynamicPackageOnTime();
	void AisSendUdpStaticPackageOnTime();
	void BtnAisSpinChange(int);

    void CalcRadarTargetInfo();

private:
    Ui::MainWin*ui;
	QTimer udpTimer;
    QTimer udpTimer2;
    UDPTransceiver* udpTransceiver;
    double lon;
    double lat;
    double speeDeta;

	QTimer radarUdpTimer;

	QTimer aisUdpTimer;
	double aisLon;
	double aisLat;

	QTimer nradarTimer;

};
#endif // MAINWIN_H
