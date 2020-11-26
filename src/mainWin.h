#ifndef MAINWIN_H
#define MAINWIN_H

#define _USE_MATH_DEFINES

#include "./ui_mainWin.h"
#include <QWidget>
#include <QTimer>
#include <QTime>
#include <QtMath>
#include "math.h"
#include "QByteArray"
#include "network/udptransceiver.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWin; }
QT_END_NAMESPACE

class MainWin : public QWidget
{
    Q_OBJECT

public:
    MainWin(QWidget *parent = nullptr);
    ~MainWin();

public slots:
    void BtnStartClicked();
    void BtnStopClicked();
    void SendUdpPackageOnTime();
    void BtnSpinChange(int);

	void BtnRadarStartClicked();
	void BtnRadarStopClicked();
	void BtnRadarCreateClicked();
	void RadarSendUdpPackageOnTime();
	void BtnRadarSpinChange(int);

	void BtnAisStartClicked();
	void BtnAisStopClicked();
	void BtnAisCreateClicked();
	void AisSendUdpPackageOnTime();
	void BtnAisSpinChange(int);

private:
    Ui::MainWin*ui;
    QTimer udpTimer;
    UDPTransceiver* udpTransceiver;
    double lon;
    double lat;
    double speeDeta;

	QTimer radarUdpTimer;

	QTimer aisUdpTimer;
	double aisLon;
	double aisLat;
};
#endif // MAINWIN_H
