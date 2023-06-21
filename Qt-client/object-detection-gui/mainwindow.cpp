#include <QFileDialog>
#include <QPixmap>
#include <QDebug>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>
#include <QHostAddress>
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QHttpMultiPart>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QBuffer>
#include <QPainter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_socket{ new QTcpSocket(this) }
    , m_tcpServer { new QTcpServer(this) }
{
    ui->setupUi(this);

    connect(ui->m_openImageBt, &QPushButton::clicked, this, &MainWindow::openImageDialog);
    connect(ui->m_processImageBt, &QPushButton::clicked, this, &MainWindow::processImage);
    connect(ui->m_connectBt, &QPushButton::clicked, this, &MainWindow::connectToSocket);
    connect(m_tcpServer, &QTcpServer::newConnection, this, &MainWindow::acceptConnection);

    if (!m_tcpServer->listen(QHostAddress::Any, 5000))
    {
        qDebug() << "Server could not start!";
    }
    else
    {
        qDebug() << "Server started on port" << m_tcpServer->serverPort();
    }
    ui->m_processImageBt->setEnabled(false);
 }

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::openImageDialog()
{
   auto fileName = QFileDialog::getOpenFileName(this,
                                            tr("Open Image"), "/home/jana", tr("Image Files (*.png *.jpg *.bmp)"));

   QPixmap image(fileName);

   // TODOsz check is the file empty
   m_loadedImage = image.copy();

   double imageWidth = static_cast<double>(image.width());
   double imageHeight = static_cast<double>(image.height());
   double aspectRatio = imageWidth / imageHeight;

   m_imageScaleRatio = aspectRatio < 1 ? imageHeight / ui->m_imageLabel->height() : imageWidth / ui->m_imageLabel->width();
   qDebug() << m_imageScaleRatio;
   ui->m_imageLabel->setPixmap(image.scaled(image.width()/m_imageScaleRatio, image.height()/m_imageScaleRatio, Qt::KeepAspectRatio));
}

void MainWindow::connectToSocket()
{
   int port = ui->m_hostAddress->text().toInt();
   m_socket->connectToHost(QHostAddress::LocalHost, port);
   m_socket->open(QIODevice::ReadWrite);

   if(m_socket->isOpen())
   {
        ui->m_processImageBt->setEnabled(true);
        ui->m_hostAddress->setStyleSheet("background-color: green");
   }
}

void MainWindow::processImage()
{
   QByteArray ba;              // Construct a QByteArray object
   QBuffer buffer(&ba);        // Construct a QBuffer object using the QbyteArray
   m_loadedImage.save(&buffer, "PNG"); // Save the QImage data into the QBuffer
   if (m_socket->isOpen())
   {
       QString stringToSend = QString("file_name.png;%1").arg(buffer.size());
       m_socket->write(stringToSend.toStdString().c_str());
       m_socket->waitForBytesWritten();
       m_socket->flush();
       m_socket->write(ba);
       m_socket->write("<END>");
       m_socket->waitForBytesWritten();
       m_socket->flush();
   }
}

void MainWindow::acceptConnection()
{
   m_tcpServerConnection = m_tcpServer->nextPendingConnection();
   connect(m_tcpServerConnection, &QTcpSocket::readyRead, this, &MainWindow::updateServerProgress);
//   connect(m_tcpServerConnection, &QTcpSocket::SocketError, &MainWindow::displayError);
//   m_tcpServer->close();
}

void MainWindow::updateServerProgress()
{
    bytesReceived += (int)m_tcpServerConnection->bytesAvailable();
    QByteArray package = m_tcpServerConnection->readAll();
    QString dataAsString = QString(package);

    QStringList csvLines = dataAsString.split("\n");

    QPixmap processedImage = m_loadedImage.copy();
    QPainter qPainter(&processedImage);
    qPainter.setBrush(Qt::NoBrush);
    QPen pen(Qt::red);
    pen.setWidth(3);
    qPainter.setPen(pen);

    for (const QString& csvLine : csvLines)
    {
       if (csvLine.isEmpty())
       {
           continue;
       }
       QStringList values = csvLine.split(";");
       if (values.size() != 6)
       {
           qDebug() << "Invalid csv line:" << csvLine;
           continue;
       }
       bool ok;
       values[0].toInt(&ok);
       if (!ok)
       {
           continue;
       }

       QPoint topLeft (values[0].toInt(), values[1].toInt());
       QPoint bottomRight (values[2].toInt(), values[3].toInt());
       QRect boundingBox(topLeft, bottomRight);
//       qDebug() << topLeft << bottomRight;
       qPainter.drawRect(boundingBox);
    }
    ui->m_imageLabel->setPixmap(processedImage.scaled(processedImage.width()/m_imageScaleRatio, processedImage.height()/m_imageScaleRatio, Qt::KeepAspectRatio));
    ui->m_imageLabel->update();
}
