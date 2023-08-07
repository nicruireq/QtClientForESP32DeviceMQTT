#ifndef GUIPANEL_H
#define GUIPANEL_H

#include <QWidget>
#include <QtSerialPort/qserialport.h>
#include "qmqtt.h"

#include "topic.h"

namespace Ui {
class GUIPanel;
}

//QT4:QT_USE_NAMESPACE_SERIALPORT

class GUIPanel : public QWidget
{
    Q_OBJECT
    
public:
    //GUIPanel(QWidget *parent = 0);
    explicit GUIPanel(QWidget *parent = 0);
    ~GUIPanel(); // Da problemas
    
private slots:

    void on_runButton_clicked();
    void on_pushButton_clicked();

    void onMQTT_Received(const QMQTT::Message &message);
    void onMQTT_Connected(void);

    void onMQTT_subscribed(const QString &topic);


    void on_pingButton_clicked();

    void on_gpioRedButton_toggled(bool checked);

    void on_gpioGreenButton_toggled(bool checked);

    void on_gpioBlueButton_toggled(bool checked);

    void on_sondeaButton_clicked();

private: // funciones privadas
//    void pingDevice();
    void startClient();
    void processError(const QString &s);
    void activateRunButton();
    void cambiaLEDs();
    // Functions to subscribe new messages on topics
    void SendMessageForGpioRGBLeds();
    void SendMessageCommand(const Commands &name);
    // Functions to process incoming data on yopic by topic
    void processFromTopicCommand(const QJsonObject &jsonData);
    void processFromTopicGPIOLed(const QJsonObject &jsonData);
    void processFromTopicButtons(const QJsonObject &jsonData);
private:
    Ui::GUIPanel *ui;
    int transactionCount;
    QMQTT::Client *_client;
    bool connected;
};

#endif // GUIPANEL_H
