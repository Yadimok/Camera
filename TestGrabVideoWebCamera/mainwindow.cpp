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

    connect(kGrabVideo, SIGNAL(destroyed()), thread, SLOT(quit()));
    connect(kGrabVideo, SIGNAL(destroyed()), kGrabVideo, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();

    connect(kGrabVideo, SIGNAL(SendInfo(QString)), this, SLOT(GetInfo(QString)));
    connect(kGrabVideo, SIGNAL(SendFrame(AVFrame *)), this, SLOT(GetFrame(AVFrame *)));

    labVideo = new QLabel();
    pbStop = new QPushButton(tr("Stop"));
    pbStop->setEnabled(false);
    pbStart = new QPushButton(tr("Start"));
    pbStart->setEnabled(false);
    pbGrabPicture = new QPushButton(tr("Grab picture"));
    pbGrabPicture->setEnabled(false);

    QGroupBox *videoLabelGroupBox = new QGroupBox();
    QHBoxLayout *hVideoBoxLayout = new QHBoxLayout();
    hVideoBoxLayout->addWidget(labVideo);
    videoLabelGroupBox->setLayout(hVideoBoxLayout);

    QComboBox *comboBox = new QComboBox();
    comboBox->setToolTip(tr("Resolution of web camera"));
    comboBox->addItems(QStringList() << "-1"
                       << "320x240"
                       << "640x480"
                       << "800x600"
                       << "1280x720");
    comboBox->setCurrentIndex(0);

    QGroupBox *groupBox = new QGroupBox();
    QHBoxLayout *hBoxLayout = new QHBoxLayout();
    hBoxLayout->addWidget(pbStart);
    hBoxLayout->addStretch(5);
    hBoxLayout->addWidget(pbStop);
    hBoxLayout->addStretch(5);
    hBoxLayout->addWidget(comboBox);
    hBoxLayout->addStretch(5);
    hBoxLayout->addWidget(pbGrabPicture);
    groupBox->setLayout(hBoxLayout);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(videoLabelGroupBox);
    layout->stretch(2);
    layout->addWidget(groupBox);

    ui->centralwidget->setLayout(layout);

    connect(pbStart, SIGNAL(clicked()), SLOT(on_pbStart()));
    connect(pbStop, SIGNAL(clicked()), SLOT(on_pbStop()));
    connect(pbGrabPicture, SIGNAL(clicked()), SLOT(on_pbGrabPicture()));
    connect(comboBox, SIGNAL(currentIndexChanged(int)), SLOT(on_comboBoxCurrentIndex(int)));

    mTimer = new QTimer();
    connect(mTimer, SIGNAL(timeout()), SLOT(updateDateTime()));
    mTimer->start(1000);
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
        on_pbStop();
        event->accept();
    }
}

void MainWindow::GetInfo(QString message)
{
    QMessageBox msgBox;
    msgBox.setText("Error");
    msgBox.setText(message);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
}

void MainWindow::on_comboBoxCurrentIndex(int index)
{
    switch (index) {
    case 0:
        resolution = SIZE_NONE;
        break;
    case 1:
        resolution = SIZE_320_240;
        break;
    case 2:
        resolution = SIZE_640_480;
        break;
    case 3:
        resolution = SIZE_800_600;
        break;
    case 4:
        resolution = SIZE_1280_720;
        break;
    default:
        resolution = SIZE_640_480;
        break;
    }

    pbStart->setEnabled(true);
}

void MainWindow::on_pbStart()
{
    pbStart->setEnabled(false);
    pbStop->setEnabled(true);
    pbGrabPicture->setEnabled(true);

    kGrabVideo->SetRunning(true);
    kGrabVideo->InitVideo(resolution);
    kGrabVideo->GetVideo();
}

void MainWindow::on_pbStop()
{
    pbStart->setEnabled(true);
    pbStop->setEnabled(false);

    kGrabVideo->SetRunning(false);    
}

void MainWindow::on_pbGrabPicture()
{
    if (labVideo->pixmap() != nullptr)
    {
        QString fileName = QDateTime::currentDateTime().toString("dd_MM_yyyy_hh_mm_ss_zzz");
        QString suffix = ".png";
        fileName += suffix;

        QByteArray array;
        QBuffer buffer(&array);
        buffer.open(QIODevice::WriteOnly);

        labVideo->pixmap()->save(fileName, "PNG", QUALITY);

        if (buffer.isOpen())
            buffer.close();
    }
}

void MainWindow::GetFrame(AVFrame *frame)
{
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    QImage img(frame->data[0], frame->width, frame->height, QImage::Format_RGB888);
    labVideo->setPixmap(QPixmap::fromImage(img));
    qApp->processEvents();
}

void MainWindow::updateDateTime()
{
    QString dateTime = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
    statusBar()->showMessage(dateTime);
}
