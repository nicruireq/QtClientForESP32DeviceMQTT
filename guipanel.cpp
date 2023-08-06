#include "guipanel.h"
#include "ui_guipanel.h"
#include <QSerialPort>      // Comunicacion por el puerto serie
#include <QSerialPortInfo>  // Comunicacion por el puerto serie
#include <QMessageBox>      // Se deben incluir cabeceras a los componentes que se vayan a crear en la clase

#include <QJsonObject>
#include <QJsonDocument>

// y que no estén incluidos en el interfaz gráfico. En este caso, la ventana de PopUp <QMessageBox>
// que se muestra al recibir un PING de respuesta

#include<stdint.h>      // Cabecera para usar tipos de enteros con tamaño
#include<stdbool.h>     // Cabecera para usar booleanos

#include "topic.h"

GUIPanel::GUIPanel(QWidget *parent) :  // Constructor de la clase
    QWidget(parent),
    ui(new Ui::GUIPanel)               // Indica que guipanel.ui es el interfaz grafico de la clase
  , transactionCount(0)
{
    ui->setupUi(this);                // Conecta la clase con su interfaz gráfico.
    setWindowTitle(tr("Interfaz de Control")); // Título de la ventana

    _client=new QMQTT::Client(QHostAddress::LocalHost, 1883); //localhost y lo otro son valores por defecto


    connect(_client, SIGNAL(connected()), this, SLOT(onMQTT_Connected()));    
    connect(_client, SIGNAL(received(const QMQTT::Message &)), this, SLOT(onMQTT_Received(const QMQTT::Message &)));
    connect(_client, SIGNAL(subscribed(const QString &)), this, SLOT(onMQTT_subscribed(const QString &)));    

    connected=false;                 // Todavía no hemos establecido la conexión USB
}

GUIPanel::~GUIPanel() // Destructor de la clase
{
    delete ui;   // Borra el interfaz gráfico asociado a la clase
}


// Establecimiento de la comunicación USB serie a través del interfaz seleccionado en la comboBox, tras pulsar el
// botón RUN del interfaz gráfico. Se establece una comunicacion a 9600bps 8N1 y sin control de flujo en el objeto
// 'serial' que es el que gestiona la comunicación USB serie en el interfaz QT
void GUIPanel::startClient()
{
    _client->setHostName(ui->leHost->text());
    _client->setPort(1883);
    _client->setKeepAlive(300);
    _client->setCleanSession(true);
    _client->connectToHost();

}

// Funcion auxiliar de procesamiento de errores de comunicación (usada por startSlave)
void GUIPanel::processError(const QString &s)
{
    activateRunButton(); // Activa el botón RUN
    // Muestra en la etiqueta de estado la razón del error (notese la forma de pasar argumentos a la cadena de texto)
    ui->statusLabel->setText(tr("Status: Not running, %1.").arg(s));
}

// Funcion de habilitacion del boton de inicio/conexion
void GUIPanel::activateRunButton()
{
    ui->runButton->setEnabled(true);
}

// SLOT asociada a pulsación del botón RUN
void GUIPanel::on_runButton_clicked()
{
    startClient();
}


void GUIPanel::onMQTT_subscribed(const QString &topic)
{
     ui->statusLabel->setText(tr("subscribed %1").arg(topic));
}


void GUIPanel::on_pushButton_clicked()
{
    ui->statusLabel->setText(tr(""));
}


void GUIPanel::onMQTT_Received(const QMQTT::Message &message)
{
    bool previousblockinstate,checked;
    if (connected)
    {
        //Deshacemos el escalado


        QJsonParseError error;
        QJsonDocument mensaje=QJsonDocument::fromJson(message.payload(),&error);

        if ((error.error==QJsonParseError::NoError)&&(mensaje.isObject()))
        { //Tengo que comprobar que el mensaje es del tipo adecuado y no hay errores de parseo...

            QJsonObject objeto_json=mensaje.object();
            QJsonValue entrada=objeto_json["redLed"]; //Obtengo la entrada redLed. Esto lo puedo hacer porque el operador [] está sobrecargado
            QJsonValue entrada2=objeto_json["greenLed"]; //Obtengo la entrada orangeLed. Esto lo puedo hacer porque el operador [] está sobrecargado
            QJsonValue entrada3=objeto_json["blueLed"]; //Obtengo la entrada greenLed. Esto lo puedo hacer porque el operador [] está sobrecargado

            // obtener respuesta en topic COMMAND
            QJsonValue cmdResponse = objeto_json["cmd"];
            if (cmdResponse.isString() && cmdResponse.toString()==topicCommandCmds[PING_RESPONSE])
            {
                QMessageBox msgBox;
                msgBox.setText("Received ping response from board!!!");
                msgBox.setWindowTitle("PING response");
                msgBox.exec();
            }

            if (entrada.isBool())
            {   //Compruebo que es booleano...

                checked=entrada.toBool(); //Leo el valor de objeto (si fuese entero usaria toInt(), toDouble() si es doble....
                previousblockinstate=ui->gpioRedButton->blockSignals(true);   //Esto es para evitar que el cambio de valor
                                                                     //provoque otro envio al topic por el que he recibido

                ui->gpioRedButton->setChecked(checked);
                if (checked)
                {
                    ui->gpioRedButton->setText("Apaga");

                }
                else
                {
                    ui->gpioRedButton->setText("Enciende");
                }
                ui->gpioRedButton->blockSignals(previousblockinstate);
            }            
            if (entrada2.isBool())
            {   //Compruebo que es booleano...

                checked=entrada2.toBool(); //Leo el valor de objeto (si fuese entero usaria toInt(), toDouble() si es doble....
                previousblockinstate=ui->gpioGreenButton->blockSignals(true);   //Esto es para evitar que el cambio de valor
                                                                     //provoque otro envio al topic por el que he recibido

                ui->gpioGreenButton->setChecked(checked);
                if (checked)
                {
                    ui->gpioGreenButton->setText("Apaga");

                }
                else
                {
                    ui->gpioGreenButton->setText("Enciende");
                }
                ui->gpioGreenButton->blockSignals(previousblockinstate);
            }
            if (entrada3.isBool())
            {   //Compruebo que es booleano...

                checked=entrada3.toBool(); //Leo el valor de objeto (si fuese entero usaria toInt(), toDouble() si es doble....
                previousblockinstate=ui->gpioBlueButton->blockSignals(true);   //Esto es para evitar que el cambio de valor
                                                                     //provoque otro envio al topic por el que he recibido

                ui->gpioBlueButton->setChecked(checked);
                if (checked)
                {
                    ui->gpioBlueButton->setText("Apaga");

                }
                else
                {
                    ui->gpioBlueButton->setText("Enciende");
                }
                ui->gpioBlueButton->blockSignals(previousblockinstate);
            }
        }


    }
}


/* -----------------------------------------------------------
 MQTT Client Slots
 -----------------------------------------------------------*/
void GUIPanel::onMQTT_Connected()
{
    ui->runButton->setEnabled(false);

    // Se indica que se ha realizado la conexión en la etiqueta 'statusLabel'
    ui->statusLabel->setText(tr("Ejecucion, conectado al servidor"));

    connected=true;

    // suscribir a topic para enviar comandos a la placa
    _client->subscribe(topics[COMMAND],0);
    _client->subscribe(topics[LED],0);
}



void GUIPanel::SendMessageForGpioRGBLeds()
{
    QJsonObject objeto_json;
    //Añade un campo "redLed" al objeto JSON, con el valor (true o false) contenido en checked
    objeto_json["redLed"]=ui->gpioRedButton->isChecked(); //Puedo hacer ["redLed"] porque el operador [] está sobrecargado.
    //Añade un campo "orangeLed" al objeto JSON, con el valor (true o false) contenido en checked
    objeto_json["greenLed"]=ui->gpioGreenButton->isChecked(); //Puedo hacer ["orangeLed"] porque el operador [] está sobrecargado.
    //Añade un campo "greenLed" al objeto JSON, con el valor (true o false) contenido en checked
    objeto_json["blueLed"]=ui->gpioBlueButton->isChecked();

    QJsonDocument mensaje(objeto_json); //crea un objeto de tivo QJsonDocument conteniendo el objeto objeto_json (necesario para obtener el mensaje formateado en JSON)

    QMQTT::Message msg(0, topics[LED], mensaje.toJson()); //Crea el mensaje MQTT contieniendo el mensaje en formato JSON

    //Publica el mensaje
    _client->publish(msg);

}


void GUIPanel::on_pingButton_clicked()
{
    QJsonObject pingCmd;
    pingCmd["cmd"] = topicCommandCmds[PING];
    QJsonDocument payload(pingCmd);
    QMQTT::Message msg(0, topics[COMMAND], payload.toJson());

    //Publica el mensaje
    _client->publish(msg);
}


void GUIPanel::on_gpioRedButton_toggled(bool checked)
{
    //Rojo
    if (checked)
    {
        ui->gpioRedButton->setText("Apaga");

    }
    else
    {
        ui->gpioRedButton->setText("Enciende");
    }
    SendMessageForGpioRGBLeds();
}


void GUIPanel::on_gpioGreenButton_toggled(bool checked)
{
    //Verde
    if (checked)
    {
        ui->gpioBlueButton->setText("Apaga");

    }
    else
    {
        ui->gpioBlueButton->setText("Enciende");
    }
    SendMessageForGpioRGBLeds();
}


void GUIPanel::on_gpioBlueButton_toggled(bool checked)
{
    //Orange
    if (checked)
    {
        ui->gpioGreenButton->setText("Apaga");

    }
    else
    {
        ui->gpioGreenButton->setText("Enciende");
    }
    SendMessageForGpioRGBLeds();
}
