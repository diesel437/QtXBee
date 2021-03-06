/*
 * Copyright (C) 2015 ThomArmax (Thomas COIN)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Thomas COIN <esvcorp@gmail.com> 18/04/2015
 */

#include "XBeePacket"
#include <QDebug>

namespace QtXBee {

/**
 * @brief XBeePacket's constructor
 * @param parent
 */
XBeePacket::XBeePacket(QObject *parent) :
    QObject(parent),
    m_startDelimiter(0x7E),
    m_length(0),
    m_frameType(UndefinedId),
    m_frameId(0),
    m_checksum(0)
{
}

/**
 * @brief Sets the frame's start delimiter.
 * By default the start delemiter is set to 0x7E
 * @sa XBeePacket::startDelimiter()
 * @param sd
 */
void XBeePacket::setStartDelimiter(unsigned sd)
{
    m_startDelimiter = sd;
}

/**
 * @brief Sets the frame's length
 * @param length
 * @sa XBeePacket::length()
 * @todo Does it make sense that this method is public ? If not, make it protected.
 */
void XBeePacket::setLength(unsigned length)
{
    m_length = length;
}

/**
 * @brief Sets the frame's type
 * @param type
 * @sa XBeePacket::frameType()
 * @todo Does it make sense that this method is public ? If not, make it protected.
 */
void XBeePacket::setFrameType(ApiId type)
{
    m_frameType = type;
}

/**
 * @brief Sets the frame's id
 * @param id
 * @sa XBeePacket::frameId()
 */
void XBeePacket::setFrameId(quint8 id)
{
    m_frameId = id;
}

/**
 * @brief Sets the checksum
 * @param cs
 * @sa XBeePacket::checksum()
 * @todo Does it make sense that this method is public ? If not, make it protected.
 */
void XBeePacket::setChecksum(unsigned cs)
{
    m_checksum = cs;
}

/**
 * @brief Returns the frame's start delimiter
 * @returns the frame's start delimiter
 * @sa XBeePacket::setStartDelimiter()
 * @todo Does it make sense that this method is public ? If not, make it protected.
 */
unsigned XBeePacket::startDelimiter() const
{
    return m_startDelimiter;
}

/**
 * @brief Returns the frame-specific data length (Number of bytes between the length and the checksum)
 * @returns the packet's length
 * @sa XBeePacket::setLength()
 */
quint16 XBeePacket::length() const
{
    return m_length;
}

/**
 * @brief Returns the frame's type
 * @returns the frame's type
 * @sa XBeePacket::setFrameType()
 * @sa XBeePacket::APIFrameType
 */
XBeePacket::ApiId XBeePacket::frameType() const
{
    return m_frameType;
}

/**
 * @brief Returns the frame's id
 * @returns the frame's id
 * @sa XBeePacket::setFrameId()
 */
quint8 XBeePacket::frameId() const
{
    return m_frameId;
}

/**
 * @brief Returns the packet's checksum
 * @returns the packet's checksum
 * @sa XBeePacket::setChecksum()
 */
unsigned XBeePacket::checksum() const
{
    return m_checksum;
}

/**
 * @brief Computes the checksum of the given QByteArray, and set it.
 * @param array
 * @sa XBeePacket::setChecksum()
 * @sa XBeePacket::checksum()
 */
void XBeePacket::createChecksum(QByteArray array)
{
    int len = array.size();
    unsigned int sum = 0x00;
    unsigned int ff = 0xFF;
    unsigned int fe = 0xFE;
    for(int i=0;i<len;i++)
    {
        unsigned int a = array.at(i);
        if (a == (unsigned int)4294967295){
            a = ff;
        } else if (a == (unsigned int)4294967294){
            a = fe;
        }

        sum += a;
    }
    setChecksum((ff - sum) & 0x000000FF);
}

/**
 * @brief Returns the frame's packet (raw data)
 * @returns the frame's packet (raw data)
 */
QByteArray XBeePacket::packet() const
{
    return m_packet;
}

/**
 * @brief Sets the packet's raw data
 * @param packet
 * @return true if succeeded; false otherwise.
 */
bool XBeePacket::setPacket(const QByteArray &packet)
{
    ApiId apiId = UndefinedId;
    quint8 apiSpecificOffset = 4; // 5th byte
    QByteArray specificData;

    clear();
    m_packet = packet;

    if(packet.size() < 5) {
        qDebug() << Q_FUNC_INFO << "bad packet !";
        return false;
    }

    setStartDelimiter(packet.at(0));
    setLength((unsigned char)packet.at(2) + ((unsigned char)packet.at(1)<<8));
    apiId = (ApiId)(packet.at(3)&0xff);
    if(apiId != frameType()) {
        qDebug() << Q_FUNC_INFO << "Bad API frame ID !" << qPrintable(QString("(expected 0x%1, got 0x%2)").arg(frameType(),0,16).arg(apiId,0,16));
        return false;
    }

    if(packet.size() > 5) {
        for(int i=apiSpecificOffset; i< packet.size()-1; i++) {
            specificData.append(packet.at(i));
        }
        parseApiSpecificData(specificData);
    }
    setChecksum(packet.at(packet.size()-1));

    assemblePacket();
    return true;
}

/**
 * @brief Parses the packet API specific data.
 *
 * Are conciderated as packet specific data, the data between API frame Id and checksum.
 *
 * For example, the AT command response frame:
 * @image html at_command_response_frame.jpg
 * The specific part is bytes [6 - (#length() -1)] (AT command, Status, Value)
 *
 * @param data
 * @return true if succeeded; false otherwise.
 * @note All subclasses must reimplement this method, which is called by XBeePacket::setPacket()
 */
bool XBeePacket::parseApiSpecificData(const QByteArray &data)
{
    Q_UNUSED(data)
    return false;
}

/**
 * @brief Assembles the packet to be able to send it.
 *
 * This function is called by the XBee class, when the packet in being sent.
 *
 * @note Overload this function to create your own packet.
 */
void XBeePacket::assemblePacket()
{

}

/**
 * @brief Clears the packet by reseting all attributes to their default value.
 * @note All subclass should reimplement this method.
 */
void XBeePacket::clear()
{
    m_packet.clear();
    m_startDelimiter = 0x7E;
    m_length = 0;
    m_frameId = -1;
    m_checksum = -1;
}

/**
 * @brief Returns a debug string containing all packet's informations.
 * @returns a debug string containing all packet's informations.
 */
QString XBeePacket::toString()
{
    QString str;
    str.append(QString("Raw packet      : 0x%1\n").arg(QString(packet().toHex())));
    str.append(QString("Frame id        : %1 (0x%2)\n").arg(frameId(), 0, 16).arg(frameId(), 0, 16));
    str.append(QString("Frame type      : %1 (0x%2)\n").arg(frameTypeToString(frameType())).arg(QString::number(frameType(), 16)));
    str.append(QString("Start delimiter : 0x%1\n").arg(QString::number(startDelimiter(), 16)));
    str.append(QString("Length          : %1 bytes\n").arg(m_length));
    str.append(QString("Checksum        : %1\n").arg(checksum()));

    return str;
}

/**
 * @brief Returns the given frame type ApiId into a human readable string
 * @returns the given frame type ApiId into a human readable string
 * @param type
 * @sa ApiId
 */
QString XBeePacket::frameTypeToString(const ApiId type)
{
    QString str;
    switch(type) {
    case TxRequest64Id              : str = "Tx 16 bits Address Request"                ; break;
    case TxRequest16Id              : str = "Tx 16 bits Address Request"                ; break;
    case ATCommandId                : str = "AT Command Request"                        ; break;
    case ATCommandQueueId           : str = "AT Command - Queue Parameter Request"      ; break;
    case ZBTxRequestId              : str = "ZigBee Tx Request"                         ; break;
    case ZBExplicitTxRequestId      : str = "ZigBee Explicit Tx Request"                ; break;
    case RemoteATCommandRequestId   : str = "Remote AT Command Request"                 ; break;
    case CreateSourceRouteId        : str = "Create Source Route"                       ; break;
    case Rx64ResponseId             : str = "Rx 64 bits Address Response"               ; break;
    case Rx16ResponseId             : str = "Rx 16 bits Address Response"               ; break;
    case Rx64IOResponseId           : str = "Rx 64 bits Address I/O Response"           ; break;
    case Rx16IOResponseId           : str = "Rx 16 bits Address I/O Response"           ; break;
    case ATCommandResponseId        : str = "AT Command Response"                       ; break;
    case TxStatusResponseId         : str = "Tx Status Response"                        ; break;
    case ModemStatusResponseId      : str = "Modem Status Response"                     ; break;
    case ZBTxStatusResponseId       : str = "ZigBee Tx Status Response"                 ; break;
    case ZBRxResponseId             : str = "ZigBee Rx Response"                        ; break;
    case ZBExplicitRxResponseId     : str = "ZigBee Explicit Rx Response"               ; break;
    case ZBIOSampleResponseId       : str = "ZigBee I/O Sample Response"                ; break;
    case XBeeSensorReadIndicatorId  : str = "XBee Sensore Read Indicator"               ; break;
    case ZBIONodeIdentificationId   : str = "ZigBee I/O Node Indentification Indicator" ; break;
    case RemoteATCommandResponseId  : str = "Remote AT Command Response"                ; break;
    case OverTheAirFirmwareUpdateId : str = "Over-the-air Firmware Update"              ; break;
    case RouteRecordIndicatorId     : str = "Route Record Indicator"                    ; break;
    case ManyToOneRouteRequestId    : str = "Many-to-one Route Request"                 ; break;
    default                         : str = "Unkown"                                    ; break;
    }
    return str;
}

void XBeePacket::escapePacket()
{
    int escapeBytes = 0;
    QByteArray escapedPacked;
    int i;

    // Skip byte 0 (start delimiter)
    for (i=1; i<m_packet.size(); i++) {
        if (isSpecialByte(m_packet.at(i))) {
            qDebug() << Q_FUNC_INFO << "packet requires escaping for byte" << i << "(" <<QByteArray().append(m_packet.at(i)).toHex() << ")";
            escapeBytes++;
        }
    }

    if (escapeBytes == 0) {
        qDebug() << "No need to escape packet" << m_packet.toHex();
        return;
    }
    else {
        qDebug() << Q_FUNC_INFO << "packet need to be escaped";

        int pos = 1;

        escapedPacked.reserve(m_packet.size() + escapeBytes);
        escapedPacked[0] = startDelimiter();

        for (i = 1; i < m_packet.size(); i++) {
            if (isSpecialByte(m_packet[i])) {
                escapedPacked[pos] = StartDelimiter;
                escapedPacked[++pos] = 0x20 ^ m_packet[i];

                qDebug() << Q_FUNC_INFO << "xor'd byte is" << QString("0x%1").arg(escapedPacked[pos]);
            } else {
                escapedPacked[pos] = m_packet[i];
            }
            pos++;
        }
        qDebug() << Q_FUNC_INFO << m_packet.toHex() << "===>" << escapedPacked.toHex();
        m_packet.clear();
        m_packet = escapedPacked;
    }
}

bool XBeePacket::unescapePacket()
{
    QByteArray unEscapedPacket;
    int escapeBytes = 0;
    int i = 0;
    int pos = 0;

    for(i=0; i<m_packet.size(); i++) {
        if(m_packet.at(i) == Escape) {
            escapeBytes++;
        }
    }

    if (escapeBytes == 0) {
        return true;
    }

    unEscapedPacket.reserve(m_packet.size() - escapeBytes);

    for(i = 0; i<m_packet.size(); i++) {
        if (m_packet[i] == (char)Escape) {
            // discard escape byte and un-escape following byte
            unEscapedPacket[pos] = 0x20 ^ m_packet[++i];
        } else {
            unEscapedPacket[pos] = m_packet[i];
        }
        pos++;
    }

    qDebug() << Q_FUNC_INFO << m_packet.toHex() << "====>" << unEscapedPacket.toHex();
    return true;
}

bool XBeePacket::isSpecialByte(const char c)
{
    return  c == StartDelimiter ||
            c == Escape ||
            c == XON ||
            c == XOFF;
}

} // END namepsace
