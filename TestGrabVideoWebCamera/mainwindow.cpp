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

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("TestGrabVideoWebCamera");

    QThread *thread = new QThread;
    kGrabVideo = new GrabVideo();
    kGrabVideo->moveToThread(thread);

    connect(thread, SIGNAL(finished()), kGrabVideo, SLOT(deleteLater()));
    connect(this, SIGNAL(destroyed()), thread, SLOT(quit()));
    connect(kGrabVideo, SIGNAL(destroyed()), thread, SLOT(quit()));
    connect(kGrabVideo, SIGNAL(destroyed()), kGrabVideo, SLOT(deleteLater()));
    thread->start();

    connect(kGrabVideo, SIGNAL(signalSendInfo(QString)), this, SLOT(slotGetInfo(QString)));
    connect(kGrabVideo, SIGNAL(signalSendImage(QImage)), this, SLOT(slotGetImage(QImage)));

    labelVideo = new QLabel();
    pbStop = new QPushButton(tr("Stop"));
    pbStop->setEnabled(false);
    pbStart = new QPushButton(tr("Start"));
    pbStart->setEnabled(true);
    pbGrabPicture = new QPushButton(tr("Grab picture"));
    pbGrabPicture->setEnabled(false);

    QPointer<QGroupBox> videoLabelGroupBox = new QGroupBox();
    QPointer<QHBoxLayout> hVideoBoxLayout = new QHBoxLayout();
    hVideoBoxLayout->addWidget(labelVideo);
    videoLabelGroupBox->setLayout(hVideoBoxLayout);

    QPointer<QGroupBox> groupBox = new QGroupBox();
    QPointer<QHBoxLayout> hBoxLayout = new QHBoxLayout();
    hBoxLayout->addWidget(pbStart);
    hBoxLayout->addStretch(5);
    hBoxLayout->addWidget(pbStop);
    hBoxLayout->addStretch(5);
    hBoxLayout->addWidget(pbGrabPicture);
    groupBox->setLayout(hBoxLayout);

    QPointer<QVBoxLayout> layout = new QVBoxLayout();
    layout->addWidget(videoLabelGroupBox);
    layout->stretch(2);
    layout->addWidget(groupBox);

    ui->centralwidget->setLayout(layout);

    connect(pbStart, SIGNAL(clicked()), SLOT(slotStart()));
    connect(pbStop, SIGNAL(clicked()), SLOT(slotStop()));
    connect(pbGrabPicture, SIGNAL(clicked()), SLOT(slotGrabPicture()));

    mTimer = new QTimer();
    connect(mTimer, SIGNAL(timeout()), SLOT(slotUpdateDateTime()));
    mTimer->start(1000);

    bClosed = false;
}

MainWindow::~MainWindow()
{
    delete ui;

    delete kGrabVideo;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton ret;
    ret = QMessageBox::question(this, QApplication::applicationName(), tr("Do you want to close app?"),
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ret == QMessageBox::No) {
        event->ignore();
    } else {
        if (!bClosed)
            slotStop();
        event->accept();
    }
}

void MainWindow::slotGetInfo(QString message)
{
    QMessageBox msgBox;
    msgBox.setText("Error");
    msgBox.setText(message);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
}

void MainWindow::slotStart()
{
    pbStart->setEnabled(false);
    pbStop->setEnabled(true);
    pbGrabPicture->setEnabled(true);

    kGrabVideo->InitVideo();
    kGrabVideo->OpenCamera();

    bClosed = true;
}

void MainWindow::slotStop()
{
    kGrabVideo->CloseCamera();

    pbStart->setEnabled(true);
    pbStop->setEnabled(false);

    bClosed = false;
}

void MainWindow::slotGrabPicture()
{
    if (labelVideo->pixmap() != nullptr)
    {
        QString fileName = QDateTime::currentDateTime().toString("dd_MM_yyyy_hh_mm_ss_zzz");
        QString suffix = ".png";
        fileName += suffix;

        QByteArray array;
        QBuffer buffer(&array);
        buffer.open(QIODevice::WriteOnly);

        labelVideo->pixmap()->save(fileName, "PNG", QUALITY);

        if (buffer.isOpen())
            buffer.close();
    }
}

void MainWindow::slotGetImage(QImage image)
{
    qApp->processEvents();
    labelVideo->setPixmap(QPixmap::fromImage(image));
    qApp->processEvents();
}

void MainWindow::slotUpdateDateTime()
{
    QString dateTime = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
    statusBar()->showMessage(dateTime);
}
