#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>

#include "uvcframecapture.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

const float SCALE_PLUS = 1.25f;
const float SCALE_MINUS = 0.8f;
const int QUALITY = 100;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();



private:
    Ui::MainWindow *ui;

    QPointer<QDockWidget> dockWidget;
    QPointer<QGraphicsView> graphicsView;
    QPointer<QGraphicsScene> graphicsScene;
    QGraphicsPixmapItem *graphicsPixmapItem;

    QPointer<QPushButton> pbScaledPlus;
    QPointer<QPushButton> pbScaledMinus;
    QPointer<QPushButton> pbStartStream;
    QPointer<QPushButton> pbStopStream;
    QPointer<QPushButton> pbTakePicture;

    QPointer<QLabel> labelCaptured;

    UVCFrameCapture *kUVCFrameCapture;
    QPointer<QThread> kThread;
    QPointer<QTimer> kTimer;

protected:
    void closeEvent(QCloseEvent *event) override;


private slots:
    void slotScaledPlus();
    void slotScaledMinus();
    void slotTakePicture();

    void slotLoadImage(QImage image);
    void slotStartUVC();
    void slotStopUVC();

    void slotUpdateDateTime();

};
#endif // MAINWINDOW_H
