//main.cpp

#include <QCoreApplication>
#include <QDebug>

#include <XBee>
#include <RemoteATCommandRequest>
#include "xbeeresponseprinter.h"

using namespace QtXBee;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    XBee xb;
    RemoteATCommandRequest remoteNI;
    XBeeResponsePrinter printer;

    // Configure serial
    // port here
    xb.setSerialPort("/dev/ttyUSB0");

    if(!xb.open()) {
        qFatal("Failed to open serial port");
    }

    QObject::connect(&xb,
                     SIGNAL(receivedRemoteCommandResponse(RemoteATCommandResponse*)),
                     &printer,
                     SLOT(onPacketReceived(RemoteATCommandResponse*))
                     );

    remoteNI.setCommand(ATCommand::ATNI); // Node Indentifier
    remoteNI.setDestinationAddress16(0x2);
    xb.sendAsync(&remoteNI);

    return a.exec();
}
