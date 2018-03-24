#ifndef ARDUINOSERIALMONITOR_H
#define ARDUINOSERIALMONITOR_H

#include <QObject>
#include <QtSerialPort>
#include <cstdio>
#include "arduinoexceptions.h"

class ArduinoSerialMonitor:public QObject
{
    Q_OBJECT

public:
    /**
     * @brief ArduinoSerialMonitor
     * @param portName the directory to the port, example "/dev/ttyUSB1"
     * @param baudrate baudrate communication with Arduino
     * @param parent QObject parent
     */
    ArduinoSerialMonitor(QString portName, int baudrate, QObject* parent = NULL);
    virtual ~ArduinoSerialMonitor();

    /**
     * @brief open Opens the serial port communications.
     * @throws SerialPortOpenException
     */
    void open();

    /**
     * @brief close Closesserial port communication
     */
    void close();

    /**
     * @brief sendToArduino Sends string to Arduino through serial port
     * @param msg the message to send
     */
    void sendToArduino(QString msg);

public slots:
    /**
     * @brief readArduino Slot called whenever a message is received through serial port
     */
    void readArduino();

    /**
     * @brief writeString Just for testing
     * @param str
     */
    void writeString(QString str){qDebug()<<str;}

signals:
    /**
     * @brief lineReceived Signal emitted when a line is received through serial port
     * @param line the received string line
     */
    void lineReceived(QString line);


protected:
    QSerialPort port; ///port to manage serial communications
};




#endif // ARDUINOSERIALMONITOR_H