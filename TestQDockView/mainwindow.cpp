#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(tr("TestQDockView"));

    //Dock widget
    QPointer<QWidget> pWidget = new QWidget;
    dockWidget = new QDockWidget(tr("Dock widget \"Web-camera\""));
    dockWidget->setFeatures(dockWidget->features() ^ QDockWidget::DockWidgetClosable);
    dockWidget->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
    QPointer<QHBoxLayout> hboxLayout = new QHBoxLayout;
    QPointer<QVBoxLayout> vboxLayout = new QVBoxLayout;
    QPointer<QGroupBox> groupBox = new QGroupBox;
    pbScaledPlus = new QPushButton;
    pbScaledPlus->setIcon(QIcon(":/zoom-in.png"));
    pbScaledPlus->setToolTip(tr("Zoom in"));
    pbScaledMinus = new QPushButton;
    pbScaledMinus->setIcon(QIcon(":/zoom-out.png"));
    pbScaledMinus->setToolTip(tr("Zoom out"));
    pbStartStream = new QPushButton;
    pbStartStream->setIcon(QIcon(":/play_500x500.png"));
    pbStartStream->setToolTip(tr("Start stream"));
    pbStopStream = new QPushButton;
    pbStopStream->setIcon(QIcon(":/stop_500x500.png"));
    pbStopStream->setToolTip(tr("Stop stream"));
    pbTakePicture = new QPushButton;
    pbTakePicture->setIcon(QIcon(":/camera-photo.png"));
    pbTakePicture->setToolTip(tr("Take picture"));
    graphicsScene = new QGraphicsScene;
    graphicsView = new QGraphicsView(graphicsScene);
    graphicsView->setRenderHints(QPainter::Antialiasing);
    graphicsPixmapItem = new QGraphicsPixmapItem;
    graphicsScene->addItem(graphicsPixmapItem);

    hboxLayout->addWidget(pbStartStream);
    hboxLayout->addStretch(5);
    hboxLayout->addWidget(pbStopStream);
    hboxLayout->addStretch(5);
    hboxLayout->addWidget(pbScaledPlus);
    hboxLayout->addStretch(5);
    hboxLayout->addWidget(pbScaledMinus);
    hboxLayout->addStretch(5);
    hboxLayout->addWidget(pbTakePicture);
    groupBox->setLayout(hboxLayout);

    vboxLayout->addWidget(graphicsView);
    vboxLayout->addWidget(groupBox);

    pWidget->setLayout(vboxLayout);
    dockWidget->setWidget(pWidget);

    kUVCFrameCapture = new UVCFrameCapture();
    kThread = new QThread;
    kUVCFrameCapture->moveToThread(kThread);

    connect(kThread, SIGNAL(finished()), kUVCFrameCapture, SLOT(deleteLater()));
    connect(this, SIGNAL(destroyed()), kThread, SLOT(quit()));
    connect(kUVCFrameCapture, SIGNAL(destroyed()), kThread, SLOT(quit()));
    connect(kUVCFrameCapture, SIGNAL(destroyed()), kUVCFrameCapture, SLOT(deleteLater()));
    kThread->start(QThread::NormalPriority);

    connect(pbStartStream, SIGNAL(clicked()), this, SLOT(slotStartUVC()));
    connect(pbStopStream, SIGNAL(clicked()), this, SLOT(slotStopUVC()));
    connect(pbScaledPlus, SIGNAL(clicked()), this, SLOT(slotScaledPlus()));
    connect(pbScaledMinus, SIGNAL(clicked()), this, SLOT(slotScaledMinus()));
    connect(pbTakePicture, SIGNAL(clicked()), this, SLOT(slotTakePicture()));
    connect(kUVCFrameCapture, SIGNAL(signalSendFrame(QImage)), this, SLOT(slotLoadImage(QImage)));

    kTimer = new QTimer;
    connect(kTimer, SIGNAL(timeout()), SLOT(slotUpdateDateTime()));
    kTimer->start(1000);
}

MainWindow::~MainWindow()
{
    delete ui;

    delete kUVCFrameCapture;

    if (kThread != nullptr)
    {
        kThread->quit();
        kThread->wait();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton ret;
    ret = QMessageBox::question(this, QApplication::applicationName(), tr("Do you want to close app?"),
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ret == QMessageBox::No)
    {
        event->ignore();
    } else
    {
        slotStopUVC();
        event->accept();
    }
}

void MainWindow::slotUpdateDateTime()
{
    QString dateTime = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
    statusBar()->showMessage(dateTime);
}

void MainWindow::slotScaledPlus()
{
    graphicsView->scale(SCALE_PLUS, SCALE_PLUS);
}

void MainWindow::slotScaledMinus()
{
    graphicsView->scale(SCALE_MINUS, SCALE_MINUS);
}

void MainWindow::slotTakePicture()
{
    if (!graphicsPixmapItem->pixmap().isNull())
    {
        QString fileName = QDateTime::currentDateTime().toString("dd_MM_yyyy_hh_mm_ss_zzz");
        QString suffix = ".png";
        fileName += suffix;

        QByteArray array;
        QBuffer buffer(&array);
        buffer.open(QIODevice::WriteOnly);

        graphicsPixmapItem->pixmap().save(fileName, "PNG", QUALITY);

        if (buffer.isOpen())
            buffer.close();
    }
}

void MainWindow::slotLoadImage(QImage image)
{
    qApp->processEvents();
    graphicsPixmapItem->setPixmap(QPixmap::fromImage(image));
    qApp->processEvents();
}

void MainWindow::slotStartUVC()
{
    kUVCFrameCapture->OpenDevice();
    kUVCFrameCapture->StartStream();
}

void MainWindow::slotStopUVC()
{
    kUVCFrameCapture->StopStream();
}

