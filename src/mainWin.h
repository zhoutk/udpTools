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
	int id;					//01 - Ŀ������	1000+
	int threatLevel;		//02 - ��в�ȼ�	3
	int IFF;				//03 - ��������	3
	QString state;			//04 - ���Ҵ���
	int aimType;			//05 - Ŀ������	15
	int hasWarning;			//06 - �澯��־	1
							   
	double direction;		//07 - Ŀ�귽λ
	double dist;			//08 - Ŀ�����
	double aimDirect;		//09 - Ŀ����Ժ���
	double aimSpeed;		//10 - Ŀ������ٶ�
	double upAngle;			//11 - ����
	double height;			//12 - �߶�
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
