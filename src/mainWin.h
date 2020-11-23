#ifndef MAINWIN_H
#define MAINWIN_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWin; }
QT_END_NAMESPACE

class MainWin : public QWidget
{
    Q_OBJECT

public:
    MainWin(QWidget *parent = nullptr);
    ~MainWin();

private:
    Ui::MainWin*ui;
};
#endif // MAINWIN_H
