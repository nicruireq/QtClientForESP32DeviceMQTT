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
#include <vector>


std::map<QString, TopicName> mapTopicWithTag;

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
    isLedModePWM = false;    // By default always start in mode GPIO

    // initialize maps of topic strings to integer tags
    initializeMapOfTopics();

    // disable gui widgets for controlling board
    // while mqtt connection is not stablished
    disableBoardWidgetsInGUI();
}

GUIPanel::~GUIPanel() // Destructor de la clase
{
    delete ui;   // Borra el interfaz gráfico asociado a la clase
}


void GUIPanel::disableBoardWidgetsInGUI()
{
    ui->tabGroupForBoardWidgets->setDisabled(true);
    // Default state of widgets in temperature tab
    ui->buttonTempStop->setDisabled(true);
    ui->qwtTempPlot->setDisabled(true);

}


void GUIPanel::enableBoardWidgetsInGUI()
{
    ui->tabGroupForBoardWidgets->setEnabled(true);
    // by default indicators for buttons are in off state
    ui->button1StateLed->setChecked(false);
    ui->button2StateLed->setChecked(false);
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


void GUIPanel::processFromTopicCommand(const QJsonObject &jsonData)
{
    // get response on topic COMMAND
    QJsonValue cmdResponse = jsonData["cmd"];

    if (cmdResponse.isString() && cmdResponse.toString()==topicCommandCmds[PING_RESPONSE])
    {
        // process ping response
        QMessageBox msgBox;
        msgBox.setText("Received ping response from board!!!");
        msgBox.setWindowTitle("PING response");
        msgBox.exec();
    }
    else if (cmdResponse.isString() && cmdResponse.toString()==topicCommandCmds[ACK_MODE_LEDS_PWM])
    {
        // process response to change mode of leds to pwm
        // This ack means that the change to PWM in the board has succeed
        ui->redKnob->setEnabled(true);
        ui->greenKnob->setEnabled(true);
        ui->blueKnob->setEnabled(true);
        ui->groupBox_onOffLedsGpioButtons->setDisabled(true);
        isLedModePWM = true;
        // change control of led mode in GUI
        bool previousblockinstate = ui->radioBtnPWM->blockSignals(true);
        ui->radioBtnPWM->setChecked(true);
        ui->radioBtnPWM->blockSignals(previousblockinstate);
    }
    else if (cmdResponse.isString() && cmdResponse.toString()==topicCommandCmds[ACK_MODE_LEDS_GPIO])
    {
        // process response to change mode of leds to GPIO
        // This ack means that the change to GPIO in the board has succeed
        ui->redKnob->setDisabled(true);
        ui->greenKnob->setDisabled(true);
        ui->blueKnob->setDisabled(true);
        ui->groupBox_onOffLedsGpioButtons->setEnabled(true);
        isLedModePWM = false;
        // change control of led mode in GUI
        bool previousblockinstate = ui->radioBtnGPIO->blockSignals(true);
        ui->radioBtnGPIO->setChecked(true);
        ui->radioBtnGPIO->blockSignals(previousblockinstate);
    }
    else if (cmdResponse.isString() && cmdResponse.toString()==topicCommandCmds[ACK_MODE_PUSH_BUTTONS_ASYNC])
    {
        ui->sondeaButton->setDisabled(true);
        ui->checkBoxAsyncMode->setEnabled(true);
    }
    else if (cmdResponse.isString() && cmdResponse.toString()==topicCommandCmds[ACK_MODE_PUSH_BUTTONS_POLL])
    {
        ui->sondeaButton->setEnabled(true);
        ui->checkBoxAsyncMode->setEnabled(false);
    }
    else if (cmdResponse.isString() && cmdResponse.toString()==topicCommandCmds[ACK_START_TEMP_SAMPLING])
    {
        ui->buttonTempStart->setDisabled(true);
        ui->buttonTempStop->setEnabled(true);
        ui->qwtTempPlot->setEnabled(true);
        configureQwtPlot();
    }
    else if (cmdResponse.isString() && cmdResponse.toString()==topicCommandCmds[ACK_STOP_TEMP_SAMPLING])
    {
        ui->buttonTempStart->setEnabled(true);
        ui->buttonTempStop->setDisabled(true);
        ui->qwtTempPlot->setDisabled(true);
    }

}


void GUIPanel::processFromTopicGPIOLed(const QJsonObject &jsonData)
{
    bool previousblockinstate, checked;
    // Manage GPIO RGB LEDs representation
    QJsonValue entrada=jsonData["redLed"]; //Obtengo la entrada redLed. Esto lo puedo hacer porque el operador [] está sobrecargado
    QJsonValue entrada2=jsonData["greenLed"]; //Obtengo la entrada orangeLed. Esto lo puedo hacer porque el operador [] está sobrecargado
    QJsonValue entrada3=jsonData["blueLed"]; //Obtengo la entrada greenLed. Esto lo puedo hacer porque el operador [] está sobrecargado

    // Case Mode GPIO
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


void GUIPanel::processFromTopicPWMLed(const QJsonObject &jsonData)
{
    bool previousblockinstate;

    QJsonValue inputRed = jsonData["redLed"];
    QJsonValue inputGreen = jsonData["greenLed"];
    QJsonValue inputBlue = jsonData["blueLed"];

    previousblockinstate=ui->redKnob->blockSignals(true);   //Esto es para evitar que el cambio de valor
    if (inputRed.isDouble())
    {
        ui->redKnob->setValue(inputRed.toInt());
    }
    ui->redKnob->blockSignals(previousblockinstate);

    previousblockinstate=ui->greenKnob->blockSignals(true);
    if (inputGreen.isDouble())
    {
        ui->greenKnob->setValue(inputGreen.toInt());
    }
    ui->greenKnob->blockSignals(previousblockinstate);

    previousblockinstate=ui->blueKnob->blockSignals(true);
    if (inputBlue.isDouble())
    {
        ui->blueKnob->setValue(inputBlue.toInt());
    }
    ui->blueKnob->blockSignals(previousblockinstate);
}

void GUIPanel::processFromTopicButtons(const QJsonObject &jsonData)
{
    // get response from buttons state polling
    // get response on topic COMMAND
    QJsonValue button1State = jsonData["button1"];
    QJsonValue button2State = jsonData["button2"];
    const QString on = "on";
    const QString off = "off";

    if (button1State.isString() && button1State.toString()==on)
    {
        ui->button1StateLed->setChecked(true);
    }
    else if (button1State.isString() && button1State.toString()==off)
    {
        ui->button1StateLed->setChecked(false);
    }

    if (button2State.isString() && button2State.toString()==on)
    {
        ui->button2StateLed->setChecked(true);
    }
    else if (button2State.isString() && button2State.toString()==off)
    {
        ui->button2StateLed->setChecked(false);
    }

}

void GUIPanel::processFromTopicAdc(const QJsonObject &jsonData)
{
    QJsonValue lastReading = jsonData["last_reading"];

    if (lastReading.isDouble())
    {
        ui->potentiometerReading->setValue(lastReading.toInt());
    }
}


void GUIPanel::processFromTopicBoardStatus(const QJsonObject &jsonData)
{
    QJsonValue boardState = jsonData["status"];

    if (boardState.isString())
    {
        if (boardState.toString() == "alive")
        {
            ui->ledConnectionIndicator->setColor(QColor(0,255,0));
            enableBoardWidgetsInGUI();
        }
        else if (boardState.toString() == "died")
        {
            ui->ledConnectionIndicator->setColor(QColor(255,0,0));
            // change program state to disconnected
            connected = false;
            _client->disconnectFromHost();  // fail??
            disableBoardWidgetsInGUI();
            activateRunButton();
        }
    }
}

void GUIPanel::configureQwtPlot()
{
    //Configuramos la grafica
    ui->qwtTempPlot->setTitle("Temperature sensor");
    ui->qwtTempPlot->setAxisTitle(QwtPlot::xBottom,"Sample");
    ui->qwtTempPlot->setAxisTitle(QwtPlot::yLeft, "Cº");
    //ui->qwtTempPlot->axisAutoScale(true); // Con Autoescala
    ui->qwtTempPlot->setAxisScale(QwtPlot::yLeft, 0, 50); // Con escala definida
    ui->qwtTempPlot->setAutoReplot(false);

    //Creamos una curva y la añadimos a la grafica
    m_curve_1 = new QwtPlotCurve();
    m_curve_1->setPen(QPen(Qt::red));
    m_Grid = new QwtPlotGrid();
    m_Grid->attach(ui->qwtTempPlot);
    m_curve_1->attach(ui->qwtTempPlot);

    //Inicializadmos los datos que se muestran en la grafica
    for (int i=0; i<NMAX; i++) {
        yVal1[i]=0;
        xVal[i]=i;
    }
    m_curve_1->setRawSamples(xVal,yVal1,NMAX);
    ui->qwtTempPlot->replot();
}

void GUIPanel::processFromTopicTemp(const QJsonObject &jsonData)
{
    QJsonValue sample = jsonData["grades"];

    if (sample.isDouble())
    {
        //Actualizamos gráfica correspondiente
        memmove(yVal1,yVal1+1,sizeof(double)*(NMAX-1)); //Desplazamos las muestras hacia la izquierda
        yVal1[NMAX-1] = sample.toDouble(); //Añadimos el último punto
        m_curve_1->setRawSamples(xVal,yVal1,NMAX);  //Refrescamos..
        ui->qwtTempPlot->replot();
    }
}


void GUIPanel::onMQTT_Received(const QMQTT::Message &message)
{
    if (connected)
    {
        QJsonParseError error;
        QJsonDocument mensaje=QJsonDocument::fromJson(message.payload(),&error);

        if ((error.error==QJsonParseError::NoError)&&(mensaje.isObject()))
        { //Tengo que comprobar que el mensaje es del tipo adecuado y no hay errores de parseo...

            QJsonObject objeto_json=mensaje.object();

            switch (mapTopicWithTag[message.topic()]) {
            case COMMAND:
                processFromTopicCommand(objeto_json);
                break;
            case LED:
                if (isLedModePWM)
                    processFromTopicPWMLed(objeto_json);
                else
                    processFromTopicGPIOLed(objeto_json);
                break;
            case BUTTONS:
                processFromTopicButtons(objeto_json);
                break;
            case ADC:
                processFromTopicAdc(objeto_json);
                break;
            case BOARD_STATUS:
                processFromTopicBoardStatus(objeto_json);
                break;
            case TEMP:
                processFromTopicTemp(objeto_json);
                break;
            default:
                break;
            }
        }   // data processing end
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

    // suscribir a topics
    for (QString t : topics)
    {
        _client->subscribe(t, 0);
    }

    // enable widgets for controlling board in GUI
    enableBoardWidgetsInGUI();
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


void GUIPanel::SendMessageForPWMRGBLeds()
{
    QJsonObject objeto_json;

    objeto_json["redLed"] = (uint8_t) ui->redKnob->value();
    objeto_json["greenLed"] = (uint8_t) ui->greenKnob->value();
    objeto_json["blueLed"] = (uint8_t) ui->blueKnob->value();

    QJsonDocument mensaje(objeto_json);

    QMQTT::Message msg(0, topics[LED], mensaje.toJson());

    _client->publish(msg);
}


// Generate json message of type command and publish
// on COMMAND topic
void GUIPanel::SendMessageCommand(const Commands &name, CommandParams* params)
{
    QJsonObject jsonCmd;
    jsonCmd["cmd"] = topicCommandCmds[name];
    // add params if this are available
    if (params != nullptr)
    {
        for (auto pair : (*params))
        {
            jsonCmd[pair.first] = pair.second;
        }
    }
    QJsonDocument payload(jsonCmd);
    QMQTT::Message msg(0, topics[COMMAND], payload.toJson());

    _client->publish(msg);
}


void GUIPanel::on_pingButton_clicked()
{
    SendMessageCommand(PING);
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

void GUIPanel::on_sondeaButton_clicked()
{
    SendMessageCommand(POLL_BUTTONS);
}


void GUIPanel::on_radioBtnGPIO_clicked(bool checked)
{
    if (checked)
    {
        SendMessageCommand(MODE_LEDS_GPIO);
    }
}


void GUIPanel::on_radioBtnPWM_clicked(bool checked)
{
    if (checked)
    {
        SendMessageCommand(MODE_LEDS_PWM);
    }
}


void GUIPanel::on_redKnob_valueChanged(double value)
{
    SendMessageForPWMRGBLeds();
}


void GUIPanel::on_greenKnob_valueChanged(double value)
{
    SendMessageForPWMRGBLeds();
}


void GUIPanel::on_blueKnob_valueChanged(double value)
{
    SendMessageForPWMRGBLeds();
}


void GUIPanel::on_checkBoxAsyncMode_toggled(bool checked)
{
    if (connected)
    {
        if (!checked)
        {
            // change to poll mode
            SendMessageCommand(MODE_PUSH_BUTTONS_POLL);
        }
        else
        {
            // if checked -> change to async mode
            SendMessageCommand(MODE_PUSH_BUTTONS_ASYNC);
        }
    }
}


void GUIPanel::on_buttonTempStart_clicked()
{
    double interval = ui->dsbTempSamplingInterval->value() * 1000;
    CommandParams params = {{"interval", QString::number(interval)}};

    if (connected)
    {
        SendMessageCommand(START_TEMP_SAMPLING, &params);
    }
}


void GUIPanel::on_buttonTempStop_clicked()
{
    if (connected)
    {
        SendMessageCommand(STOP_TEMP_SAMPLING);
    }
}

