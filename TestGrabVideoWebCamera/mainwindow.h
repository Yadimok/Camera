/*
*
* Copyright (C) Yadimok2020
*
* TestGrabVideoWebCamera is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* TestGrabVideoWebCamera is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public License
* along with TestGrabVideoWebCamera. If not, write to
* the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
**/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include <QThread>

#include "grabvideo.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    enum {
        QUALITY = 100
    };

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    GrabVideo *kGrabVideo;

    QPointer<QLabel> labelVideo;
    QPointer<QPushButton> pbStart;
    QPointer<QPushButton> pbStop;
    QPointer<QPushButton> pbGrabPicture;
    QTimer *mTimer;
    bool bClosed;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void slotGetInfo(QString message);
    void slotStart();
    void slotStop();
    void slotGrabPicture();
    void slotGetImage(QImage image);
    void slotUpdateDateTime();
};
#endif // MAINWINDOW_H
