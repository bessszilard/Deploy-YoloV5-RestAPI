#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class QTcpSocket;
class QTcpServer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void openImageDialog();
    void connectToSocket();
    void processImage();
    void acceptConnection();
    void updateServerProgress();

private:
    Ui::MainWindow *ui;
    double m_imageScaleRatio; // TODOsz initialize
    QPixmap m_loadedImage;
    QTcpSocket* m_socket;
    QTcpServer* m_tcpServer;
    QTcpSocket* m_tcpServerConnection;
    int bytesReceived;
};
#endif // MAINWINDOW_H
