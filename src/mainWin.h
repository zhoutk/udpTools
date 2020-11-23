#ifndef MAINWIN_H
#define MAINWIN_H

#include "./ui_mainWin.h"
#include <QWidget>
#include <QTimer>

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

private:
    Ui::MainWin*ui;
    QTimer udpTimer;
};
#endif // MAINWIN_H
